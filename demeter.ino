#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid     = "Bebes on Parade";
const char* password = "lameepmeep";

WiFiClient espClient;

const char* mqtt_server = "raspberrypi.local";
const String sensorName = "jeff";

PubSubClient client(espClient);

const int AirValue = 150;   //you need to replace this value with Value_1
const int WaterValue = 105;  //you need to replace this value with Value_2
int soilMoistureValue = 0;
int soilmoisturepercent=0;

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

  client.setServer(mqtt_server, 1883);

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

  client.publish(("$SYS/demeter/readings/" + sensorName + "/max").c_str(), String(WaterValue).c_str());
  client.publish(("$SYS/demeter/readings/" + sensorName + "/min").c_str(), String(AirValue).c_str());
}


unsigned long lastMsg = 0;

void loop() {
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg < 2000) {
    return;
  }
  lastMsg = now;

  soilMoistureValue = analogRead(A0);  //put Sensor insert into soil
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);

  if(soilmoisturepercent >= 100) {
    Serial.print(soilMoistureValue);
    Serial.print(" - ");
    Serial.println("100 %");
  } else if(soilmoisturepercent <=0) {
    Serial.print(soilMoistureValue);
    Serial.print(" - ");
    Serial.println("0 %");
  } else if(soilmoisturepercent > 0 && soilmoisturepercent < 100) {
    Serial.print(soilMoistureValue);
    Serial.print(" - ");
    Serial.print(soilmoisturepercent);
    Serial.println("%");
  }

  client.publish(("$SYS/demeter/readings/" + sensorName + "/raw").c_str(), String(soilMoistureValue).c_str());
  client.publish(("$SYS/demeter/readings/" + sensorName + "/percent").c_str(), String(soilmoisturepercent).c_str ());

}
