#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define ENTRY_IR_SENSOR_PIN 5
#define EXIT_IR_SENSOR_PIN 6
#define SLOT1_IR_SENSOR_PIN 2
#define SLOT2_IR_SENSOR_PIN 3
#define SLOT3_IR_SENSOR_PIN 4
#define SERVO_PIN 7
#define RST_PIN 9
#define SS_PIN 10

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
const char* iftttKey = "your_IFTTT_KEY";

Servo gateServo;
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {3, 4, 5, 6};
byte colPins[COLS] = {7, 8, 9, 10};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

long startTime;
float parkingRate = 2.0; // 2 rupees per minute
float balance = 50.0; // Initial balance
int slotAvailable = 0;
boolean parkingActive = false;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();
  gateServo.attach(SERVO_PIN);
  gateServo.write(0); // Initial gate position closed
  pinMode(ENTRY_IR_SENSOR_PIN, INPUT);
  pinMode(EXIT_IR_SENSOR_PIN, INPUT);
  pinMode(SLOT1_IR_SENSOR_PIN, INPUT);
  pinMode(SLOT2_IR_SENSOR_PIN, INPUT);
  pinMode(SLOT3_IR_SENSOR_PIN, INPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(2000);
}

void loop() {
  checkSlots();
  if (digitalRead(ENTRY_IR_SENSOR_PIN) == HIGH && !parkingActive) {
    if (rfidCheck()) {
      slotAvailable = findEmptySlot();
      if (slotAvailable != 0) {
        startParking();
      } else {
        lcd.clear();
        lcd.print("No Slots Free");
      }
    }
  }
  
  if (digitalRead(EXIT_IR_SENSOR_PIN) == HIGH && parkingActive) {
    stopParking();
  }
  
  char key = keypad.getKey();
  if (key) {
    rechargeBalance(key);
  }
  delay(1000);
}

void checkSlots() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (digitalRead(SLOT1_IR_SENSOR_PIN) == HIGH) lcd.print("Slot 1: Occupied");
  else lcd.print("Slot 1: Free");
  
  lcd.setCursor(0, 1);
  if (digitalRead(SLOT2_IR_SENSOR_PIN) == HIGH) lcd.print("Slot 2: Occupied");
  else lcd.print("Slot 2: Free");
  
  lcd.setCursor(0, 2);
  if (digitalRead(SLOT3_IR_SENSOR_PIN) == HIGH) lcd.print("Slot 3: Occupied");
  else lcd.print("Slot 3: Free");
}

boolean rfidCheck() {
  if (!rfid.PICC_IsNewCardPresent()) return false;
  if (!rfid.PICC_ReadCardSerial()) return false;
  
  // Simple UID check (replace with real logic to validate RFID card)
  Serial.print("RFID UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();
  
  lcd.clear();
  lcd.print("Card Identified");
  delay(2000);
  return true;
}

int findEmptySlot() {
  if (digitalRead(SLOT1_IR_SENSOR_PIN) == LOW) return 1;
  if (digitalRead(SLOT2_IR_SENSOR_PIN) == LOW) return 2;
  if (digitalRead(SLOT3_IR_SENSOR_PIN) == LOW) return 3;
  return 0;
}

void startParking() {
  startTime = millis();
  parkingActive = true;
  openGate();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Park in Slot ");
  lcd.print(slotAvailable);
  delay(3000);
}

void stopParking() {
  long parkingTime = (millis() - startTime) / 60000; // Convert milliseconds to minutes
  float totalCost = parkingTime * parkingRate;
  
  if (balance >= totalCost) {
    balance -= totalCost;
    lcd.clear();
    lcd.print("Time: ");
    lcd.print(parkingTime);
    lcd.setCursor(0, 1);
    lcd.print("Cost: ");
    lcd.print(totalCost);
    delay(3000);
    
    lcd.clear();
    lcd.print("Bal: ");
    lcd.print(balance);
    
    if (balance < 10) {
      sendLowBalanceNotification();
    }
    
    closeGate();
    parkingActive = false;
  } else {
    lcd.clear();
    lcd.print("Low Balance!");
    lcd.setCursor(0, 1);
    lcd.print("Recharge Needed");
  }
  delay(3000);
}

void openGate() {
  gateServo.write(90); // Open gate
  lcd.clear();
  lcd.print("Gate Open");
  delay(5000);
  gateServo.write(0); // Close gate
}

void closeGate() {
  gateServo.write(0); // Close gate
  lcd.clear();
  lcd.print("Gate Closed");
  delay(5000);
}

void rechargeBalance(char key) {
  if (key >= '0' && key <= '9') {
    int rechargeAmount = (key - '0') * 10;
    balance += rechargeAmount;
    lcd.clear();
    lcd.print("Recharged: ");
    lcd.print(rechargeAmount);
    lcd.setCursor(0, 1);
    lcd.print("New Bal: ");
    lcd.print(balance);
    delay(2000);
  }
}

void sendLowBalanceNotification() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String("http://maker.ifttt.com/trigger/low_balance/with/key/") + iftttKey;
    http.begin(url);
    int httpCode = http.GET();
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}
