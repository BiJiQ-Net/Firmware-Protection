#ifndef PROTECT_H
#define PROTECT_H

#include <WiFi.h>
#include <EEPROM.h>
#include <MD5Builder.h>

// Define the EEPROM size
#define EEPROM_SIZE 512
#define HASH_ADDRESS 0  // EEPROM address where hash is stored


// Define the length of the MD5 hash (32 characters)
#define HASH_LENGTH 32

// SALT
const String SALT = "SALTYFISH";


// Function to get the MAC address as a string without delimiters
String getMACAddress() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String macStr;
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) macStr += "0";  // Pad with zero
    macStr += String(mac[i], HEX);
  }
  macStr.toUpperCase();
  return macStr;
}

// Function to perform double MD5 hashing
String doubleMD5(String data) {
  // First MD5 hash
  MD5Builder md5First;
  md5First.begin();
  md5First.add(data);
  md5First.calculate();
  String firstHash = md5First.toString();

  // Second MD5 hash
  MD5Builder md5Second;
  md5Second.begin();
  md5Second.add(firstHash);
  md5Second.calculate();
  return md5Second.toString();
}

// Function to read the hash from EEPROM
String readHashFromEEPROM() {
  char hashString[HASH_LENGTH + 1];
  EEPROM.get(HASH_ADDRESS, hashString);
  hashString[HASH_LENGTH] = '\0';  // Ensure null-termination
  return String(hashString);
}

// Function to write the hash to EEPROM
void writeHashToEEPROM(String hash) {
  char hashBuffer[HASH_LENGTH + 1];
  hash.toCharArray(hashBuffer, HASH_LENGTH + 1);
  EEPROM.put(HASH_ADDRESS, hashBuffer);
  EEPROM.commit();  // Ensure changes are written to EEPROM
}


// Function to prompt for hash input
void promptForHash() {
  String macAddress = getMACAddress();
  String concatenatedMAC = SALT + macAddress;
  String expectedHash = doubleMD5(concatenatedMAC); // expectedHash = md5(md5(SALT + macAddress))

  while (true) { // Loop until a valid hash is entered
    Serial.println("Device Not Registered");
    Serial.println("Please Enter Registration Code:");
    
    while (!Serial.available()) {
      // Wait for user input
    }
    String userInput = Serial.readString();
    userInput.trim();

    // Validate hash length
    if (userInput.length() == HASH_LENGTH) {
      // Compare the input hash with the expected hash
      if (userInput == expectedHash) {
        // Write the hash to EEPROM
        writeHashToEEPROM(userInput);
        Serial.println("Hash registered successfully. Restarting...");
        delay(1000);
        ESP.restart();
      } else {
        Serial.println("Invalid registration code. Please try again.");
      }
    } else {
      Serial.println("Invalid hash length. Please try again.");
    }
  }
}


// Function to initialize and check hash
void ProtectSetup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  String macAddress = getMACAddress();
  String concatenatedMAC = SALT + macAddress; // Concatenate MAC address three times
  String expectedHash = doubleMD5(concatenatedMAC);
  String storedHash = readHashFromEEPROM();

  // Print the MAC address
  Serial.print("MAC Address: ");
  Serial.println(macAddress);

  // Check if the stored hash matches the expected hash
  if (storedHash == expectedHash) {
    Serial.println("Device is Registered");
    // Initialize your application here
  } else {
    // Serial.println("Device is not Registered");
    promptForHash();
  }
}

#endif // PROTECT_H
