/**
   This is the Subgap meter for Pewdiepie VS T-Series
   It needs a youtube API key and to put the URL into the web proxy to allow http conenctions
   then just the SSID and password is needed
*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <ArduinoJson.h>

#include <LiquidCrystal_I2C.h>

ESP8266WiFiMulti WiFiMulti;

LiquidCrystal_I2C lcd(0x27, 16, 2); // To setup the LCD

long subGap = 0;
long oldSubGap = 0;

void setup() {

  Serial.begin(115200);
  // outputs the startup screen while it is connecting
  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(4, 0);
  lcd.print("The Subgap");
  lcd.setCursor(8, 1);
  lcd.print("Meter");
  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) { //Waits for the wifi to setup
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("SSID", "PASSWORD");


}


void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // https://www.googleapis.com/youtube/v3/channels?part=statistics&id=UC-lHJZR3Gqxm24_Vd_AJ5Yw%2CUCq-Fj5jknLsUf-MWSy4_brA&key= **Insert Key Here **
    // This is my quick solution to not being able to use HTTPS, it uses a HTTP proxy

    
    // Go to webproxy.stealthy.co and enter the link above, disable URL encryption and paste it below
    // http://webproxy.stealthy.co/index.php?q=https%3A%2F%2Fwww.googleapis.com%2Fyoutube%2Fv3%2Fchannels%3Fpart%3Dstatistics%26id%3DUC-lHJZR3Gqxm24_Vd_AJ5Yw%252CUCq-Fj5jknLsUf-MWSy4_brA%26key%3DAIzaSyBT09vyv5azSeZHYxge7g_ZNQ5ZYZstMZE
    if (http.begin(client, "http://webproxy.stealthy.co/index.php?q=https%3A%2F%2Fwww.googleapis.com%2Fyoutube%2Fv3%2Fchannels%3Fpart%3Dstatistics%26id%3DUC-lHJZR3Gqxm24_Vd_AJ5Yw%252CUCq-Fj5jknLsUf-MWSy4_brA%26key%3DAIzaSyBT09vyv5azSeZHYxge7g_ZNQ5ZYZstMZE")) {  // HTTP
      // start connection and send HTTP header
      int httpCode = http.GET();
      if (httpCode > 0) {

        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          
          char cPayload[900];// Need this for the JSON library because it needs a char

          payload.toCharArray(cPayload, 900);// converts a String to a char 
          
          StaticJsonBuffer<900> jsonBuffer; // Needed for the JSON library, max needed is around 900 chars
          JsonObject& root = jsonBuffer.parseObject(cPayload);
          long pewDiePie; // declares the subcount for pewdiepie and t-series as it is needed to not be in an if else
          long tSeries;
          if(strcmp(root["items"][0]["id"], "UC-lHJZR3Gqxm24_Vd_AJ5Yw") == 0){ // Sometimes youtube switches the order around so this changes it, This is a quick fix
            pewDiePie = root["items"][0]["statistics"]["subscriberCount"];
            tSeries = root["items"][1]["statistics"]["subscriberCount"];
          } else {
            pewDiePie = root["items"][1]["statistics"]["subscriberCount"];
            tSeries = root["items"][0]["statistics"]["subscriberCount"];
          }
          subGap = pewDiePie - tSeries;
          if(subGap != oldSubGap){ // This is to stop it from refreshing when there is no new data
            Serial.print("Sub Gap: "); // Output's the subgap to the serial monitor
            Serial.println(subGap);
            //lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Sub gap:");
            //lcd.setCursor(0, 9);
            //lcd.print("       ");
            lcd.setCursor(8, 0);
            lcd.print(subGap);
            if(subGap < 0){ // This is to make it clearer who is winning
              lcd.setCursor(0, 1);
              lcd.print("RIP PEWDIEPIE");
            } else {
              lcd.setCursor(0, 1);
              lcd.print("PEWDS IS ALIVE");
            }
            oldSubGap = subGap;
          }
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());// for debugging
      }
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  delay(1000);
}
