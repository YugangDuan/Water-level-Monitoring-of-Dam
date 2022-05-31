#include <ESP8266WiFi.h>
#include <espnow.h>

#define rainAnalog A0
#define rainDigital D1

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xE8,0xDB,0x84,0xAF,0x5C,0xCD};//接收方MAC地址

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int rainFall;
  int isRain;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Callback when data is sent
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
  // Init Serial Monitor
  Serial.begin(115200);
  pinMode(rainDigital,INPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void loop() {
  // Set values to send
  int rainAnalogVal = analogRead(rainAnalog);
  int rainDigitalVal = digitalRead(rainDigital);

  Serial.print(rainAnalogVal);
  Serial.print("\t");
  Serial.println(rainDigitalVal);
  myData.rainFall = 1024 - rainAnalogVal;
  myData.isRain = rainDigitalVal;
  // Send message via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  delay(2000);
}
