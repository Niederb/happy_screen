#include <EEPROM.h>

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_ESP32_DEV
#error "Wrong board selection for this example, please select Inkplate 6 in the boards menu."
#endif

#include "Inkplate.h"
#include "driver/rtc_io.h"
#include "SdFat.h"
#include <esp_wifi.h>
#include "driver/adc.h"

Inkplate display(INKPLATE_3BIT); // Create an object on Inkplate library and also set library into 1 Bit mode (BW)
SdFile file;                     // Create SdFile object used for accessing files on SD card

#include <time.h>
#include <string.h>
#include <stdio.h>

/*
// Create a config.h file with the following contents
#pragma once
char *ssid = ""; // SSD of your WIFI network
char *pass = ""; // Password to your WIFI network
int time_zone = 2; // Your UTC timezone
int change_hour = 4; // At which hour you want to change the image (range 0 to 23)
*/
#include "config.h"

/// Connect to the WIFI network specified in config.h
void connect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    int cnt = 0;
    Serial.print(F("Waiting for WiFi to connect..."));
    while ((WiFi.status() != WL_CONNECTED))
    {
        Serial.print(F("."));
        delay(1000);
        ++cnt;

        if (cnt == 20)
        {
            Serial.println("Can't connect to WIFI, restarting");
            delay(100);
            ESP.restart();
        }
    }
    Serial.println(F(" connected"));
}

/// Synchronize device time with a time server
void syncTime()
{
    // Used for setting correct time
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print(F("Waiting for NTP time sync: "));
    time_t nowSecs = time(nullptr);
    while (nowSecs < 8 * 3600 * 2)
    {
        // Print a dot every half a second while time is not set
        delay(500);
        Serial.print(F("."));
        yield();
        nowSecs = time(nullptr);
    }

    Serial.println();

    // Used to store time info
    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);

    Serial.print(F("Current time: "));
    Serial.print(asctime(&timeinfo));
}

/// Calculate the number of seconds until hour is next time
/// For example: How many seconds until 18:00 (today or tomorrow, depending if 18:00 already passed today)
/// 24 hour time format
double seconds_to_hour(int hour) {
    // Get current time (now)
    time_t now = time(NULL);

    struct tm specific_hour_time;
    struct tm now_time;
    gmtime_r(&now, &specific_hour_time);
    gmtime_r(&now, &now_time);

    specific_hour_time.tm_hour = hour - time_zone;
    specific_hour_time.tm_min = 0;
    specific_hour_time.tm_sec = 0;

    Serial.println(asctime(&now_time));
    Serial.println(asctime(&specific_hour_time));

    double delta = difftime(mktime(&specific_hour_time), now);
    Serial.println(delta);
    if (delta < 0) {
      return 24.0 * 3600.0 - abs(delta);
    } else {
      return delta;
    }
}

void setup()
{
    Serial.begin(115200);
    randomSeed(analogRead(0));

    display.begin();        // Init Inkplate library (you should call this function ONLY ONCE)
    display.clearDisplay(); // Clear frame buffer of display
    display.display();      // Put clear image on display

    // Init SD card. Display if SD card is init propery or not.
    if (!display.sdCardInit())
    {
        // If SD card init not success, display error on screen and stop the program (using infinite loop)
        display.println("SD Card error!");
        display.partialUpdate();
        return;
    }

    display.println("SD Card OK! Reading image...");
    display.partialUpdate();
    connect();
    syncTime();

    // Shutdown WIFI and GPIO to save power
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    rtc_gpio_isolate(GPIO_NUM_12);
    esp_wifi_stop();  
    adc_power_off();

    Serial.println("Battery level:");
    Serial.println(display.readBattery());

    // Max value is exclusive
    int number_files = countFiles();
    int randNumber = random(1, number_files + 1);
    char buffer[13];
    sprintf(buffer, "image%03d.jpg", randNumber);
    showImage(buffer);

    double time_to_change = seconds_to_hour(change_hour);
    Serial.println("Going to sleep for seconds");
    Serial.println(time_to_change);

    esp_sleep_enable_timer_wakeup(time_to_change * 1000 * 1000);
    esp_deep_sleep_start(); 
}

void showImage(const String& image_name)
{
    if (!display.drawImage(image_name, 0, 0, true, false))
    {
        // If is something failed (wrong filename or wrong format), write error message on the screen.
        // You can turn off dithering for somewhat faster image load by changing the fifth parameter to false, or
        // removing the parameter completely
        display.clearDisplay();
        display.println("Image open error");
        display.display();
    }
    display.display();
}

// Count the number of files in the root directory
int countFiles() {
  File root;
  int root_file_count = 0;
  if (!root.open("/")) {
    return 0;
  }
  while (file.openNext(&root, O_RDONLY)) {
    if (!file.isHidden()) {
      root_file_count++;
    }
    file.close();
  }
  Serial.println("Number of files in root:");
  Serial.println(root_file_count);
  return root_file_count;
}

void loop()
{
    // Nothing...
}
