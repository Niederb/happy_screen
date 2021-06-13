#include <EEPROM.h>

/*
   Inkplate_SD_BMP example for e-radionica Inkplate6
   For this example you will need a micro USB cable, Inkplate6 and a SD card loaded with
   image1.bmp and image2.bmp file that can be found inside folder of this example.
   Select "Inkplate 6(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 6(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/
   To work with SD card on Inkplate, you will need to add one extra library.
   Download and install it from here: https://github.com/e-radionicacom/Inkplate-6-SDFat-Arduino-Library
   You can open .bmp, .jpeg or .png files that have color depth of 1 bit (BW bitmap), 4 bit, 8 bit and
   24 bit AND have resoluton smaller than 800x600 or otherwise it won't fit on screen.
   Format your SD card in standard FAT fileformat.
   This example will show you how you can read .bmp and .jpeg files (pictures) from SD card and
   display that image on e-paper display.
   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   15 July 2020 by e-radionica.com
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_ESP32_DEV
#error "Wrong board selection for this example, please select Inkplate 6 in the boards menu."
#endif

#include "Inkplate.h"            //Include Inkplate library to the sketch
#include "SdFat.h"               //Include library for SD card
Inkplate display(INKPLATE_3BIT); // Create an object on Inkplate library and also set library into 1 Bit mode (BW)
SdFile file;                     // Create SdFile object used for accessing files on SD card

#include <time.h>
#include <string.h>
#include <stdio.h>

char *ssid = "dfn-06184";
char *pass = "2ddc-cy79-omp4-cpxg";

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

double seconds_to_hour(int hour) {
    // Get current time (now)
    time_t now = time(NULL);
    int time_zone = 2;

    struct tm specific_hour_time;
    struct tm now_time;
    gmtime_r(&now, &specific_hour_time);
    gmtime_r(&now, &now_time);

    specific_hour_time.tm_hour = hour - time_zone;
    specific_hour_time.tm_min = 0;
    specific_hour_time.tm_sec = 0;

    Serial.println(asctime(&now_time));
    Serial.println(asctime(&specific_hour_time));

    //display.println(asctime(&specific_hour_time));

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

    int randNumber = random(34);
    char buffer[13];
    sprintf(buffer, "image%03d.jpg", randNumber);
    showImage(buffer);
    
    
    double time_to_four = seconds_to_hour(4);
    Serial.println("Going to sleep for seconds");
    Serial.println(time_to_four);
    esp_sleep_enable_timer_wakeup(time_to_four * 1000 * 1000);
    //esp_sleep_enable_timer_wakeup(24 * 3600 * 1000 * 1000);
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


void loop()
{
    // Nothing...
}
