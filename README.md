# RFID Car Parking System

## Overview
The RFID-based car parking system in IoE aims to streamline vehicle identification and parking management. Vehicles will have RFID tags, which are scanned by RFID readers installed at the entry and exit points of the parking facility. Parking space sensors detect vehicle presence in each space. Parking fee will be charged according to the time the vehicle was parked. A microcontroller collects data from the RFID readers and sensors, processes it, and sends it to a cloud service for real-time and historical analysis. Users receive real-time notifications via SMS about parking availability and their parking session details.

## Project Features
1. Automated Car Parking System:
	- Hands-free vehicle identification with RFID tags.  
	- Entry and exit barrier control using servo motors.  

2. Intelligent Connectivity & Notifications:
	- Real-time SMS alerts for parking availability, session status, and balance updates.  
	
3. Data Analysis & Monitoring:
   	- Cloud-based storage for real-time and historical usage data.  
   	- Data insights to manage parking efficiency and optimize operations.  

4. Security & Access Control:
   	- Ensures only authorized vehicles access parking and toll areas.  
   	- Tamper-proof RFID tags and secure data transfer.  

5. Rechargeable Wallet System:
   	- 4x4 Keypad for easy on-site balance top-up.  
   	- Parking fee deduction directly from RFID card balance.

## System Architecture & Workflow
1. Entry Process:
	- RFID reader scans the vehicle’s tag at the entry point.
	- If sufficient balance exists, the servo motor opens the barrier.
	- The vehicle’s entry time is recorded and uploaded to the cloud.

2. Parking Management:
	- IR sensors detect available spaces and update the cloud platform.
	- Users receive notifications about available parking spaces via SMS.

3. Exit Process:
	- RFID reader scans the tag at the exit.
	- Parking fees are calculated based on the entry and exit time.
	- If the user has sufficient balance, the barrier opens, and fees are deducted.

4. Recharge Process:
	- Users can recharge their RFID cards using the 4x4 keypad interface.
	- The new balance is updated in real-time and stored on the cloud.

## Components Required
- Arduino Uno
- RFID Card Reader
- RFID Card Tags
- 2x16 IC2 LCD Display
- 4x4 Keypad for recharging balance
- IR Sensors
- Servo Motor
- ESP8266 WiFi Module

### Contributors
- [Vishnu Yelde](https://github.com/VishnuYelde)
- [Prachiti Yadav](https://github.com/PrachitiYadav)
- [Rahul Kulkarni](https://github.com/RahulKulkarni681)

### License
This project is licensed under the [GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/). See the LICENSE file for more details.
