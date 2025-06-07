# Firmware-Protection with MAC-Based Registration

This project provides a simple registration mechanism for ESP32/BW16 devices using the device's **MAC address** and **MD5 hashing**. It prevents unauthorized use by verifying a unique hash on boot. If the hash does not match, the user is prompted to enter a registration code via Serial Monitor.

This code will protect your firmware from device cloning.

---
## 🧰 How to Use the Code

To use the protection system in your own firmware:

### 1. Include the Header

```cpp
#include "protect.h"
```

### 2. Call `ProtectSetup()` in `setup()`

```cpp
void setup() {
  ProtectSetup();       // Will prompt for registration if device is unregistered
  Serial.begin(115200); // Optional: for serial debug output
}

void loop() {
  // Your application code here
}
```
---

## 🔐 How It Works

1. **MAC Address Retrieval:**
   - The device reads its own MAC address.
   - Example: `A1B2C3D4E5F6`

2. **Hash Generation:**
   - The MAC address is salted with a secret string: `SALTYFISH`.
   - It is then hashed **twice using MD5**:
     ```
     doubleMD5("SALTYFISH" + macAddress)
     ```

3. **Registration Flow:**
   - On first boot or unregistered device:
     - User sees a message on Serial Monitor asking for a registration code.
     - If the entered hash matches the expected one, it is saved to EEPROM.
     - On next boot, the device checks EEPROM and allows usage if the hash is valid.

---

## 🧪 How to Register a Device

1. **Generate Registration Code (off-device):**

Use this Python code:

```python
import hashlib

def double_md5(data):
    first = hashlib.md5(data.encode()).hexdigest()
    second = hashlib.md5(first.encode()).hexdigest()
    return second.upper()

salt = "SALTYFISH"
mac = "A1B2C3D4E5F6"  # Replace with the actual MAC address
reg_code = double_md5(salt + mac)
print("Registration Code:", reg_code)
```

2. **Upload the firmware to the ESP device.**
3. **Open Serial Monitor.**
4. When prompted, enter the registration code from step 1.
5. If valid, the device stores it and restarts. On the next boot, it runs normally.

---

## 📁 File Breakdown

### `protect.h`

| Function               | Description                                                  |
|------------------------|--------------------------------------------------------------|
| `getMACAddress()`      | Returns MAC address as uppercase hex string without `:`.     |
| `doubleMD5(data)`      | Performs MD5 hashing twice on input data.                    |
| `writeHashToEEPROM()`  | Saves valid hash to EEPROM.                                  |
| `readHashFromEEPROM()` | Reads hash from EEPROM for comparison.                       |
| `promptForHash()`      | Prompts user via Serial to enter registration code.          |
| `ProtectSetup()`       | Main setup call to validate or prompt for registration.      |

---

## 🧠 Notes

- You must ensure the `EEPROM.begin()` size matches your device limits.
- This system assumes physical access (via Serial) is required for registration.
- You may change the `SALT` constant to something unique to your application.

---

## 📜 License

This project is provided under the MIT License. Modify and use freely.

---
