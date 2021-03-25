#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>

extern "C" {
  #include <user_interface.h>
}

const String sensorName = "test-2x3h";

const char* ssid     = "Bebes on Parade";
const char* password = "lameepmeep";

WiFiClient espClient;

const char* mqttHost = "raspberrypi.local";
PubSubClient client(espClient);

// Publishes moisture value
void publish(int moistureValue) {
  Serial.println("Connecting to " + String(ssid));

  delay(100);
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());

  client.setServer(mqttHost, 1883);

  int attempts = 0;
  int maxAttempts = 3;

  // Loop until we're reconnected
  while (!client.connected()) {
    if (attempts++ >= maxAttempts) {
      Serial.println("Reached max MQTT connection attempts");
      Serial.println("Failed to publish");
      return;
    }

    Serial.print("Attempting MQTT connection (" + String(attempts) + "/" + String(maxAttempts) + "): ");

    String clientId = "sensor_" + sensorName;

    if (client.connect(clientId.c_str())) {
      Serial.println("connected to mqtt");
    } else {
      Serial.println("failed, rc=" + String(client.state()) + " trying again in 5 seconds");
      delay(5000);
    }
  }

  client.loop();

  client.publish(("$SYS/demeter/readings/" + sensorName + "/raw").c_str(), String(moistureValue).c_str());
  Serial.println("Published successfully");

  client.disconnect();
}


// Enter max deep sleep cycle the provided number of times. This is usually around 3.5 hours
void deepSleepCycles(uint32_t numCycles) {
  // Read counter from memory
  uint32_t resetCounter = 0;
  ESP.rtcUserMemoryRead(0, &resetCounter, sizeof(resetCounter));
  // Increment counter
  resetCounter++;
  
  Serial.println("Deep sleep progress: " + String(resetCounter) + "/" + String(numCycles));

  ESP.rtcUserMemoryWrite(0, &resetCounter, sizeof(resetCounter));

  if (resetCounter <= numCycles) {
//    ESP.deepSleep(ESP.deepSleepMax());
    ESP.deepSleep(1e6 * 60 * 60 * 3);
  } else {
    Serial.println("Deep sleep complete!");
  }

  resetSleepCounter();
}

void resetSleepCounter() {
  uint32_t resetCounter = 0;
  ESP.rtcUserMemoryWrite(0, &resetCounter, sizeof(resetCounter));
}

/////////////////////
// Start main loop //
/////////////////////

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps

  // Reset counter if hard reset is detected

  Serial.println("Reset reason: " + ESP.getResetReason());

  if (ESP.getResetReason() != "Deep-Sleep Wake") {
    Serial.println("Hard reset: zeroing reset counter.");
    resetSleepCounter();
  } else {
    // Sleep for 2 cycles, 7-8 hours
    deepSleepCycles(2);
  }

  // Read and push sensor value
  int moistureValue = analogRead(A0);
  Serial.println("Recorded value: " + String(moistureValue));
  publish(moistureValue);

  
  // Sleep for 2 cycles, 7-8 hours
  deepSleepCycles(2);
}

void loop() {
  // deep sleep restarts esp, setup() acts as loop
}
