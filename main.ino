#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// Define Pins for RFID
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);  // Create instance of RFID

// Servo motor for barrier
Servo barrierServo;
#define SERVO_PIN A3  // Servo connected to pin A3

// IR Sensors
#define IR_ENTRY A0     // Entry IR sensor before barrier
#define IR_EXIT A1      // Exit IR sensor after barrier
#define IR_SLOT_1 A2    // IR sensor for Parking Slot 1

// LCD Display
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set LCD address to 0x27 for a 16 chars and 2 line display

// Keypad
const byte ROWS = 4;  // Four rows
const byte COLS = 4;  // Four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {2, 3, 4, 5};  // Row pins
byte colPins[COLS] = {6, 7, 8, 9};  // Column pins
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Parking fee settings
#define PARKING_FEE_PER_MINUTE 10  // Rs 10 per minute
unsigned long entryTime;  // Time when car entered
unsigned long exitTime;   // Time when car exited

// User's RFID card data (3 users)
struct User {
  String id;
  String name;
  float balance;
};

// Array of 3 users
User users[3] = {
  {"14DB8A7", "Aditya", 50.0},  // User 1: Aditya
  {"3B5A7111", "Vishnu", 300.0},    // User 2: Vishnu
  {"EEEEEEEE", "Prachiti", 100.0} // User 3: Prachiti
  {"ABCDEFGH", "Rahul", 100.0} 
};

User *currentUser = nullptr;  // Pointer to store current user after RFID scan

// System State
bool carParked = false;  // Car parked status
// bool cardScanned = false;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  SPI.begin();  // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522

  // Initialize servo motor
  barrierServo.attach(SERVO_PIN);
  barrierServo.write(0);  // Close the barrier initially

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RFID based Car");
  lcd.setCursor(0, 1);
  lcd.print("Parking System");
  delay(3000);
  lcd.clear();

  // Initialize IR sensors
  pinMode(IR_ENTRY, INPUT);
  pinMode(IR_EXIT, INPUT);
  pinMode(IR_SLOT_1, INPUT);
  
  // Welcome message on LCD
  lcd.setCursor(0, 0);
  lcd.print("Waiting for car");
}

void loop() {
  // Check if car is arriving at the entry
  if (digitalRead(IR_ENTRY) == LOW && !carParked) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Car Detected");
    lcd.setCursor(0, 1);
    lcd.print("Scan RFID Card");

    // Wait for RFID card scan
    if (checkRFIDCard()) {
      openBarrier();
      entryTime = millis();  // Record the entry time
      parkCar();
    }
  }

  // Check if car is leaving from the exit
  if (digitalRead(IR_EXIT) == LOW && carParked) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Car Exiting");
    lcd.setCursor(0, 1);
    lcd.print("Scan RFID Card");

    // Wait for RFID card scan
    if (checkRFIDCard()) {
      exitTime = millis();  // Record the exit time
      float fee = calculateParkingFee();  // Calculate parking fee

      if (currentUser->balance >= fee) {
        deductFee(fee);  // Deduct fee from card balance
        thankUser();  // Display thank you message
        openBarrier();
        leaveParking();
      } else {
        promptRecharge(fee);  // Prompt user to recharge if balance is low
      }
    }
  }
}

// Function to check RFID card
bool checkRFIDCard() {
  Serial.println(cardUID);
  Serial.println(currentUser);
  Serial.println(carParked);
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    // cardScanned = true;
    String cardUID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      // cardUID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
      cardUID.concat(String(rfid.uid.uidByte[i], HEX));
    }
    cardUID.toUpperCase();
    Serial.println(cardUID);  // Print scanned card UID

    // Check if the scanned card belongs to any user
    for (int i = 0; i < 3; i++) {
      if (cardUID == users[i].id) {
        currentUser = &users[i];  // Set the current user
        if (!carParked) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Welcome, " + currentUser->name);
          delay(1000);
        }
        return true;
      }
    }

    // If card not found in the list
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Card Not Found");
    delay(2000);
    return false;
  }
  return false;
}

// Function to open barrier
void openBarrier() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Opening Barrier");
  barrierServo.write(90);  // Rotate the servo to open the barrier
  delay(3000);  // Wait for the car to pass
  barrierServo.write(0);  // Close the barrier
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Barrier Closed");
  delay(1000);
}

// Function to park the car
void parkCar() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Slot 1 Empty");

  // Wait until the car parks at slot 1
  while (digitalRead(IR_SLOT_1) == HIGH) {
    delay(100);
  }

  carParked = true;  // Set car parked status
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Car Parked");
  lcd.setCursor(0, 1);
  lcd.print("Slot 1 Occupied");
}

// Function to calculate parking fee based on time parked
float calculateParkingFee() {
  unsigned long parkingDuration = (exitTime - entryTime) / 60000;  // Calculate parking time in minutes
  if (parkingDuration == 0) {
    parkingDuration = 1;  // Minimum charge for 1 minute
  }
  float fee = parkingDuration * PARKING_FEE_PER_MINUTE;  // Rs 10 per minute
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Parking Fee: Rs");
  lcd.setCursor(0, 1);
  lcd.print(fee);
  delay(3000);
  return fee;
}

// Function to deduct parking fee from user's balance
void deductFee(float fee) {
  currentUser->balance -= fee;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fee Deducted");
  lcd.setCursor(0, 1);
  lcd.print("Rem Bal: Rs " + String(currentUser->balance));
  delay(3000);
}

// Function to display thank you message when leaving
void thankUser() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Thanks, " + currentUser->name);
  lcd.setCursor(0, 1);
  lcd.print("Visit Again!");
  delay(3000);
}

// Function for car exiting the parking
void leaveParking() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Car Leaving");
  
  // Wait for car to leave
  while (digitalRead(IR_EXIT) == LOW) {
    delay(100);
  }

  carParked = false;  // Set car parked status to false
  currentUser = nullptr;    // Clear the current user
  // cardScanned = false;      // Reset the card scanned flag

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Slot 1 Empty");
}

// Function to prompt recharge if balance is low
void promptRecharge(float fee) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Low Balance!");
  lcd.setCursor(0, 1);
  lcd.print("Please Recharge!");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Amt:");
  
  float enteredAmount = 0.0;
  while (true) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') {  // Confirm the amount entered
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Recharged Rs:");
        lcd.setCursor(0, 1);
        lcd.print(enteredAmount);
        currentUser->balance += enteredAmount;  // Update balance
        delay(3000);
        break;
      } else if (key >= '0' && key <= '9') {
        enteredAmount = (enteredAmount * 10) + (key - '0');  // Append digit to amount
        lcd.setCursor(10, 0);
        lcd.print(enteredAmount);
      }
    }
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New Bal: ");
  lcd.print(currentUser->balance);
  delay(3000);

  

  // Check if balance is enough to deduct the fee after recharge
  if (currentUser->balance >= fee) {
    deductFee(fee);  // Deduct fee from card balance
    thankUser();     // Display thank you message
    openBarrier();   // Open the barrier
    leaveParking();  // Car leaves the parking
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Insufficient Bal");
    lcd.setCursor(0, 1);
    lcd.print("Recharge Again!");
    delay(2000);
  }
}

