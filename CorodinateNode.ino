#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <string>
#define rainAnalog A0

// WiFi
const char *ssid = "B-207@Museum_walk"; // Enter your WiFi name
const char *password = "GeaZvZqEp";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "rainfall";
const char *topic_2 = "8266rate";
const char *topic_3 = "distanced";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

//ThingSpeak
String apiKey = "0QBW49UXT4290LR1";
const char* server = "api.thingspeak.com";

WiFiClient espClient;
PubSubClient client(espClient);
String waterlevel = "-1";
String waterfall = "-1";
int lastSender = 0;
String rate = "30";

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    // connecting to a WiFi network
    pinMode(LED_BUILTIN,OUTPUT);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);

    while (!client.connected()) {
        String client_id = "esp8266-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public emqx mqtt broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    client.subscribe(topic_3);
    client.subscribe(topic);
}
void callback(char *topic, byte *payload, unsigned int length) {
  int israin = 1;
  for(int i = 0;i<=4;i++){
    if( topic[i]!= topic_3[i]){
      israin = 0;
    }
  }
  if(israin ==0){
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    char chTemp[1024] = {};
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
        chTemp[i] = (char)payload[i];
    }
    Serial.println();
    waterfall = chTemp;
    memset(chTemp, 0x00, 1024);
    lastSender = 0;
  }else{
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    char chTemp[1024] = {};
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
        chTemp[i] = (char)payload[i];
    }
    Serial.println();
    waterlevel = chTemp;
    memset(chTemp, 0x00, 1024);
    lastSender = 1;
  }
  if(atoi(waterfall.c_str())>=0&&atoi(waterlevel.c_str())>=0){
    if (espClient.connect(server,80)) { // "184.106.153.149" or api.thingspeak.com
      String postStr = apiKey;
      postStr +="&field1=";
      postStr += waterlevel;
      postStr +="&field2=";
      postStr += waterfall;
      postStr +="&field3=";
      postStr += rate;
      postStr += "\r\n\r\n";

      espClient.print("POST /update HTTP/1.1\n");
      espClient.print("Host: api.thingspeak.com\n");
      espClient.print("Connection: close\n");
      espClient.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
      espClient.print("Content-Type: application/x-www-form-urlencoded\n");
      espClient.print("Content-Length: ");
      espClient.print(postStr.length());
      espClient.print("\n\n");
      espClient.print(postStr);
  }
  espClient.stop();
      while (!client.connected()) {
        String client_id = "esp8266-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public emqx mqtt broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
  if(atoi(waterfall.c_str()) >=50){
    client.publish(topic_2, String(15000).c_str(), true);
    rate = "15";
  }else{
    client.publish(topic_2, String(30000).c_str(), true);
    rate = "30";
  }
  }
}
void loop() {
    // publish and subscribe
    if(lastSender ==0){
      client.subscribe(topic_3);
    }else{
      client.subscribe(topic);
    }
    if(atoi(waterlevel.c_str())<=6){
      digitalWrite(LED_BUILTIN,HIGH);
      }else{
        digitalWrite(LED_BUILTIN,LOW);
        }
    client.loop();
    delay(1000);
}
