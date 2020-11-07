#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid     = "Bebes on Parade";
const char* password = "lameepmeep";

WiFiClient espClient;

const char* mqttHost = "raspberrypi.local";
const String sensorName = "jeff";

PubSubClient client(espClient);

int maxBound = 0;
int minBound = 10000;

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps
  pinMode(0, OUTPUT);

    // We start by connecting to a WiFi network
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

    // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    String clientId = "jeff";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected to mqtt");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


unsigned long lastMsgTime = 0;

void loop() {
  client.loop();

  unsigned long now = millis();
  if (now - lastMsgTime < 2000) return;
  lastMsgTime = now;

  int moistureValue = analogRead(A0);

  maxBound = max(maxBound, moistureValue);
  minBound = min(minBound, moistureValue);

  Serial.println("Max: " + String(maxBound));
  Serial.println("Min: " + String(minBound));

  if (maxBound == minBound) return;

  int moisturePct = map(moistureValue, maxBound, minBound, 0, 100);

  Serial.println(String(moistureValue) + " - " + String(moisturePct) + "%");
  Serial.println("");

  client.publish(("$SYS/demeter/readings/" + sensorName + "/max").c_str(), String(maxBound).c_str());
  client.publish(("$SYS/demeter/readings/" + sensorName + "/min").c_str(), String(minBound).c_str());

  client.publish(("$SYS/demeter/readings/" + sensorName + "/raw").c_str(), String(moistureValue).c_str());
  client.publish(("$SYS/demeter/readings/" + sensorName + "/percent").c_str(), String(moisturePct).c_str ());
}
