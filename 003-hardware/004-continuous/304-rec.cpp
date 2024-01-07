#include "MicroBit.h"
#include "MicroBitUARTService.h"
#include <iostream>
#include <random>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstring>

#include "sha256.h"
#include <md5.h>


#include "aes.c"
#include "aes.h"



MicroBit uBit;



/** 
 * Run commands
 * */ 




// Fan
void runFan(){
    uBit.display.scrollAsync("Fan");
    // Variable holding the speed
    int duty = 0;
    // The instructions are inside a forever loop, the motor will speed up and slow down forever.
    while(1)
    {
        // The first loop writes the value of the variable “duty” to Pin P0 and increases by 1 in every step until reaching the maximum value of the fan motor speed (1023).
        while(duty < 1023)
        {
            uBit.io.P0.setAnalogValue(duty);
            duty ++;
            uBit.sleep(10);
        }
        // The second loop writes the value of the variable “duty” to Pin P0 and decreases by 1 in every step until reaching the reaching 0 (stopping the fan).
        while(duty > 0)
        {
            uBit.io.P0.setAnalogValue(duty);
            duty --;
            uBit.sleep(10);
        }
    }

}



// Run commands
void runLightSensor(){
      uBit.display.scrollAsync("Light Sensor");
      // Run the command
      // initialize an integer variable that will contain the brightness level.
      int light=0;
      while(1)
      {
        // Read the voltage at the pin P0 that is converted to a value in the range of 0 – 1024. It 
        // represents the sensed lightness. 
        light = uBit.io.P0.getAnalogValue(); 
        if(light >= 200)
        {
          // Create then display a sun image
          MicroBitImage smiley("0,255,255,255,0\n255,0,0,0,255\n255,0,0,0,255\n255,0,0,0,255\n0,255,255,255,0\n");
          uBit.display.print(smiley);
        }
        else
        {
          // Create then display a moon image
          MicroBitImage 
          smiley("0,0,255,255,255\n0,255,255,0,0\n255,255,0,0,0\n255,255,255,0,0\n0,255,255,255,255\n");
          uBit.display.print(smiley);
        }
        runFan();
      }
}

void runLed(){
    uBit.display.scrollAsync("LED");
    // Run the command
    // Define LED pins
    MicroBitPin P0(MICROBIT_ID_IO_P0, MICROBIT_PIN_P0, PIN_CAPABILITY_DIGITAL);
    MicroBitPin P1(MICROBIT_ID_IO_P1, MICROBIT_PIN_P1, PIN_CAPABILITY_DIGITAL);
    MicroBitPin P2(MICROBIT_ID_IO_P2, MICROBIT_PIN_P2, PIN_CAPABILITY_DIGITAL);

    while(1) {
        // Red - turn amber LED off and red LED on
        P1.setDigitalValue(0);
        P0.setDigitalValue(1);
        uBit.sleep(4000); // Delay for 4 seconds

        // Amber - turn red LED off and amber LED on
        P0.setDigitalValue(0);
        P1.setDigitalValue(1);
        uBit.sleep(1000);

        // Green - turn amber LED off and green LED on
        P1.setDigitalValue(0);
        P2.setDigitalValue(1);
        uBit.sleep(4000);

        // Amber - turn green LED off and amber LED on
        P2.setDigitalValue(0);
        P1.setDigitalValue(1);
        uBit.sleep(1000);
    }
    

}

std::string cyclicRotate(const std::string &originalString, int shift) {
  std::string rotatedString;
  size_t length = originalString.length();

  // Ensure shift value is within the string length
  shift = shift % length;

  for (size_t i = 0; i < length; ++i) {
    int newIndex = (i + shift) % length;
    rotatedString += originalString[newIndex];
  }

  return rotatedString;
}

/**
 * XOR two Halves in a SHA256 hashed string
 * Step 1. Two halves should have same length
 * Step 2. After checking the length, XOR each halves
 *  */

// Function to perform XOR on two hexadecimal strings
std::string xorHexStrings(const std::string &str1, const std::string &str2) {
    if (str1.length() != str2.length()) {
        std::cerr << "Error: Input strings must be of equal length." << std::endl;
        return "";
    }

    std::stringstream result;

    for (size_t i = 0; i < str1.length(); ++i) {
        // Perform bitwise XOR on each character and convert to hexadecimal
        int hexValue = std::stoi(str1.substr(i, 1), nullptr, 16) ^ std::stoi(str2.substr(i, 1), nullptr, 16);
        result << std::hex << hexValue;
    }

    return result.str();
}

/**
 * AES-ECB-256 
 * Step 1. Encryption 
 * Step 2. Decryption
 */

// step 1. Function to encrypt a string using TinyAES
std::string encrypt(const std::string &input, const uint8_t *key, const uint8_t *iv) {
    std::vector<uint8_t> inputBytes(input.begin(), input.end());

    // Pad the input if needed
    size_t padding = 16 - (inputBytes.size() % 16);
    if (padding != 0) {
        inputBytes.insert(inputBytes.end(), padding, static_cast<uint8_t>(padding));
    }

    AES_ctx ctx;
    AES_init_ctx(&ctx, key);

    // Encrypt the data
    AES_ECB_encrypt(&ctx, inputBytes.data());

    return std::string(inputBytes.begin(), inputBytes.end());
}

// Step 2. Function to decrypt a string using TinyAES
std::string decrypt(const std::string &input, const uint8_t *key, const uint8_t *iv) {
    std::vector<uint8_t> inputBytes(input.begin(), input.end());

    AES_ctx ctx;
    AES_init_ctx(&ctx, key);

    // Decrypt the data
    AES_ECB_decrypt(&ctx, inputBytes.data());

    // Remove padding
    size_t padding = inputBytes.back();
    inputBytes.resize(inputBytes.size() - padding);

    return std::string(inputBytes.begin(), inputBytes.end());
}


// Event handler function for radio datagram events
void onDataReceived(MicroBitEvent) {

    // Assuming receive expects a uint8_t* buffer and length
    uint8_t receivedData[512];  // Adjust the size as needed
    int receivedSize = uBit.radio.datagram.recv(receivedData, sizeof(receivedData));
    uBit.serial.printf("\n Received Size: %d \r\n", receivedSize);

    std::string enc(reinterpret_cast<char*>(receivedData), receivedSize);
    uBit.serial.printf("\n Received Encrypted Text: %s \r\n", enc.c_str());

    /** Salt */
    std::string sa1 = enc.substr(0,8);
    std::string sa2 = cyclicRotate(sa1, 3);
    std::string sa = sa1+sa2;
    std::string salt = sa+sa;
    std::string k2 = md5(salt);
    
    uBit.serial.printf("\r\n  Salt: %s\r\n", salt.c_str());
    uBit.serial.printf("\r\n  k2: %s\r\n", k2.c_str());
    
    
    
    /**
     * Create k1 for dpk
     * Step 1. Use Sha256 to hash a shared secret
     * Step 2. Separate SHA256 hashed secret into two halves
     * Step 3. XOR two halves, and the result is k2
     */
    // // Import SHA256 Library
    SHA256 sha256;

    /**
     * Shared Secret
     * For Decryting DPK in Receiver's side
     */

    ManagedString a("uwe");
    const char* b = a.toCharArray();
    std::string c(b);

    // Step 1. SHA256 Hashed secret
    c = sha256(c).c_str();
    size_t length = c.length();
    size_t middleIndex = length / 2;
   
     
    // Step 2. Separate SHA256 hashed secret into two halves 
    // Extract the first half
    std::string firstHalf = c.substr(0, middleIndex);

    // Extract the second half
    std::string secondHalf = c.substr(middleIndex);

    // Step 3. XOR two halves of SHA256 Hashed secret
    std::string xorResult = xorHexStrings(firstHalf.c_str(), secondHalf.c_str());
    std::string k1 = xorResult;

     /**
     * DPK = k1 + k2
     * which used as a key for AES-ECB-256 to encrypt the command
     */
    std::string kk = k1;
    std::string dpk = kk + k2;
    uBit.serial.printf("\r\n Received DPK: %s \r\n", dpk.c_str());

    
    /**
     * Convert DPK into an uint8_t array for AES
     * 256-bit key and IV (Not neccessary here)
     * */
   // 256-bit key 
    uint8_t key[32] = {0x00};
    
    for (int i = 0; i < 32; ++i) {
      std::string byteString = dpk.substr(i * 2, 2);
      int byteValue = std::stoi(byteString, nullptr, 16);
      key[i] = static_cast<uint8_t>(byteValue);
    }
    // for (size_t i = 0; i < 32; ++i) {
    //     sscanf(dpk.substr(2 * i, 2).c_str(), "%02x",(unsigned int *)&key[i]);
    // }
    uint8_t iv[16] = {0x00};
    // The rest is cipher
    std::string encryptedText = enc.substr(8);
    uBit.serial.printf("\r\n  encryptedText: %s\r\n", encryptedText.c_str());

    std::string decryptedText = decrypt(encryptedText, key, iv);
    uBit.serial.printf("\r\n Decrypted Text: %s \r\n", decryptedText.c_str());


    /**
     * Show Commands
     */
   
    if (decryptedText.substr(0,2) == "ax") {

      uBit.display.print("A");
      uBit.sleep(1000);

      uBit.display.scrollAsync("Run FAN +  LED!!!");
      uBit.serial.printf("\r\n Command : %s \r\n", decryptedText.substr(0,2).c_str());
      uBit.serial.printf("\r\n Run the Command! Run LED + FAN!!! \r\n");
      uBit.serial.printf("\r\n Hold on... \r\n ");
      // Run the command
      runFan();
      uBit.sleep(1000);
      runLed();
      uBit.sleep(1000);

    }
     // Use decrypted string to determine the button press
    else if (decryptedText.substr(0,2) == "bx") {
      uBit.display.print("B");
      uBit.sleep(1000);
      uBit.display.scrollAsync("Run Light Sensor!!!");
      uBit.serial.printf("\r\n Command : %s \r\n", decryptedText.substr(0,2).c_str());
      uBit.serial.printf("\r\n Run the Command! Run Light Sensor!!! \r\n");

      uBit.serial.printf("\r\n Hold on... \r\n ");
      // Run the command
      runLightSensor();
      uBit.sleep(1000);
    }
    else if (decryptedText.substr(0,2) == "cx") {
        uBit.display.print("C");
         uBit.sleep(1000);
        uBit.display.scrollAsync("Run LED + Light Sensor!!!");
        uBit.serial.printf("\r\n Command : %s \r\n", decryptedText.substr(0,2).c_str());
      uBit.serial.printf("\r\n Run the Command! LED + Light Sensor!!! \r\n");

        uBit.serial.printf("\r\n Hold on... \r\n ");
        // Run the command
        runLed();
        uBit.sleep(1000);
        runLightSensor();
        uBit.sleep(1000);

    } 
}



int main() {
    // Initialise the micro:bit runtime.
    uBit.init();
    // Sets the radio to listen to packets sent with the given group id.
    uBit.radio.setGroup(83);
    // Indicate that the receiver is ready
    uBit.display.print("R"); 
    uBit.radio.enable();
    uBit.serial.printf("\n *******Receiver receives a cipher******* \r\n");

    // Register the event handler function for radio datagram events
    while (1) {
    uBit.messageBus.listen(MICROBIT_ID_RADIO,MICROBIT_RADIO_EVT_DATAGRAM, onDataReceived);
        
        uBit.sleep(100);
    }

    return 0;
}