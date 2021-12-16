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
int LIMIT = 10;
int DATA_ARRAY_SPACE = 1024;

/**
 * Fills *dataArray with response from API. Return true on success, false otherwise.
 */
bool fetchDataArray(DynamicJsonDocument *dataArray) {
  String ssid     = "Bebes on Parade";
  String password = "lameepmeep";
  WiFiClient wifiClient;
  
  String apiUrl = "http://raspberrypi.local:3000/status?compact=true&count=" + String(LIMIT);
  HTTPClient httpClient;

  Serial.println("Connecting to " + String(ssid));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");

  String rawJson;

  Serial.println("Begin GET");

  if (httpClient.begin(wifiClient, apiUrl)) {

    httpClient.setTimeout(30000); // 30 seconds
    int httpCode = httpClient.GET();
    
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET response code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        rawJson = httpClient.getString();
        Serial.println("Raw JSON: " + rawJson);
      }
    } else {
      Serial.printf("HTTP GET failed, error: %s\n", httpClient.errorToString(httpCode).c_str());
      return false;
    }
    
    httpClient.end();
  } else {
    Serial.println("Unable to connect via HTTP");
    return false;
  }

  // Parse JSON object
  DeserializationError error = deserializeJson(*dataArray, rawJson);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }

  return true;
}

/**
 * Initializes display for drawing
 */
void initializeDisplay() {
  DEV_Module_Init();
  EPD_4IN2_Init();
  EPD_4IN2_Clear();
  DEV_Delay_ms(500);
  Paint_NewImage(canvas, EPD_4IN2_WIDTH, EPD_4IN2_HEIGHT, 0, WHITE);
  Paint_SetRotate(ROTATE_90);
  Paint_Clear(WHITE);
}

/**
 * Draws the status of a single sensor
 */
void drawSensorStatus(int index, String name, int value, int isAlive) {
  Serial.println("Drawing [" + String(index) + "] " + name + ": " + String(value) + ", " + String(isAlive));

  if (isAlive == 0) name = "!" + name;
  int bufferSize = name.length() + 1;
  char nameAsChar[bufferSize];
  name.toCharArray(nameAsChar, bufferSize);

  // Max width = 300
  // Max height = 400

  UWORD startX = 1;
  UWORD startY = 10 + (index * 30);
  UWORD barStartX = 150;
  UWORD barEndX = barStartX + 145;
  UWORD barEndY = startY + 20;

  // Draw the name of the sensor
  Paint_DrawString_EN(
    startX,
    startY + 1,
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

  UWORD borderWidth = 4;
  UWORD fillBarX = map(value, 0, 100, barStartX + borderWidth, barEndX - borderWidth);

  // Draw the fill of the bar
  Paint_DrawRectangle(
    barStartX + borderWidth, startY + borderWidth,
    fillBarX, barEndY - borderWidth + 1,
    BLACK,
    DOT_PIXEL_1X1,
    DRAW_FILL_FULL
  );
}

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps

  DynamicJsonDocument dataArray(DATA_ARRAY_SPACE);
  bool success = fetchDataArray(&dataArray);
  if (!success) {
    Serial.println("Failed to fetch data array");
    return;
  }

  int dataSize = dataArray.size();
  Serial.println("Number of sensors: " + String(dataSize));

  initializeDisplay();

  for (int i = 0; i < dataSize; i++) {
    JsonObject obj = dataArray[i].as<JsonObject>();

    drawSensorStatus(
      i,
      obj["n"],     // sensor.name
      obj["v"],     // moistureLevel
      obj["a"]      // isAlive as 0,1
    );
  }

  Serial.println("Displaying canvas on screen");
  EPD_4IN2_Display(canvas);

  dataArray.clear();
  Serial.println("Done");
}

void loop() {
  // Left intentionally blank
}
