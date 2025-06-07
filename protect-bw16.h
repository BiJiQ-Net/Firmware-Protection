#ifndef PROTECT_H
#define PROTECT_H

#include <WiFi.h>
#include <FlashMemory.h>
#include "mbedtls/md5.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#define HASH_ADDRESS 0x00100000  // Flash memory base address
#define HASH_OFFSET  0x0000      // Offset within the flash memory
#define HASH_LENGTH  32          // MD5 hash length as hex string

const String SALT = "SALTYFISH";

// Convert binary MD5 to hexadecimal string
String toHexString(const unsigned char *hash, size_t length = 16) {
  char hex[33];
  for (size_t i = 0; i < length; i++) {
    sprintf(&hex[i * 2], "%02x", hash[i]);
  }
  hex[32] = '\0';
  return String(hex);
}

// Perform double MD5 hash using mbedTLS
String doubleMD5(String data) {
  unsigned char firstHash[16];
  unsigned char secondHash[16];

  mbedtls_md5((const unsigned char*)data.c_str(), data.length(), firstHash);
  String firstHashHex = toHexString(firstHash);
  mbedtls_md5((const unsigned char*)firstHashHex.c_str(), firstHashHex.length(), secondHash);
  return toHexString(secondHash);
}

// Get MAC address as uppercase string without delimiters
String getMACAddress() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String macStr;
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) macStr += "0";
    macStr += String(mac[i], HEX);
  }
  macStr.toUpperCase();
  return macStr;
}

// Read a string from flash memory
String readStringFromFlash(uint32_t address, size_t length) {
  char buffer[length + 1];
  for (size_t i = 0; i < length; i += 4) {
    uint32_t word = FlashMemory.readWord(address + i);
    memcpy(buffer + i, &word, min((size_t)4, length - i));
  }
  buffer[length] = '\0';
  return String(buffer);
}

// Write a string to flash memory
void writeStringToFlash(uint32_t address, const String& str, size_t length) {
  char buffer[length + 1];
  str.toCharArray(buffer, length + 1);
  for (size_t i = 0; i < length; i += 4) {
    uint32_t word = 0;
    memcpy(&word, buffer + i, min((size_t)4, length - i));
    FlashMemory.writeWord(address + i, word);
  }
}

// Read hash from flash
String readHashFromFlash() {
  return readStringFromFlash(HASH_OFFSET, HASH_LENGTH);
}

// Write hash to flash
void writeHashToFlash(String hash) {
  Serial.print("Writing hash to flash: ");
  Serial.println(hash);
  writeStringToFlash(HASH_OFFSET, hash, HASH_LENGTH);
  Serial.println("Hash written to flash memory successfully");
}

// Prompt the user for registration
void promptForHash() {
  String macAddress = getMACAddress();
  String concatenatedMAC = SALT + macAddress;
  String expectedHash = doubleMD5(concatenatedMAC);

  while (true) {
    Serial.println("Device Not Registered");
    Serial.println("Please Enter Registration Code:");

    while (!Serial.available()) {
      //delay(10); // Prevent busy-waiting
    }
    String userInput = Serial.readString();
    userInput.trim();

    Serial.print("User Input: ");
    Serial.println(userInput);

    if (userInput.length() == HASH_LENGTH) {
      if (userInput == expectedHash) {
        writeHashToFlash(userInput);
        Serial.println("Hash registered successfully. Restarting...");
        delay(1000);
        sys_reset();  // Restart the BW16 module
      } else {
        Serial.println("Invalid registration code. Please try again.");
      }
    } else {
      Serial.println("Invalid hash length. Please try again.");
    }
  }
}

// Initialization function
void ProtectSetup() {
  Serial.begin(115200);
  FlashMemory.begin(HASH_ADDRESS, 0x4000);

  String macAddress = getMACAddress();
  String concatenatedMAC = SALT + macAddress;
  String expectedHash = doubleMD5(concatenatedMAC);
  String storedHash = readHashFromFlash();

  Serial.print("MAC Address: ");
  Serial.println(macAddress);

  if (storedHash == expectedHash) {
    Serial.println("Device is Registered");
  } else {
    promptForHash();
  }
}

#endif // PROTECT_H
