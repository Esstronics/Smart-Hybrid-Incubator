#include <DHT.h>
#include <FirebaseESP32.h>
#include <WiFi.h>
#include <LiquidCrystal.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// WiFi credentials
#define WIFI_SSID "Incubator"
#define WIFI_PASSWORD "Incubator"

// Firebase credentials
#define FIREBASE_HOST "https://myfirebaseproject-63b3e-default-rtdb.firebaseio.com"  // e.g., "your-project-id.firebaseio.com"
#define FIREBASE_AUTH "wb3Rq3TRqzPtMbvBqPhlmfG5FyHiZ4GBAj9ppOx0"
// FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// DHT11 pins
// #define DHTPIN1 25
// #define DHTPIN2 33
#define DHTPIN 33
#define DHTTYPE DHT11

// Relay pins
// #define RELAY1 13
// #define RELAY2 12
#define RELAY 14
#define HUMIDIFIER_RELAY 23
#define STEPPER_RELAY 13

// LCD pins
#define RS 22
#define EN 21
#define D4 19
#define D5 18
#define D6 5
#define D7 4
#define CT 32
#define cndl 12

// Initialize DHT sensors
DHT dht(DHTPIN, DHTTYPE);

// Initialize LCD
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

// Firebase object
FirebaseData firebaseData;

// Variables
volatile float maxTemp = 38, minTemp = 36, maxHumidity = 60, minHumidity = 50;
// volatile float temp;
// volatile int humidity;
volatile int stepperActivationsPerDay = 3;
volatile int stepperActivationCount = 1;
int d;
unsigned long lastStepperActivationTime = 0;
unsigned long stepperActivationInterval = 28800; // Interval between activations
int Days = 0;
unsigned long dayCounter = 0;
// Task handles
TaskHandle_t firebaseTaskHandle;
TaskHandle_t sensorRelayDisplayTaskHandle;

void setup() {
  Serial.begin(115200);

  // Initialize DHT sensors
  dht.begin();

  // Initialize relays
  pinMode(RELAY, OUTPUT);
  pinMode(HUMIDIFIER_RELAY, OUTPUT);
  pinMode(STEPPER_RELAY, OUTPUT);
  pinMode(CT, OUTPUT);
  pinMode(cndl,OUTPUT);
  analogWrite(CT, 100);
  digitalWrite(RELAY, LOW); 
  digitalWrite(HUMIDIFIER_RELAY, LOW);
  digitalWrite(STEPPER_RELAY, LOW);
  digitalWrite(cndl,LOW);
  
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.setCursor(3, 0);
  lcd.print("Incubator");
  lcd.setCursor(5, 1);
  lcd.print("System");
  delay(2000);
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Connecting");
  lcd.setCursor(4, 1);
  lcd.print("to WiFi");
  delay(100);
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED && millis() <= 10000) {
    delay(500);
    Serial.print(".");
  }
  if(WiFi.status() == WL_CONNECTED){
  Serial.println("Connected to Wi-Fi");
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("Connected");
  delay(2000);
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Fetching Data");
  lcd.setCursor(4,1);
  lcd.print("From Cloud");
  delay(2000);
  lcd.clear();
  lcd.setCursor(4,1);
  lcd.print("Done!");
  delay(1000);
  // Initialize Firebase
  config.api_key =  FIREBASE_AUTH;
  config.database_url = FIREBASE_HOST;
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setDoubleDigits(7);

  if (Firebase.RTDB.getString(&firebaseData, "/Incubator1/dayCounter")) {
    String f = firebaseData.stringData();
    f.replace(String("\""), "");
    f.replace(String("\\"), "");
    f.replace(String("\""), "");
    f.trim();
    dayCounter = f.toInt();
  }
  // Firebase.reconnectWiFi(true);
  }
  else{ 
    lcd.clear(); 
    lcd.setCursor(2,0);
    lcd.print("No Available");
    lcd.setCursor(4,1);
    lcd.print("WiFi");
    delay(2000);
  } 
   

  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Turning..");
  digitalWrite(STEPPER_RELAY, HIGH); // Turn on stepper relay
  delay(5000); // Keep it on for 5 seconds (adjust as needed)
  digitalWrite(STEPPER_RELAY, LOW); // Turn off stepper relay
    

  // Create RTOS tasks
  xTaskCreatePinnedToCore(firebaseTask, "FirebaseTask", 10000, NULL, 1, &firebaseTaskHandle, 1);
  xTaskCreatePinnedToCore(sensorRelayDisplayTask, "SensorRelayDisplayTask", 10000, NULL, 1, &sensorRelayDisplayTaskHandle, 0);
  
}

void loop() {
  // The loop function is not used in RTOS-based implementations
  vTaskDelete(NULL); // Delete the default task created by Arduino
}

void firebaseTask(void *parameter) {
  for (;;) {
    int a=0;
    // dayCounter = dayCounter + (millis()/1000);
    if(WiFi.status() == WL_CONNECTED){
    if (dayCounter + (millis()/1000) == 86400){
    Days += 1; Firebase.RTDB.setInt(&firebaseData,"/Incubator1/Days", Days);}
    fetchFirebaseSettings();
    // Serial.println("fetched");
    sendDataToFirebase();
    if(a==0){
    config.api_key =  FIREBASE_AUTH;
    config.database_url = FIREBASE_HOST;
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.setDoubleDigits(7); a=1;
    }
    vTaskDelay(pdMS_TO_TICKS(300)); // Delay for 5 seconds
    }
    else{a=0; //Serial.println("No Connection");
    }
  }
}

void sensorRelayDisplayTask(void *parameter) {
  for (;;) {
    // Read sensor data
    float temp = dht.readTemperature();
    int humidity = dht.readHumidity();
    if (isnan(temp) || isnan(humidity)){ temp = 0; humidity=0;}
    Serial.println(temp);
    Serial.println(humidity);
    // Control heating relays
    controlRelay(RELAY, temp, minTemp, maxTemp);

    //control humidifier 
    controlHumidity(HUMIDIFIER_RELAY, humidity, minHumidity, maxHumidity);
    // Control stepper motor relay
    controlStepperMotor();

    // Display data on LCD
    displayOnLCD(temp,humidity,stepperActivationCount);
    Serial.println(maxTemp);
    Serial.println(minTemp);
    Serial.println(minHumidity);
    Serial.println(maxHumidity);
    vTaskDelay(pdMS_TO_TICKS(200)); // Delay for 5 seconds
  }
}

void fetchFirebaseSettings() {
  // Fetch maxTemp
  if (Firebase.RTDB.getString(&firebaseData, "/Incubator1/maxTemp")) {
    String f = firebaseData.stringData();
    f.replace(String("\""), "");
    f.replace(String("\\"), "");
    f.replace(String("\""), "");
    f.trim();
    maxTemp = f.toFloat();
    
  }

  // Fetch minTemp
  if (Firebase.RTDB.getString(&firebaseData, "/Incubator1/minTemp")) {
    String f = firebaseData.stringData();
    f.replace(String("\""), "");
    f.replace(String("\\"), "");
    f.replace(String("\""), "");
    f.trim();
    minTemp = f.toFloat();
    
  }

  // Fetch maxHumidity
  if (Firebase.RTDB.getString(&firebaseData, "/Incubator1/maxHumidity")) {
    String f = firebaseData.stringData();
    f.replace(String("\""), "");
    f.replace(String("\\"), "");
    f.replace(String("\""), "");
    f.trim();
    maxHumidity = f.toFloat();
  }

  // Fetch minHumidity
  if (Firebase.RTDB.getString(&firebaseData, "/Incubator1/minHumidity")) {
    String f = firebaseData.stringData();
    f.replace(String("\""), "");
    f.replace(String("\\"), "");
    f.replace(String("\""), "");
    f.trim();
    minHumidity = f.toFloat();
  }

  if (Firebase.RTDB.getString(&firebaseData, "/Incubator1/Candle")) {
    String f = firebaseData.stringData();
    f.replace(String("\""), "");
    f.replace(String("\\"), "");
    f.replace(String("\""), "");
    f.trim();
    int Candle = f.toInt();
    digitalWrite(cndl,Candle);
  }

  // Fetch stepperActivationsPerDay
  if (Firebase.RTDB.getString(&firebaseData, "/Incubator1/stepperActivationsPerDay")) {
    String f = firebaseData.stringData();
    f.replace(String("\""), "");
    f.replace(String("\\"), "");
    f.replace(String("\""), "");
    f.trim();
    stepperActivationsPerDay = f.toInt();
    // Calculate the interval between activations
    if (stepperActivationsPerDay > 0) {
      stepperActivationInterval = 86400 / stepperActivationsPerDay; // 86400 seconds in a day
    }
  }
}

void sendDataToFirebase() {
  float temp = dht.readTemperature();
  int humidity = dht.readHumidity();

  Firebase.RTDB.setFloat(&firebaseData, "/Incubator1/temperature", temp);
  Firebase.RTDB.setInt(&firebaseData, "/Incubator1/humidity", humidity);
  Firebase.RTDB.setInt(&firebaseData, "/Incubator1/stepperActivationCount", stepperActivationCount);
  Firebase.RTDB.setInt(&firebaseData,"/Incubator1/Candling", digitalRead(cndl));
  Firebase.RTDB.setInt(&firebaseData,"/Incubator1/dayCounter", dayCounter + (millis()/1000));
}

void controlRelay(int relayPin, float sensorValue, float minVal, float maxVal) {
  if (sensorValue > 5 && sensorValue < minVal) {
    digitalWrite(relayPin, HIGH); // Turn on relay
  } else if (sensorValue > maxVal) {
    digitalWrite(relayPin, LOW); // Turn off relay
  }
}

void controlHumidity(int humidPin, int sensorValue, int minVal, int maxVal) {
  if (sensorValue >0 && sensorValue < minVal && d == 0) {
    digitalWrite(HUMIDIFIER_RELAY,HIGH);
    delay(300);
    digitalWrite(HUMIDIFIER_RELAY,LOW);
    d=1;
     // Turn on relay
  } else if (sensorValue > maxVal && d == 1) {
    digitalWrite(HUMIDIFIER_RELAY,HIGH); 
    delay(500);
    digitalWrite(HUMIDIFIER_RELAY,LOW);
    delay(1000);
    digitalWrite(HUMIDIFIER_RELAY,HIGH);
    delay(300);
    digitalWrite(HUMIDIFIER_RELAY,LOW); // Turn off relay
   d = 0;
  }
  // else {
  //   digitalWrite(humidPin,LOW);
  //   delay(200);
  //   digitalWrite(humidPin,HIGH);}
}


void controlStepperMotor() {
  unsigned long currentTime = millis() / 1000; // Convert to seconds
  if (stepperActivationCount < stepperActivationsPerDay) {
    if (currentTime - lastStepperActivationTime >= stepperActivationInterval) {
      digitalWrite(STEPPER_RELAY, HIGH); // Turn on stepper relay
      delay(5000); // Keep it on for 5 seconds (adjust as needed)
      digitalWrite(STEPPER_RELAY, LOW); // Turn off stepper relay
      stepperActivationCount++;
      lastStepperActivationTime = currentTime;
    }
  }
}

void displayOnLCD(float temp,int humidity,int count) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tmp:"); lcd.print(temp,1); lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Hum:"); lcd.print(humidity); lcd.print("%");
  lcd.setCursor(11,0); lcd.print("Turns");
  lcd.setCursor(13,1); lcd.print(count);
}