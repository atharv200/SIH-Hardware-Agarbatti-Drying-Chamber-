#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHT1_PIN 33
#define DHT2_PIN 32
#define DHT3_PIN 25
#define DHT4_PIN 26
#define DHT5_PIN 27
#define DHT6_PIN 14

#define DHTTYPE DHT11

#define HEATER_PIN 16
#define EXHAUST_PIN 17

#define SPRAY_BUTTON_PIN 18
#define PRE_MODE_BUTTON_PIN 19
#define POST_MODE_BUTTON_PIN 21
#define SPRAYER_PIN 22

DHT dht1(DHT1_PIN, DHTTYPE);
DHT dht2(DHT2_PIN, DHTTYPE);
DHT dht3(DHT3_PIN, DHTTYPE);
DHT dht4(DHT4_PIN, DHTTYPE);
DHT dht5(DHT5_PIN, DHTTYPE);
DHT dht6(DHT6_PIN, DHTTYPE);

unsigned long LastSensorRead = 0;
unsigned long LastDataSent = 0;
unsigned long SprayStartTime = 0;
int CurrHeaterState = LOW;
int CurrExhaustState = LOW;
float avgT = 0;
float avgH = 0;
bool SprayActive = false;

enum Mode {
  PRE_SPRAY,
  POST_SPRAY
};

Mode CurrentMode = PRE_SPRAY;

const char* ssid = "OnePlus Nord 2T 5G";
const char* password = "atharv@06";
const char* apikey = "52XMJOLZB5UP5524";

void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  dht1.begin();
  dht2.begin();
  dht3.begin();
  dht4.begin();
  dht5.begin();
  dht6.begin();

  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);

  pinMode(EXHAUST_PIN, OUTPUT);
  digitalWrite(EXHAUST_PIN, LOW);

  pinMode(SPRAY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PRE_MODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(POST_MODE_BUTTON_PIN, INPUT_PULLUP);

  pinMode(SPRAYER_PIN, OUTPUT);
  digitalWrite(SPRAYER_PIN, LOW);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi Connected!");
  Serial.println("Current Mode: PRE-SPRAY");
}

void loop() {

  unsigned long currentTime = millis();

  if (digitalRead(PRE_MODE_BUTTON_PIN) == LOW) {
    CurrentMode = PRE_SPRAY;
    Serial.println("PRE-SPRAY MODE");
    delay(200);
  }

  if (digitalRead(POST_MODE_BUTTON_PIN) == LOW) {
    CurrentMode = POST_SPRAY;
    Serial.println("POST-SPRAY MODE");
    delay(200);
  }

  if (digitalRead(SPRAY_BUTTON_PIN) == LOW && !SprayActive) {

    SprayActive = true;
    SprayStartTime = currentTime;

    digitalWrite(SPRAYER_PIN, HIGH);

    Serial.println("Spray Started");

    delay(200);
  }

  if (SprayActive && currentTime - SprayStartTime >= 5000) {

    SprayActive = false;

    digitalWrite(SPRAYER_PIN, LOW);

    Serial.println("Spray Finished");
  }

  if (currentTime - LastSensorRead > 2000) {

    LastSensorRead = currentTime;

    float temp1 = dht1.readTemperature();
    float hum1 = dht1.readHumidity();

    float temp2 = dht2.readTemperature();
    float hum2 = dht2.readHumidity();

    float temp3 = dht3.readTemperature();
    float hum3 = dht3.readHumidity();

    float temp4 = dht4.readTemperature();
    float hum4 = dht4.readHumidity();

    float temp5 = dht5.readTemperature();
    float hum5 = dht5.readHumidity();

    float temp6 = dht6.readTemperature();
    float hum6 = dht6.readHumidity();

    if (
      isValid(temp1, hum1) &&
      isValid(temp2, hum2) &&
      isValid(temp3, hum3) &&
      isValid(temp4, hum4) &&
      isValid(temp5, hum5) &&
      isValid(temp6, hum6)
    ) {

      avgT = (temp1 + temp2 + temp3 + temp4 + temp5 + temp6) / 6;
      avgH = (hum1 + hum2 + hum3 + hum4 + hum5 + hum6) / 6;

      printStatus(avgT, avgH);

      heaterControl(avgT);
      exhaustControl(avgH);
    }

    else {
      Serial.println("Error in sensor!");
    }
  }

  if (currentTime - LastDataSent > 10000) {

    LastDataSent = currentTime;

    HTTPClient http;

    String url = "https://api.thingspeak.com/update?api_key=";
    url += apikey;

    url += "&field1=";
    url += avgT;

    url += "&field2=";
    url += avgH;

    http.begin(url);

    int httpCode = http.GET();

    Serial.print("HTTP response: ");
    Serial.println(httpCode);

    Serial.print("ThingSpeak response: ");
    Serial.println(http.getString());

    http.end();
  }
}

bool isValid(float temp, float hum) {

  if (isnan(temp) || isnan(hum))
    return false;

  else if (temp < 10 || temp > 100 || hum < 0 || hum > 100)
    return false;

  else
    return true;
}

void printStatus(float avgT, float avgH) {

  Serial.print("Average Temperature: ");
  Serial.println(avgT);

  Serial.print("Average Humidity: ");
  Serial.println(avgH);

  Serial.print("Heater: ");
  Serial.println(CurrHeaterState);

  Serial.print("Exhaust: ");
  Serial.println(CurrExhaustState);

  if (CurrentMode == PRE_SPRAY) Serial.println("Mode: PRE-SPRAY");
  else Serial.println("Mode: POST-SPRAY");

  Serial.print("Sprayer: ");
  Serial.println(SprayActive);
}

void heaterControl(float avgT) {

  if (CurrentMode == PRE_SPRAY) {
    if (avgT < 45) {
      digitalWrite(HEATER_PIN, HIGH);
      CurrHeaterState = HIGH;
    }
    else if (avgT > 55) {
      digitalWrite(HEATER_PIN, LOW);
      CurrHeaterState = LOW;
    }
    else {
      digitalWrite(HEATER_PIN, CurrHeaterState);
    }
  }
  else if (CurrentMode == POST_SPRAY) {
    if (avgT < 30) {
      digitalWrite(HEATER_PIN, HIGH);
      CurrHeaterState = HIGH;
    }
    else if (avgT > 40) {
      digitalWrite(HEATER_PIN, LOW);
      CurrHeaterState = LOW;
    }
    else {
      digitalWrite(HEATER_PIN, CurrHeaterState);
    }
  }
}

void exhaustControl(float avgH) {
  if (avgH > 70) {
    digitalWrite(EXHAUST_PIN, HIGH);
    CurrExhaustState = HIGH;
  }
  else {
    digitalWrite(EXHAUST_PIN, LOW);
    CurrExhaustState = LOW;
  }
}