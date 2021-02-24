#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>

extern "C" {
  #include <user_interface.h>
}

int sleepTime = 1e6 * 60 * 60 * 6;
const String sensorName = "jeff";

const char* ssid     = "Bebes on Parade";
const char* password = "lameepmeep";

WiFiClient espClient;

const char* mqttHost = "raspberrypi.local";
PubSubClient client(espClient);

void blink() {
  digitalWrite(0, HIGH);
  delay(500);
  digitalWrite(0, LOW);
}

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps
  pinMode(0, OUTPUT);
}

void loop() {
  int moistureValue = analogRead(A0);
  Serial.println("Recorded value: " + String(moistureValue));
  publish(moistureValue);

  Serial.println("Sleeping for " + String(sleepTime) + "uS");
  ESP.deepSleep(sleepTime);
}

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

  blink();
}
