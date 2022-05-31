#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const int trigPin = 12;//d6
const int echoPin = 14;//d5

//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;
int i = 0;
// WiFi
const char *ssid = "B-207@Museum_walk"; // Enter your WiFi name
const char *password = "GeaZvZqEp";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "distanced";
const char *topic_3 = "8266rate";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;
  
  WiFiClient espClient;
  PubSubClient client(espClient);
int timeRange = 30000;

void setup() {
  Serial.begin(115200); // Starts the serial communication
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

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
}
void collect(){
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm =16.18 - duration * SOUND_VELOCITY/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance on the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);
  // publish and subscribe
}
void reconnect() {
  while (!client.connected()) {
    if (client.connect("8266",mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // 等5秒之後再重試
    }
  }
}
void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    char chTemp[1024] = {};
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
        chTemp[i] = (char)payload[i];
    }
    Serial.println();
    String ss = chTemp;
    timeRange = atoi(ss.c_str());
    memset(chTemp, 0x00, 1024);
    Serial.println();
    Serial.println("-----------------------");
}

void loop() {
  if(i == timeRange ||i == 30000){
    collect();
    i=0;
  }
  i+=3000;
  if (!client.connected()) {
    reconnect();
  }
  client.publish(topic, String(distanceCm).c_str(), true);
  client.loop();
  delay(3000);
}
