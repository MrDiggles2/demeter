#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>

extern "C" {
  #include <user_interface.h>
}

const String sensorName = "jeff";

const char* ssid     = "Bebes on Parade";
const char* password = "lameepmeep";

WiFiClient espClient;

const char* mqttHost = "raspberrypi.local";
PubSubClient client(espClient);

void blink() {
  digitalWrite(0, LOW);
  delay(500);
  digitalWrite(0, HIGH);
}

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

// From https://github.com/ohel/analogging/blob/master/src/main.cpp
// Sleeps for provided number of hours, with special consideration when reset is triggered
void deepSleepCycle(uint32_t hours, bool end_of_setup = false) {

  uint32_t reset_counter = 0;
  bool waking_from_sleep = ESP.getResetReason() == "Deep-Sleep Wake";

  if (!end_of_setup) {
    if (waking_from_sleep) {
        ESP.rtcUserMemoryRead(0, &reset_counter, sizeof(reset_counter));
        reset_counter++;
        ESP.rtcUserMemoryWrite(0, &reset_counter, sizeof(reset_counter));
        Serial.println("Waking up from deep-sleep, progress: " + String(reset_counter) + "/" + String(hours));
    } else {
        ESP.rtcUserMemoryWrite(0, &reset_counter, sizeof(reset_counter));
        Serial.println("Hard reset: zeroing reset counter.");
        return;
    }
  }

  // With larger values, deep-sleep is unrealiable: it might never wake up and consume a lot of power.
  // Therefore sleep one hour at a time.
  // In reality, the ESP sleeps a bit less than the 60 minutes it is told to.
  if (reset_counter < hours) {
    // If this is the first time going to sleep, do the radio calibration once.
    // Otherwise, disable radio (WiFi).
    RFMode wake_mode = waking_from_sleep ? WAKE_RF_DISABLED : WAKE_RFCAL;
    if (reset_counter + 1 == hours) {
        // Wake up with radio on if the next power cycle finishes sleeping.
        wake_mode = WAKE_NO_RFCAL;
    }

    // 1: WAKE_RFCAL
    // 2: WAKE_NO_RFCAL
    // 4: WAKE_RF_DISABLED
    Serial.println("Radio mode will be: " + String(wake_mode));
    ESP.deepSleep(3600 * 1e6, wake_mode);
  }
  reset_counter = 0;
  ESP.rtcUserMemoryWrite(0, &reset_counter, sizeof(reset_counter));

}

/////////////////////
// Start main loop //
/////////////////////

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps
  pinMode(0, OUTPUT);

  deepSleepCycle(12);

  int moistureValue = analogRead(A0);
  Serial.println("Recorded value: " + String(moistureValue));
  publish(moistureValue);
  blink();

  deepSleepCycle(12, true);
}

void loop() {
  // deep sleep restarts esp, setup() acts as loop
}
