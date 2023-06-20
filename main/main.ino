#define HTTP_PORT 80
#define LED_DIN_PIN 7
#define NUM_LEDS_X 16
#define NUM_LEDS_Y 16


#include <Ethernet.h>
#include <SPI.h>
#include <ArduinoMqttClient.h>
#include <FastLED.h>


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 177 };
byte dns[] = { 192, 168, 0, 1 };

const char mqttBroker[] = "test.mosquitto.org";
const int mqttPort = 1883;
const char mqttTopic[] = "pixelio/12345";

EthernetClient ethernetClient;
MqttClient mqttClient(ethernetClient);

CRGB leds[NUM_LEDS_X * NUM_LEDS_Y];
byte matrixBuffer[NUM_LEDS_X * NUM_LEDS_Y * 3];


void setup();
void loop();
boolean init_ethernet();
void onMqttMessage(int messageSize);
void init_leds();
void show_matrix();
void set_pixel(int x, int y, CRGB color);


void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    delay(1);
  }
  Serial.println("Serial initialized successfully.");

  Serial.println("Initializing matrix...");
  init_leds();
  for (int y = 0; y < NUM_LEDS_Y; y++)
  {
    for (int x = 0; x < NUM_LEDS_X; x++)
    {
      int matrixBufferLedIndex = y * NUM_LEDS_X + x;
      matrixBuffer[matrixBufferLedIndex] = 0;
      matrixBuffer[matrixBufferLedIndex + 1] = 0;
      matrixBuffer[matrixBufferLedIndex + 2] = 0;
    }
  }

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
  Serial.print("Received a message with topic \"");
  Serial.print(mqttClient.messageTopic());
  Serial.print("\", length: \"");
  Serial.print(messageSize);
  Serial.println("\".");
  
  if (messageSize != NUM_LEDS_X * NUM_LEDS_Y * 3)
  {
    Serial.print("Message size is not equal to required. ");
    Serial.println("Message processing canceled.");
    return;
  }

  Serial.println("Processing Message payload...");
  for (int matrixBufferLedIndex = 0;
    mqttClient.available() && matrixBufferLedIndex < NUM_LEDS_X * NUM_LEDS_Y * 3;
    matrixBufferLedIndex++)
  {
    matrixBuffer[matrixBufferLedIndex] = (byte)mqttClient.read();
  }
  Serial.println("Message processed successfully. Printing Matrix.");
  show_matrix();
  Serial.println();
}


void init_leds()
{
  FastLED.addLeds<WS2812, LED_DIN_PIN, GRB>(leds, NUM_LEDS_X * NUM_LEDS_Y);

  for (int ledIndex = 0; ledIndex < NUM_LEDS_X * NUM_LEDS_Y; ledIndex++)
  {
    leds[ledIndex] = CRGB(0, 0, 0);
  }
  FastLED.show();
}


void show_matrix()
{
  for (int y = 0; y < NUM_LEDS_Y; y++)
  {
    for (int x = 0; x < NUM_LEDS_X; x++)
    {
      int matrixBufferLedIndex = (y * NUM_LEDS_X + x) * 3;
      byte colorR = matrixBuffer[matrixBufferLedIndex];
      byte colorG = matrixBuffer[matrixBufferLedIndex + 1];
      byte colorB = matrixBuffer[matrixBufferLedIndex + 2];
      CRGB color = CRGB(colorR, colorG, colorB);
      set_pixel(x, y, color);
    }
  }

  FastLED.show();
  Serial.println("Matrix printing finished.");
}


void set_pixel(int x, int y, CRGB color)
{
  int ledIndex;
  if ((NUM_LEDS_Y - y) % 2 == 0)
  {
    ledIndex = (NUM_LEDS_Y - y) * NUM_LEDS_X - 1 - x;
  }
  else
  {
    ledIndex = (NUM_LEDS_Y - y - 1) * NUM_LEDS_X + x;
  }

  leds[ledIndex] = color;
}