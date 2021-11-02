#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include <stdlib.h>

// Statically declare to keep canvas allocated on the stack to save on heap space
// H * W / byte
UBYTE *canvas = new UBYTE[400 * 300 / 8];

/**
 * Fills *data with response from API. Return true on success, false otherwise.
 */
bool fetchDataArray(JsonArray *data) {
  char* ssid     = "Bebes on Parade";
  char* password = "lameepmeep";
  WiFiClient wifiClient;
  
  char* apiUrl = "http://raspberrypi.local:3000/status?ignorePattern=(test|ignore)";
  HTTPClient httpClient;

  Serial.println("Connecting to " + String(ssid));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  String rawJson;

  if (httpClient.begin(wifiClient, apiUrl)) {
    // start connection and send HTTP header
    httpClient.setTimeout(10000);
    int httpCode = httpClient.GET();
    
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET response code: %d\n", httpCode);
      
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        rawJson = httpClient.getString();
        Serial.println(rawJson);
      }
    } else {
      Serial.printf("[HTTP] GET failed, error: %s\n", httpClient.errorToString(httpCode).c_str());
      return false;
    }
    
    httpClient.end();
  } else {
    Serial.println("[HTTP] Unable to connect");
    return false;
  }

  DynamicJsonDocument doc(768);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, rawJson);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }

  *data = doc["data"].as<JsonArray>();
  return true;
}

/**
 * Draws the status of a single sensor
 */
void drawSensorStatus(int index, String name, int value, bool isAlive) {
  Serial.println("[" + String(index) + "] " + name + ": " + String(value) + ", " + String(isAlive));

  if (!isAlive) name = "!" + name;
  int bufferSize = name.length() + 1;
  char nameAsChar[bufferSize];
  name.toCharArray(nameAsChar, bufferSize);

  // Max width = 300
  // Max height = 400

  UWORD startX = 1;
  UWORD startY = 120 + (index * 40);
  UWORD barStartX = 150;
  UWORD barEndX = barStartX + 145;
  UWORD barEndY = startY + 20;

  // Draw the name of the sensor
  Paint_DrawString_EN(
    startX,
    startY,
    nameAsChar,
    &Font20,
    WHITE,
    BLACK
  );

  // Draw the outline of the bar
  Paint_DrawRectangle(
    barStartX, startY,
    barEndX, barEndY,
    BLACK,
    DOT_PIXEL_2X2,
    DRAW_FILL_EMPTY
  );

  UWORD borderWidth = 3;
  UWORD fillBarX = map(value, 0, 100, barStartX + borderWidth, barEndX - borderWidth);

  // Draw the fill of the bar
  Paint_DrawRectangle(
    barStartX + borderWidth, startY + borderWidth,
    fillBarX, barEndY - borderWidth,
    BLACK,
    DOT_PIXEL_1X1,
    DRAW_FILL_FULL
  );
}

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps

  JsonArray data;
  bool success = fetchDataArray(&data);
  if (!success) {
    Serial.println("Failed to fetch data array");
    return;
  }
  int dataSize = data.size();
  Serial.println("Number of sensors: " + String(dataSize));

  Serial.println("Setting up canvas");
  DEV_Module_Init();
  EPD_4IN2_Init();
  EPD_4IN2_Clear();
  DEV_Delay_ms(500);
  Paint_NewImage(canvas, EPD_4IN2_WIDTH, EPD_4IN2_HEIGHT, 0, WHITE);
  Paint_SetRotate(ROTATE_90);
  Paint_Clear(WHITE);

  for (int i = 0; i < dataSize; i++) {
    drawSensorStatus(
      i,
      data[i]["name"].as<String>(),
      data[i]["value"].as<int>(),
      data[i]["isAlive"].as<bool>()
    );
  }

  data.clear();

  Serial.println("Displaying canvas on screen");
  EPD_4IN2_Display(canvas);

  Serial.println("Done");
}

void loop() {
  // Left intentionally blank
}
