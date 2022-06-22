#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <WiFi.h>

Preferences prefs;

// clear all non-volatile storage
void reformat_nvs() {
  nvs_flash_erase(); // erase the NVS partition and...
  nvs_flash_init(); // initialize the NVS partition.
  while(true);
}

#define MAX_READINGS 196


void setup() {
  //esp_sleep_enable_timer_wakeup((60*30) * 1000000LL);
  //esp_deep_sleep_start();

  Serial.begin(115200);

  // Open namesspaces for read/write
  prefs.begin("adc-log", false);
  prefs.begin("index", false);

  // Remove all preferences(logged values) under the opened namespace
  prefs.clear(); // uncomment this to reset all values
}

bool isPrime(int32_t N)
{
    for(int32_t i = 2 ; i * i <= N ; i++)
        if(N % i == 0)
            return false;
    return true;
}
int32_t countPrimes(int32_t N)
{
    if(N < 3)
        return 0;
    int32_t cnt = 1;
    for(int32_t i = 3 ; i < N ; i += 2)
        if(isPrime(i))
            cnt++;
    return cnt;
}

void loop() 
{ 
  delay(1000);

  // Read existing values from non-volatile storage
  int index = prefs.getInt("index", 0);
  uint16_t adc_readings[MAX_READINGS] = {};
  prefs.getBytes("adc-log", adc_readings, sizeof(uint16_t) * MAX_READINGS);

  // Collect new adc sample
  adc_readings[index] = analogRead(15);

  // Print all readings to serial
  Serial.println("v---------------");
  for (int i = 0; i <= index; ++i) {
    Serial.println(adc_readings[i]);
  }
  Serial.println("^---------------");

  if (index >= MAX_READINGS) {
    Serial.println("MAX READINGS REACHED!!! going to sleep");
    esp_sleep_enable_timer_wakeup((60*30) * 1000000LL);
    esp_deep_sleep_start();
  }

  ++index;

  // Write adc-log and index back to non-volatile storage
  prefs.putBytes("adc-log", adc_readings, sizeof(uint16_t) * MAX_READINGS);
  prefs.putInt("index", index);

  // Turn wifi on to increase power-draw
  WiFi.mode(WIFI_STA);
  WiFi.begin("notarealssid","notarealpassword");

  // Do some calculations to increase power-draw
  Serial.println("Calculating how many primes exist under 10,000,000... (10x)");
  for (int j = 0; j < 30; ++j) {
    Serial.println(countPrimes(10000000L));
  }

  // disable wifi so that we do not conflict with adc pins
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(1000 * 60);
}
