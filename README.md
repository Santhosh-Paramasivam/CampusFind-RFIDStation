# CampusFind RFID Check-In System  

## CampusFind

[CampusFind](https://github.com/Santhosh-Paramasivam/CampusFind.git) is a system of many applications that allow users to quickly and seamlessly find colleagues and coworkers within an institution  

📌 **Full System Documentation** : The **architecture documentation** and links to all the different components of the system can be found in the [CampusFind repository](https://github.com/Santhosh-Paramasivam/CampusFind.git)  

## Purpose

CampusFind's check-in system allows users to notify that they've checked into a room by **tapping an RFID tag**, possibly integrated into an ID card, against an **RFID reader IoT setup**

They can notify the system by tapping the tag again upon exiting a room, allowing for seamless usage of the CampusFind system  

## Video Demo

[![CampusFind IoT RFID Demo](https://github.com/user-attachments/assets/370ea4c2-bdf2-41e5-9f1f-e114b620a7ba)](https://www.youtube.com/watch?v=-RQ_UieBsZU)

## Components

- **IoT-system** that collects the users RFID UID data and transmits it to a **Flask backend** along with institution credentials
- **Flask REST API Backend** that receives the data and contains the business logic to update the user's location in the **CampusFind Firestore Database**

*Note : This repo only contains code for this IoT system*
The repo for the flask backend can be found [here](https://github.com/Santhosh-Paramasivam/campusfind-checkin-backend.git)

## Architecture  

The IoT-system consists of,

- **mrfc522 RFID tag reader** : To detect the RFID tag, acquire its UID and transmit that data to the esp8266

- **esp8266 Microcontroller** : To acquire, process and transmit the RFID UID of the tag along with institution_id, institution_api_key and other credentials  

### Pinout  

![Image](https://github.com/user-attachments/assets/774d413e-b719-4a0b-897d-c16bc3b01946)  

|esp8266|mfrc522|
|-------|-------|
|D0 (GPIO16) |SDA|
|D3 (GPIO0) |RST|
|D5 (GPIO14) |SCK|
|D6 (GPIO12) |MISO|
|D7 (GPIO13) |MOSI|
|GND |GND|
|3.3v |3.3v|  
