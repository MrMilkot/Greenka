//  **********************
//  *** NodeMCU Sketch ***
//  **********************

#include <ArduinoJson.h>
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WebSocketsClient.h>

#include <Hash.h>
#include <WiFiManager.h>

// region WiWi socket
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

// region JSON
const int capacity = JSON_OBJECT_SIZE(4);
StaticJsonDocument<capacity> doc;
String jsonStr;

// region WiFi Manager
bool wm_nonblocking = false; // global wm instance
WiFiManager wm; // global wm instance



#define TRIGGER_PIN 0
#define managerSsid "Greenko-AAA1"
#define managerPass "greenko1234"

// endregion

// region timers
uint32_t updateTimer;  // Таймер для запросов к Ardunio
int updatePeriod = 1000; // Период обновления запросов к Ardunio
// *****
uint32_t socketTimer;
int socketPeriod = 3000; 
// *****


void setup() {
Serial.begin(115200);
while (!Serial) {
  Serial.println("No connection with Arduino");
  }
  startWm();
}

void loop() { 
  getUpdatesArduino();
  
  if(wm_nonblocking) wm.process();
  wmCheckButton();

  webSocket.loop();
}

void startWebSocket() {

  Serial.println("Start Web Socket");
    
    const char *cPass = wm.getWiFiPass().c_str();
    const char *cSSID = wm.getWiFiSSID().c_str();
  
  WiFiMulti.addAP(cSSID, cPass);

  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println("Wifi[Connected]");
  webSocket.begin("asparo.ru", 80, "/greenko/websockets");
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("user", "Password");
  webSocket.setReconnectInterval(5000); // Каждые n секунд пытается переподключиться
  
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  webSocket.enableHeartbeat(15000, 3000, 2);
}

void getUpdatesArduino() {
  uint32_t timeLeftUpdate = millis() - updateTimer;
  if (timeLeftUpdate >= updatePeriod) {
    updateTimer += updatePeriod * (timeLeftUpdate / updatePeriod);
    String str = "";
  while (Serial.available()) {
  int a = Serial.read();
  Serial.write(a);
  str +=  (char)a;
}
  Serial.write("GET_ARDUINO: ");
  Serial.println(str);
  }
 }

 void startWm() {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP  
  Serial.begin(115200);
  Serial.setDebugOutput(true);  
  Serial.println("\n WM: Starting");

  pinMode(TRIGGER_PIN, INPUT);

  // wm.resetSettings(); // wipe settings
 
  if(wm_nonblocking) wm.setConfigPortalBlocking(false);
// std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
  std::vector<const char *> menu = {"wifi","info"};
  wm.setMenu(menu);
  wm.setClass("invert");
  bool res;
  res = wm.autoConnect(managerSsid,managerPass); // password protected ap
  if(!res) {
    Serial.println("[WM]: Failed to connect or hit timeout");
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("[WM]: Connected. "+ wm.getWiFiPass() + " - " + wm.getWiFiSSID());
    startWebSocket();
    wm.disconnect();
  }
}
void wmCheckButton(){
  // check for button press
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    //  TODO: Заменить способ очистки данных (custom button maybe)
    delay(50);
    if( digitalRead(TRIGGER_PIN) == LOW ){
      Serial.println("Button Pressed");
      delay(3000); // reset delay hold
      if( digitalRead(TRIGGER_PIN) == LOW ){
        Serial.println("Button Held");
        Serial.println("Erasing Config, restarting");
        wm.resetSettings();
        ESP.restart();
      }
      Serial.println("[WM]: Starting config portal");
      wm.setConfigPortalTimeout(120);
      
      if (!wm.startConfigPortal(managerSsid, managerPass)) {
        Serial.println("[WM]: Failed to connect or hit timeout");
        delay(1000);
      } else {
        Serial.println("[WM]: Connected. "+ wm.getWiFiPass() + " - " + wm.getWiFiSSID());
        startWebSocket();
        wm.disconnect();
      }
    }
  }
}

void serializeJSON() {
      doc["temp"] = "23.5";
      doc["humidity"] = "66";
      doc["lightWorked"] = true;
      doc["pumpWorked"] = false;
      serializeJson(doc, jsonStr);
  }
  
void timerSocket() {
  uint32_t timeLeftSocket = millis() - socketTimer;
      if (timeLeftSocket >= socketPeriod) {
      socketTimer += socketPeriod * (timeLeftSocket / socketTimer);
      serializeJSON();
      webSocket.sendTXT(jsonStr);
      }
 }

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      // send message to server when Connected
    }
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] get text: %s\n", payload);
    // добавить таймер todo
      // send message to server
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
        case WStype_PING:
            // pong will be send automatically
            Serial.printf("[WSc] get ping\n");
            break;
        case WStype_PONG:
            // answer to a ping we send
            Serial.printf("[WSc] get pong\n");
            break;
    }

}
