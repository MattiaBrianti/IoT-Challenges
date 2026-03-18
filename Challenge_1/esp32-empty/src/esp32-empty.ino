#include <WiFi.h>
#include <esp_now.h>

// Define PINs
#define PIR_PIN 27
#define LDR_PIN 34

// MAC receiver
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

esp_now_peer_info_t peerInfo;

// Parameters for DeepSleep
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1.4        /* Personal code ending in 59 */

// Sending callback
void OnDataSent(const wifi_tx_info_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Ok" : "Error");
}

void setup() {
  Serial.begin(115200);
  
  // Setting up the PIR and LDR pins
  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  WiFi.mode(WIFI_STA);
  esp_now_init();

  // send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Peer Registration
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  esp_now_add_peer(&peerInfo);

  // Read the PIR and LDR values
  int motionDetected = digitalRead(PIR_PIN);
  int luminosity = analogRead(LDR_PIN);

  // Create the message to send
  String message;
  if (motionDetected == HIGH) {
    message = "MOTION_DETECTED-LUMINOSITY:" + String(luminosity);
  } else {
    message = "MOTION_NOT_DETECTED-LUMINOSITY:" + String(luminosity);
  }

  // Send the message via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t*)message.c_str(), message.length());

  Serial.println("Message sent: " + message);

  //DAVEDERE
  delay(100); // Short delay to ensure the message is sent before sleeping

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
  //leave empty, as we are using deep sleep and the device will reset after waking up
}
