/*
ESP Siren for bitlair
Kartoffel
*/
#include <limits.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

// WiFi settings
const char ssid[] = "";                     //  your network SSID (name)
const char pass[] = "";                     // your network password
const char* mqtt_server = "mqtt.bitlair.nl";

// max 10
#define numAlarmPins 6
const uint8_t alarmPins[numAlarmPins] = {D1, D2, D5, D6, D7, D0};
const unsigned long alarmOnDuration = 2000;

const int BAUD_RATE   = 115200;             // serial baud rate

// MQTT stuff
WiFiClient espClient;
PubSubClient client(espClient);
const char* mqttTopic = "bitlair/siren";
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  for (uint8_t i = 0; i < numAlarmPins; i++) {
    pinMode(alarmPins[i], OUTPUT);
    digitalWrite(alarmPins[i], LOW);
  }

  Serial.begin(BAUD_RATE);
  Serial.println();
  Serial.println("MQTT Alarm");

  Serial.print("Connecting to ");

  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    Serial.print(".");
  }
  Serial.println("");

  Serial.print("WiFi connected to: ");
  Serial.println(ssid);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void alarm(uint8_t alarmNum) {
  digitalWrite(alarmPins[alarmNum], HIGH);
  delay(alarmOnDuration);
  digitalWrite(alarmPins[alarmNum], LOW);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] < 48 || (char)payload[0] > 57)
    return;

  uint8_t alarmNum = (char)payload[0] - 48;
  Serial.print("Firing alarm ");
  Serial.println(alarmNum);
  alarm(alarmNum);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      snprintf (msg, 75, "Hello world! #%ld", value);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish(mqttTopic, msg);
      ++value;
      // ... and resubscribe
      client.subscribe(mqttTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  yield();
}
