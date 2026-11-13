#define BLYNK_TEMPLATE_ID "****"
#define BLYNK_TEMPLATE_NAME "****"
#define BLYNK_AUTH_TOKEN "****"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>  
#include <BlynkSimpleEsp8266.h>

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "****";
char pass[] = "****";

#define SENSOR1 0 // First flow sensor D3
#define SENSOR2 2  // Second flow sensor D4

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
float calibrationFactor = 7.8;

// Sensor 1 variables
volatile byte pulseCount1;
byte pulse1Sec1 = 0;
float flowRate1;
unsigned int flowMilliLitres1;
float totalMilliLitres1;

// Sensor 2 variables
volatile byte pulseCount2;
byte pulse1Sec2 = 0;
float flowRate2;
unsigned int flowMilliLitres2;
float totalMilliLitres2;

int Vol1, Vol2; 

// Blynk virtual pin assignments
#define VPIN_FLOWRATE1   V0  // Sensor 1 flow rate
#define VPIN_TOTAL1      V1  // Sensor 1 total liters
#define VPIN_FLOWRATE2   V3  // Sensor 2 flow rate
#define VPIN_TOTAL2      V4  // Sensor 2 total liters
#define VPIN_RESET1      V2  // Button for sensor 1 reset
#define VPIN_RESET2      V5  // Button for sensor 2 reset

void ICACHE_RAM_ATTR pulseCounter1() 
{
  pulseCount1++;
}

void ICACHE_RAM_ATTR pulseCounter2() 
{
  pulseCount2++;
}

void setup() {
  Serial.begin(115200);

  pinMode(SENSOR1, INPUT_PULLUP);
  pinMode(SENSOR2, INPUT_PULLUP);

  // Sensor 1 init
  pulseCount1 = 0;
  flowRate1 = 0.0;
  flowMilliLitres1 = 0;
  totalMilliLitres1 = 0;
  // Sensor 2 init
  pulseCount2 = 0;
  flowRate2 = 0.0;
  flowMilliLitres2 = 0;
  totalMilliLitres2 = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR1), pulseCounter1, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR2), pulseCounter2, FALLING);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() 
{
  Blynk.run();

  currentMillis = millis();

  if (currentMillis - previousMillis > interval) {
    // Sensor 1
    pulse1Sec1 = pulseCount1;
    pulseCount1 = 0;

    flowRate1 = ((1000.0 / (millis() - previousMillis)) * pulse1Sec1) / calibrationFactor;
    flowMilliLitres1 = (flowRate1 / 60) * 1000;
    totalMilliLitres1 += flowMilliLitres1;

    // Sensor 2
    pulse1Sec2 = pulseCount2;
    pulseCount2 = 0;

    flowRate2 = ((1000.0 / (millis() - previousMillis)) * pulse1Sec2) / calibrationFactor;
    flowMilliLitres2 = (flowRate2 / 60) * 1000;
    totalMilliLitres2 += flowMilliLitres2;

    previousMillis = millis();

    // Print both to Serial
    Serial.print("Sensor 1 Flow rate: ");
    Serial.print(int(flowRate1));
    Serial.print(" L/min\t");
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres1);
    Serial.print(" mL / ");
    Serial.print(totalMilliLitres1 / 1000);
    Serial.println(" L");

    Serial.print("Sensor 2 Flow rate: ");
    Serial.print(int(flowRate2));
    Serial.print(" L/min\t");
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres2);
    Serial.print(" mL / ");
    Serial.print(totalMilliLitres2 / 1000);
    Serial.println(" L");

    // Send both sensors' data to Blynk
    Blynk.virtualWrite(VPIN_FLOWRATE1, flowRate1);
    Blynk.virtualWrite(VPIN_TOTAL1, totalMilliLitres1 / 1000.0);

    Blynk.virtualWrite(VPIN_FLOWRATE2, flowRate2);
    Blynk.virtualWrite(VPIN_TOTAL2, totalMilliLitres2 / 1000.0);

    Vol1 = totalMilliLitres1 / 1000.0;
    Vol2 = totalMilliLitres2 / 1000.0;

    if (Vol1 > Vol2)
{
   Blynk.logEvent("leakage");
}
  }
}

// Sensor 1 reset button
BLYNK_WRITE(VPIN_RESET1) 
{
  int buttonState = param.asInt();
  if (buttonState == 1) {
    totalMilliLitres1 = 0;
    Blynk.virtualWrite(VPIN_TOTAL1, 0.0);
  }
}

// Sensor 2 reset button
BLYNK_WRITE(VPIN_RESET2) 
{
    int buttonState = param.asInt();
    if (buttonState == 1) {
    totalMilliLitres2 = 0;
    Blynk.virtualWrite(VPIN_TOTAL2, 0.0);
  }
}
