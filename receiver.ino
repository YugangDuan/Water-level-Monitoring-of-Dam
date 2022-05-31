#include <ESP8266WiFi.h>
#include <espnow.h>
#include <WiFiClientSecure.h>
#include <base64.h>
#include <string>
// replace with your channel’s thingspeak API key,
String apiKey = "0QBW49UXT4290LR1";
const char* ssid = "B-207@Museum_walk";//WIFi名
const char* password = "GeaZvZqEp";//WIFI密码
const char* server = "mqtt.thingspeak.com";
int isAlarm = 0;
WiFiClient client;
uint8_t rainfallAddress[] = {0xE8,0xDB,0x84,0xAF,0x6A,0xB8};//rainfall MAC
uint8_t radioAddress[] = {0xE8,0xDB,0x84,0xAF,0x5D,0xEE};//Radio MAC

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message_1 {
    int rainload;
    int israinfall;
} struct_message_1;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message_2 {
  long duration;
  float distanceCm;
  float distanceInch;
} struct_message_2;

typedef struct struct_message_3 {
  int collectrate;
} struct_message_3;

// Create a struct_message called myData
struct_message_1 rainfallData;
struct_message_2 distanceData;
struct_message_3 returnData;

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  int israin = 0;
  for(int i = 0;i<=5;i++){
    if(*(mac+i) != rainfallAddress[i]){
      israin = 1;
    }
  }
  if(israin ==0){
    memcpy(&rainfallData, incomingData, sizeof(rainfallData));
    Serial.print("Bytes received: ");
    Serial.println(len);
    Serial.print("Rainfall: ");
    Serial.println(rainfallData.rainload);
    Serial.print("Israin: ");
    Serial.println(rainfallData.israinfall);
    Serial.println();
 

  }
  else{
    memcpy(&distanceData, incomingData, sizeof(distanceData));
    Serial.print("Bytes received: ");
    Serial.println(len);
    Serial.print("duration: ");
    Serial.println(distanceData.duration);
    Serial.print("distanceCm: ");
    Serial.println(distanceData.distanceCm);
    Serial.print("distanceInch: ");
    Serial.println(distanceData.distanceInch);
    Serial.println();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  if (client.connect(server,80)) { // "184.106.153.149" or api.thingspeak.com
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(distanceData.distanceCm);
    postStr +="&field2=";
    postStr += String(rainfallData.rainload);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  }

}
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(radioAddress,ESP_NOW_ROLE_COMBO,1,NULL,0);
  esp_now_add_peer(rainfallAddress,ESP_NOW_ROLE_COMBO,2,NULL,0);
  esp_now_register_recv_cb(OnDataRecv);
}


void loop() {

  if (isnan(rainfallData.rainload) || isnan(distanceData.distanceCm)) {
    Serial.println("Failed to read from data sensor!");
    return;
  }
  if(rainfallData.rainload >= 500){
    returnData.collectrate = 15000;
    esp_now_send(radioAddress, (uint8_t *) &returnData, sizeof(returnData));
  }else{
    returnData.collectrate = 30000;
    esp_now_send(radioAddress, (uint8_t *) &returnData, sizeof(returnData));
  }
  Serial.println("Waiting…");
  // thingspeak needs minimum 15 sec delay between updates
  delay(1000);//延迟
}
