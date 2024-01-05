
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

// Function to decrypt a string using TinyAES
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
    uint8_t receivedData[256];  // Adjust the size as needed
    int receivedSize = uBit.radio.datagram.recv(receivedData, sizeof(receivedData));
    uBit.serial.printf("\n Received Size: %d \r\n", receivedSize);

    std::string encryptedText(reinterpret_cast<char*>(receivedData), receivedSize);
    uBit.serial.printf("\n Received Encrypted Text: %s \r\n", encryptedText.c_str());

    // 128-bit key and IV (for AES-128)
    uint8_t key[32] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x97, 0x18, 0x09, 0xcf, 0x4f, 0x3c, 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x97, 0x18, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t iv[16] = {0x00};

    // Decrypt the received message


    /** Salt */
    std::string decryptedText = decrypt(encryptedText, key, iv);
    uBit.serial.printf("\r\n Decrypted Text: %s \r\n", decryptedText.c_str());

    std::string cmd = decryptedText.substr(0,2);
    uBit.serial.printf("\r\n cmd Text: %s \r\n", cmd.c_str());
    std::string sa1 = decryptedText.substr(2,16);
    uBit.serial.printf("\r\n sa1 Text: %s \r\n", sa1.c_str());
    std::string sa2 = cyclicRotate(sa1, 3);
    uBit.serial.printf("\r\n sa2 Text: %s \r\n", sa2.c_str());
    std::string salt = sa1 + sa2;
    uBit.serial.printf("\r\n salt: %s \r\n", salt.c_str());
    std::string k2 = md5(salt);
    uBit.serial.printf("\r\n MD5 hashed salt Text: %s \r\n", k2.c_str());

    
    /**
     * Create k1 for dpk
     * Step 1. Use Sha256 to hash a shared secret
     * Step 2. Separate SHA256 hashed secret into two halves
     * Step 3. XOR two halves, and the result is k2
     */
    // Import SHA256 Library
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
    std::string dpk = k1;
    dpk.append(k2);


    /**
     * Show Commands
     */
    // Use decrypted string to determine the button press
    if (decryptedText.substr(0,2) == "zy") {
      uBit.display.print("A");
      uBit.serial.printf("\r\n ax : %s \r\n", cmd.c_str());
      uBit.serial.printf("\r\n salt : %s \r\n", salt.c_str());
      
      // Run the command
      // Variable holding the speed
      // int duty = 0;
      // // The instructions are inside a forever loop, the motor will speed up and slow down forever.
      // while(1)
      // {
      //     // The first loop writes the value of the variable “duty” to Pin P0 and increases by 1 in every step until reaching the maximum value of the fan motor speed (1023).
      //     while(duty < 1023)
      //     {
      //       uBit.io.P0.setAnalogValue(duty);
      //       duty ++;
      //       uBit.sleep(10);
      //     }
      //     // The second loop writes the value of the variable “duty” to Pin P0 and decreases by 1 in every step until reaching the reaching 0 (stopping the fan).
      //     while(duty > 0)
      //     {
      //       uBit.io.P0.setAnalogValue(duty);
      //       duty --;
      //       uBit.sleep(10);
      //     }
      // }

    }
    if (decryptedText.substr(0,2) == "ax") {
      uBit.display.print("A");
      uBit.serial.printf("\r\n ax : %s \r\n", cmd.c_str());
      uBit.serial.printf("\r\n salt : %s \r\n", salt.c_str());
      
      // Run the command
      // initialize an integer variable that will contain the brightness level.
      // int light=0;
      // while(1)
      // {
      //   // Read the voltage at the pin P0 that is converted to a value in the range of 0 – 1024. It 
      //   represents the sensed lightness. 
      //   light = uBit.io.P0.getAnalogValue(); 
      //   if(light >= 200)
      //   {
      //     // Create then display a sun image
      //     MicroBitImage smiley("0,255,255,255,0\n255,0,0,0,255\n255,0,0,0,255\n255,0,0,0, 
      //     255\n0,255,255,255,0\n");
      //     uBit.display.print(smiley);
      //   }
      //   else
      //   {
      //     // Create then display a moon image
      //     MicroBitImage 
      //     smiley("0,0,255,255,255\n0,255,255,0,0\n255,255,0,0,0\n255,255,255,0,0\n0, 
      //     255,255,255,255\n");
      //     uBit.display.print(smiley);
      //   }
      // }

    }
    if (decryptedText.substr(0,2) == "bx") {
        uBit.display.print("B");

        uBit.serial.printf("\r\n bx : %s \r\n", cmd.c_str());
        uBit.serial.printf("\r\n salt : %s \r\n", salt.c_str());

        // Run the command
        // // Define LED pins
        // MicroBitPin P0(MICROBIT_ID_IO_P0, MICROBIT_PIN_P0, PIN_CAPABILITY_DIGITAL);
        // MicroBitPin P1(MICROBIT_ID_IO_P1, MICROBIT_PIN_P1, PIN_CAPABILITY_DIGITAL);
        // MicroBitPin P2(MICROBIT_ID_IO_P2, MICROBIT_PIN_P2, PIN_CAPABILITY_DIGITAL);

        // while(1) {
        //   // Red - turn amber LED off and red LED on
        //   P1.setDigitalValue(0);
        //   P0.setDigitalValue(1);
        //   uBit.sleep(4000); // Delay for 4 seconds

        //   // Amber - turn red LED off and amber LED on
        //   P0.setDigitalValue(0);
        //   P1.setDigitalValue(1);
        //   uBit.sleep(1000);

        //   // Green - turn amber LED off and green LED on
        //   P1.setDigitalValue(0);
        //   P2.setDigitalValue(1);
        //   uBit.sleep(4000);

        //   // Amber - turn green LED off and amber LED on
        //   P2.setDigitalValue(0);
        //   P1.setDigitalValue(1);
        //   uBit.sleep(1000);
        // }

    } 
}



int main() {
    uBit.init();
    uBit.display.print("R");  // Indicate that the receiver is ready
    uBit.radio.enable();

    // Register the event handler function for radio datagram events
    uBit.messageBus.listen(MICROBIT_ID_RADIO,MICROBIT_RADIO_EVT_DATAGRAM, onDataReceived);
   
    while (1) {
        uBit.sleep(100);
    }

    return 0;
}