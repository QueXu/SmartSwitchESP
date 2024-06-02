#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "QueXu"
#define AIO_KEY         "aio_prGk59hr3knUASMLeSOEDD9kpHJx"

// WiFi
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// MQTT Feeds
String feed = String(AIO_USERNAME) + "/feeds/" + WiFi.macAddress();
Adafruit_MQTT_Subscribe lightControl = Adafruit_MQTT_Subscribe(&mqtt, feed.c_str());

// Relay
#define RELAY_PIN 5

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  WiFiManager wifiManager;

  // Створення SSID з MAC-адресою
  String ssid = "SmartSwitch-" + WiFi.macAddress();

  if (!wifiManager.autoConnect(ssid.c_str())) {
    Serial.println("Failed to connect to WiFi and hit timeout");
    ESP.restart();
  }

  Serial.println("Connected to WiFi");
  Serial.print("Device MAC Address: ");
  Serial.println(WiFi.macAddress());

  mqtt.subscribe(&lightControl);
}

void loop() {
  if (!mqtt.connected()) {
    MQTT_connect();
  }
  mqtt.processPackets(5000);

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &lightControl) {
      Serial.print(F("Got: "));
      Serial.println((char *)lightControl.lastread);
      if (strcmp((char *)lightControl.lastread, "ON") == 0) {
        digitalWrite(RELAY_PIN, HIGH);
      } else if (strcmp((char *)lightControl.lastread, "OFF") == 0) {
        digitalWrite(RELAY_PIN, LOW);
      }
    }
  }
}

void MQTT_connect() {
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("MQTT Connected!");
}
