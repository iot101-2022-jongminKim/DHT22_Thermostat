#include <Arduino.h>
#include <DHTesp.h>
#include <SSD1306.h>

SSD1306 display(0x3c, 4, 5, GEOMETRY_128_32);
DHTesp dht;

const int pulseA = 12;
const int pulseB = 13;
const int pushSW = 2;
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
const int RELAY = 15;
int lux = A0;

int interval = 2000;
unsigned lastDHTReadMillis = 0;
float humidity = 0;
float temperature = 0;

void readDHT22()
{
  unsigned long currentMillis = millis();

  if (currentMillis - lastDHTReadMillis >= interval)
  {
    lastDHTReadMillis = currentMillis;
    humidity = dht.getHumidity();
    temperature = dht.getTemperature();
  }
}

IRAM_ATTR void handleRotary()
{
  // Never put any long instruction
  int MSB = digitalRead(pulseA); // MSB = most significant bit
  int LSB = digitalRead(pulseB); // LSB = least significant bit

  int encoded = (MSB << 1) | LSB;         // converting the 2 pin value to single number
  int sum = (lastEncoded << 2) | encoded; // adding it to the previous encoded value
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderValue++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderValue--;
  lastEncoded = encoded; // store this value for next time
  if (encoderValue > 60)
  {
    encoderValue = 60;
  }
  else if (encoderValue < 0)
  {
    encoderValue = 0;
  }
}

IRAM_ATTR void buttonClicked()
{
  Serial.println("pushed");
}

void setup()
{
  Serial.begin(115200);
  dht.setup(14, DHTesp::DHT22);
  Serial.println();
  Serial.println("Humidity (%) \t Temperature (C)");

  pinMode(pushSW, INPUT_PULLUP);
  pinMode(pulseA, INPUT_PULLUP);
  pinMode(pulseB, INPUT_PULLUP);
  pinMode(RELAY, OUTPUT);
  attachInterrupt(pushSW, buttonClicked, FALLING);
  attachInterrupt(pulseA, handleRotary, CHANGE);
  attachInterrupt(pulseB, handleRotary, CHANGE);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.drawString(10, 10, "Hello World");
  display.display();
}

void loop()
{
  readDHT22();
  Serial.printf("%.1f\t %.1f\n", humidity, temperature);

  if (encoderValue > temperature)
  {
    digitalWrite(RELAY, HIGH);
    Serial.println("RELAY ON");
  }
  else
  {
    digitalWrite(RELAY, LOW);
    Serial.println("RELAY OFF");
  }

  display.clear();
  display.drawString(11, 12, "temp: " + (String)temperature);
  display.drawString(11, 22, "encoderValue: " + (String)encoderValue);
  display.display();
  delay(1000);
}