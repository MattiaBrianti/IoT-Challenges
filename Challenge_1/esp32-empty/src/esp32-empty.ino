#include <WiFi.h>
#include <esp_now.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Define PINs
#define PIR_PIN 27
#define LDR_PIN 34

// MAC receiver (Broadcast address for the SINK node)
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// These constants should match the photoresistor's "gamma" and "rl10" attributes
const float GAMMA = 0.7;
const float RL10 = 50;

esp_now_peer_info_t peerInfo;

// Parameters for Deep Sleep
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1.4      /* Computed as (59%50+5)/10 = 1.4 -- Person code = 10773859 */

RTC_DATA_ATTR int bootCount = 0;

// Callback function to execute when data is sent
void OnDataSent(const wifi_tx_info_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Ok" : "Error");
}

void setup() {
  
  unsigned long t_boot_start = micros();
  

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector

  Serial.begin(115200);

  //++bootCount;
  //Serial.println("Boot number: " + String(bootCount));

  // Setting up the PIR and LDR pins
  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  unsigned long t_boot_end = micros();

  // --- 1. MEASURE SENSING TIME ---
  unsigned long t_sensing_start = micros();

  // Read the PIR and LDR values
  int motionDetected = digitalRead(PIR_PIN);
  int analogValue = analogRead(LDR_PIN);
  
  // Convert the analog value into lux value
  float voltage = analogValue / 4095. * 3.3;
  float resistance = 10000 * voltage / (3.3 - voltage);
  float luminosity = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));

  unsigned long t_sensing_end = micros();

  // --- 2. MEASURE Wi-Fi Initialization Time ---
  unsigned long t_wifi_init_start = micros();
  Serial.println("Enabling WiFi STA");
  WiFi.mode(WIFI_STA);
  esp_now_init();
  
  // Register the send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Peer Registration
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  // Add peer
  esp_now_add_peer(&peerInfo);

  unsigned long t_wifi_init_end = micros();

  // Create the message to send
  String message;
  if (motionDetected == HIGH) {
    message = "MOTION_DETECTED-LUMINOSITY:" + String(luminosity);
  } else {
    message = "MOTION_NOT_DETECTED-LUMINOSITY:" + String(luminosity);
  }

  // --- 2. MEASURE TRANSMISSION TIME ---
  unsigned long t_tx_start = micros();

  // Send the message via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t*)message.c_str(), message.length() + 1);

  unsigned long t_tx_end = micros();

  Serial.println("Message sent: " + message);

  // Short delay to ensure the message is fully sent over the air
  //delay(100);

  // Disable Wi-Fi to save power
  Serial.println("\nDisabling WiFi");
  WiFi.mode(WIFI_OFF);

  // --- 3. MEASURE IDLE TIME (Wi-Fi OFF to Deep Sleep) ---
  unsigned long t_idle_start = micros();

  Serial.println("Entering deep sleep now...");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.flush(); // Ensure all serial data is printed before sleeping

  unsigned long t_idle_end = micros();


  // --- TIMING CALCULATIONS & OUTPUT ---
  unsigned long time_boot = t_boot_end - t_boot_start;
  unsigned long time_wifi_on = t_wifi_init_end - t_wifi_init_start;
  unsigned long time_sensing = t_sensing_end - t_sensing_start;
  unsigned long time_tx = t_tx_end - t_tx_start;
  unsigned long time_idle = t_idle_end - t_idle_start;

  Serial.println("\n--- TIMING BREAKDOWN (in microseconds) ---");
  Serial.print("Boot duration: "); Serial.println(time_boot);
  Serial.print("Sensing duration: "); Serial.println(time_sensing);
  Serial.print("Wi-Fi On duration: "); Serial.println(time_wifi_on);
  Serial.print("Transmission duration: "); Serial.println(time_tx);
  Serial.print("Idle duration (Wi-Fi OFF to Sleep): "); Serial.println(time_idle);
  Serial.println("------------------------------------------\n");

  // Start Deep Sleep
  esp_deep_sleep_start();
}

void loop() {
  // Leave empty, as we are using deep sleep and the device will reset after waking up
}
