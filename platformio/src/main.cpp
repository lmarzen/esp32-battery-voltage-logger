#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <WiFi.h>

#define ADC_PIN A2
#define MAX_READINGS 384 // maximum number of samples that can be stored
#define SAMPLE_FREQ 30 // sample frequency in minutes, must be a positive integer

Preferences prefs;

// clear all non-volatile storage
void reformat_nvs()
{
  nvs_flash_erase(); // erase the NVS partition and...
  nvs_flash_init();  // initialize the NVS partition.
  while (true);
}

void setup()
{
  esp_sleep_enable_timer_wakeup(60 * 30 * 1000000LL);
  esp_deep_sleep_start();

  Serial.begin(115200);

  // Open namesspaces for read/write
  prefs.begin("adc-log", false);
  prefs.begin("index", false);

  // print current voltage to terminal
  // while (true)
  // {
  //   Serial.println( ((double) analogRead(ADC_PIN)) / 1000.0 * (3.3 / 2.0) );
  //   delay(2000);
  // }

  // Remove all preferences(logged values) under the opened namespace
  // prefs.clear(); // uncomment this to reset all values
  // Serial.println("Cleared prefs.");
  // delay(30000);
}

bool isPrime(int32_t N)
{
  for (int32_t i = 2; i * i <= N; i++)
    if (N % i == 0)
      return false;
  return true;
}
int32_t countPrimes(int32_t N)
{
  if (N < 3)
    return 0;
  int32_t cnt = 1;
  for (int32_t i = 3; i < N; i += 2)
    if (isPrime(i))
      cnt++;
  return cnt;
}

// Takes about 30 minutes for each loop
void loop()
{
  unsigned long startTime = millis();
  delay(1000);

  // Read existing values from non-volatile storage
  int index = prefs.getInt("index", 0);
  uint16_t adc_readings[MAX_READINGS] = {};
  prefs.getBytes("adc-log", adc_readings, sizeof(uint16_t) * MAX_READINGS);

  // Collect new adc sample
  adc_readings[index] = analogRead(ADC_PIN);

  // Print all readings to serial
  Serial.println("ADC Reading History");
  Serial.println("v---------------");
  for (int i = 0; i <= index; ++i)
  {
    Serial.println(adc_readings[i]);
  }
  Serial.println("^---------------");

  if (index >= MAX_READINGS)
  {
    Serial.println("MAX READINGS REACHED!!! Going to sleep...");
    esp_sleep_enable_timer_wakeup((60 * 30) * 1000000LL);
    esp_deep_sleep_start();
  }

  ++index;

  // Write adc-log and index back to non-volatile storage
  prefs.putBytes("adc-log", adc_readings, sizeof(uint16_t) * MAX_READINGS);
  prefs.putInt("index", index);

  // Turn wifi on to increase power-draw
  WiFi.mode(WIFI_STA);
  WiFi.begin("notarealssid", "notarealpassword");

  // Do some calculations to increase power-draw
  Serial.print("Calculating how many primes exist under 5,750,000... (x");
  Serial.print(SAMPLE_FREQ);
  Serial.println(")");
  long primeTime = millis();
  for (int j = 0; j < SAMPLE_FREQ; ++j)
  {
    Serial.println(countPrimes(5750000L)); // takes ~58s for esp32 at 240MHz
  }
  Serial.println("Time to calculate primes:" + String((millis() - primeTime) / 1000.0, 3) + "s");

  // disable wifi so that we do not conflict with adc pins
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);

  Serial.println("Calculations completed in: " + String((millis() - startTime) / 1000.0, 3) + "s");
  // make sure that samples are taken exactly 30 minutes apart
  unsigned long delayTime = (SAMPLE_FREQ * 60 * 1000) - (millis() - startTime);
  Serial.println("Waiting: " + String(delayTime / 1000.0, 3) + "s...");
  delay(delayTime);
  Serial.println("Total loop time: " + String((millis() - startTime) / 1000.0, 3) + "s");
}
