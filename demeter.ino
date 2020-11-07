#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RTCVars.h>
#include <Arduino.h>

extern "C" {
  #include <user_interface.h>
}



const char* ssid     = "Bebes on Parade";
const char* password = "lameepmeep";

WiFiClient espClient;

const char* mqttHost = "raspberrypi.local";
const String sensorName = "jeff";

PubSubClient client(espClient);

int maxBound = 100;
int minBound = 1024;

RTCVars state; // state object for saving values between resets

void blink() {
  digitalWrite(0, HIGH);
  delay(500);
  digitalWrite(0, LOW);
}

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps
  pinMode(0, OUTPUT);

  // Load in stored values if available

  state.registerVar(&maxBound);
  state.registerVar(&minBound);

  if (state.loadFromRTC()) {
    Serial.println('able to load in max/min values');
  } else {
    Serial.println('cold boot');
  }

  // Reset stored values if a hard reset is detected

  rst_info *resetInfo;
  resetInfo = ESP.getResetInfoPtr();

  if (resetInfo->reason != REASON_DEEP_SLEEP_AWAKE) {
    Serial.println("Hard reset detected. Resetting max (100) and min bounds (1024)");
    maxBound = 100;
    minBound = 1024;
  }

}

void loop() {
  int moistureValue = analogRead(A0);

  maxBound = max(maxBound, moistureValue);
  minBound = min(minBound, moistureValue);
  state.saveToRTC();

  Serial.println("Max: " + String(maxBound));
  Serial.println("Min: " + String(minBound));

  if (maxBound == minBound) {
    delay(200);
    return;
  }

  int moisturePct = map(moistureValue, maxBound, minBound, 0, 100);

  Serial.println(String(moistureValue) + " - " + String(moisturePct) + "%");
  Serial.println("");

  // publish(moistureValue, moisturePct);
  //delay(2000);
  //return;
  Serial.println("Sleeping for 5 seconds");
  // Deep sleep for 5 seconds
  ESP.deepSleep(5e6);
}

void publish(int moistureValue, int moisturePct) {
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
      Serial.println("Reached max MQTT connection attempts (" + String(attempts) + "/" + String(maxAttempts) + ")");
      Serial.println("Failed to publish");
      return;
    }

    Serial.print("Attempting MQTT connection (" + String(attempts) + "/" + String(maxAttempts) + ")");

    String clientId = "sensor_" + sensorName;

    if (client.connect(clientId.c_str())) {
      Serial.println("connected to mqtt");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }

  client.loop();

  client.publish(("$SYS/demeter/readings/" + sensorName + "/max").c_str(), String(maxBound).c_str());
  client.publish(("$SYS/demeter/readings/" + sensorName + "/min").c_str(), String(minBound).c_str());

  client.publish(("$SYS/demeter/readings/" + sensorName + "/raw").c_str(), String(moistureValue).c_str());
  client.publish(("$SYS/demeter/readings/" + sensorName + "/percent").c_str(), String(moisturePct).c_str ());

  blink();
}

