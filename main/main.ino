#define HTTP_PORT 80


#include <Ethernet.h>
#include <SPI.h>
#include <ArduinoMqttClient.h>


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 177 };
byte dns[] = { 192, 168, 0, 1 };

const char mqttBroker[] = "test.mosquitto.org";
const int mqttPort = 1883;
const char mqttTopic[] = "pixelio/12345";

EthernetClient ethernetClient;
MqttClient mqttClient(ethernetClient);


void setup();
void loop();
boolean init_ethernet();
void onMqttMessage(int messageSize);


void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    delay(1);
  }
  Serial.println("Serial initialized successfully.");

  if (!init_ethernet())
  {
    for (;;)
    {
      delay(1);
    }
  }

  Serial.print("Connecting to the MQTT broker: \"");
  Serial.print(mqttBroker);
  Serial.println("\".");
  if (!mqttClient.connect(mqttBroker, mqttPort)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }
  Serial.println("Connected to the MQTT broker successfully.");
  
  mqttClient.onMessage(onMqttMessage);

  Serial.print("Subscribing to topic \"");
  Serial.print(mqttTopic);
  Serial.println("\".");
  mqttClient.subscribe(mqttTopic);

  Serial.println();
}


void loop()
{
  
  if (!ethernetClient.connected())
  {
    Serial.println();
    Serial.println("disconnecting...");
    ethernetClient.stop();
    for (;;)
    {
      delay(1);
    }
  }

  mqttClient.poll();
}


boolean init_ethernet()
{
  Serial.println("Initialize Ethernet with DHCP:");
  
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");

    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.println("Ethernet shield was not found.");
      
      return false;
    }
    
    if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.println("Ethernet cable is not connected.");
    }
    
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, dns);
  }
  else
  {
    Serial.print("DHCP assigned IP: ");
    Serial.println(Ethernet.localIP());
  }

  Serial.println("Ethernet shield was successfully initialized.");

  return true;
}


void onMqttMessage(int messageSize)
{
  Serial.println("Received a message with topic \"");
  Serial.print(mqttClient.messageTopic());
  Serial.print("\", length: \"");
  Serial.print(messageSize);
  Serial.println("\", bytes:");

  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();
  Serial.println();
}