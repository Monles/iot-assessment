
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
 * Salt Generation 
 * Step 1. generate 16 characters randomly (sa1)
 * Step 2. duplicate the salt from step 1, and cyclic rotating them 3 chars (sa2)
 * Step 3. combine them into a salt (salt = sa1 + sa2) 
 * 
 * Note:
 * # 1. Final salt is 128-bit = 16 bytes = 32 characters
 * # 2. This method can reduced the length for sending the cipher
 * # 3. Also it's able to be produce the same salt in the receiver's side
 */

// step 1 - sa1
std::string generateSalt(int length) {
  const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUV"
                              "WXYZ0123456789!£$%^&*():@~#'?/><.,;[]|";
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> distribution(0, charset.size() - 1);

  std::string salt;
  for (int i = 0; i < length; ++i) {
    salt += charset[distribution(gen)];
  }

  return salt;
}

// step 2 - sa2
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
 * AES-ECB-256 
 * Step 1. Encryption 
 * Step 2. Decryption
 */

// step 1. Function to encrypt a string using TinyAES
std::string encrypt(const std::string &input, const uint8_t *key, const uint8_t *iv) {
    std::vector<uint8_t> inputBytes(input.begin(), input.end());

    // Pad the input if needed
    size_t padding = 32 - (inputBytes.size() % 32);
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


int main() {

    // Initialise the micro:bit runtime.
    uBit.init();
    // Sets the radio to listen to packets sent with the given group id.
    uBit.radio.setGroup(83);
    // Printing "S" means Sender
    uBit.display.print("S");
    uBit.radio.enable();

    uBit.serial.printf("\n*********Sender is Here for service********\n\r");
   
   
    /**
     * Create k2 for dpk
     * SALT is randomly generated in 128-bit = 16 bytes = 32 characters
     * A 128-bit SALT is typically represented using hexadecimal notation, 
     * which translates to 16 bytes or 32 characters in a string 
     * (assuming each byte is represented by two hexadecimal characters).
     */
    int saltLength = 16; 
    std::string sa1 = generateSalt(saltLength);
    std::string sa2 = cyclicRotate(sa1, 3);
    std::string salt = sa1+sa2;
    std::string k2 = md5(salt);
    
    uBit.serial.printf("\r\n  Salt: %s\r\n", salt.c_str());
    uBit.serial.printf("\r\n  k2: %s\r\n", k2.c_str());

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
     * Print the variables for DEMO
     */
    // // Original Strign of Secret
    uBit.serial.printf("\n\r  Secret: %s \n", a.toCharArray());
    // // SHA256 Hashed secret
    uBit.serial.printf("\n\r  SHA256 hashed Secret: %s \n", c.c_str());
    // // Original First Half of Secret
    uBit.serial.printf("\r\n  FirstHalf of Secret: %s\r\n", firstHalf.c_str());
    // // Original Second Half of Secret
    uBit.serial.printf("\r\n  SecondHalf of Secret: %s\r\n", secondHalf.c_str());

    // Xored SHA256 Hashed Secret (32 bytes = 256 bits)
    uBit.serial.printf("\n\r  XORed is: %s \n", xorResult.c_str());
    uBit.serial.printf("\n\r  k2: %s \n", k2.c_str());
    // MD5
    // // Original Salt before MD5 Hashed
    uBit.serial.printf("\r\n  salt %s\n", salt.c_str());

    // MD5 Hashed Salt (16 bytes = 128 bits)
    uBit.serial.printf("\r\n  Md5 Hashed salt %s \n", k2.c_str());
    uBit.serial.printf("\n\r  k1: %s \n", k1.c_str());

    // DPK
    uBit.serial.printf("\n\r  dpk: %s \n", dpk.c_str());
    
    
    /**
     * Press 1 of 3 buttons to send the command to the reciver
     * When the receiver gets the command, one device will be triggered
     * Button A - Lighte Sensor
     * Button B - LED
     * Button A + B - Fan
     */
    uBit.serial.printf("\r\n*******Press a button to send a command*******\n\r");
    uBit.serial.printf("\r\n*******3 Options: A, B or A + B*******\n\r");


    /**
     * Convert DPK into an uint8_t array for AES
     * 256-bit key and IV (Not neccessary here)
     * 
     * */

    // 256-bit key 
    uint8_t key[32] = {0x00};

    for (size_t i = 0; i < 32; ++i) {
        sscanf(dpk.substr(2 * i, 2).c_str(), "%02x",
            (unsigned int *)&key[i]);
    }

    // This is the dummy key for testing purpose
    // uint8_t key[32] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x97, 0x18, 0x09, 0xcf, 0x4f, 0x3c, 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x97, 0x18, 0x09, 0xcf, 0x4f, 0x3c};

    uint8_t iv[16] = {0x00};

    uBit.sleep(1000);
    

    while (1) {
        // Check if button A + b both are pressed
        if (uBit.buttonA.isPressed() && uBit.buttonB.isPressed()) {

            // Create a text combined with command + salt(first 16 characters)
            std::string command = "zy";
            std::string originalText = command + sa1;
            // Use AES-ECB-256 to ecrypt the text
            std::string encryptedText = encrypt(originalText, key, iv);
            // std::string encryptedText2 = encrypt(encryptedText, key, iv);

            // Print the original and encrypted strings
            uBit.serial.printf("\r\n command(The first 2 chars) + salt: %s\r\n", originalText.c_str());
            uBit.serial.printf("\r\n\r Encrypted Text: %s\r\n", encryptedText.c_str());

            // Send the encrypted message
            uBit.radio.datagram.send(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(encryptedText.data())), encryptedText.size());

            // Introduce a delay before checking the button state again
            uBit.sleep(1000);
        }

            
        // Check if button A is pressed
        if (uBit.buttonA.isPressed()) {
            // Create a text combined with command + salt(first 16 characters)
            std::string command = "ax";
            std::string originalText = command + sa1;
            // Use AES-ECB-256 to ecrypt the text
            std::string encryptedText = encrypt(originalText, key, iv);

            // Print the original and encrypted strings
            uBit.serial.printf("\r\n  command(The first 2 chars) + salt: %s\r\n", originalText.c_str());
            uBit.serial.printf("\r\n\r Encrypted Text: %s\r\n", encryptedText.c_str());

            // Send the encrypted message
            uBit.radio.datagram.send(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(encryptedText.data())), encryptedText.size());

        // Introduce a delay before checking the button state again
            uBit.sleep(1000);

            
        }

        // Check if button B is pressed
        if (uBit.buttonB.isPressed()) {

            // Create a text combined with command + salt(first 16 characters)
            std::string command = "bx";
            std::string originalText = command + sa1;

            // Use AES-ECB-256 to ecrypt the text
            std::string encryptedText = encrypt(originalText, key, iv);

            // Print the original and encrypted strings
            uBit.serial.printf("\r\n  command(The first 2 chars) + salt: %s\r\n", originalText.c_str());
            uBit.serial.printf("\r\n\r Encrypted Text: %s\r\n", encryptedText.c_str());

            // Send the encrypted message
            uBit.radio.datagram.send(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(encryptedText.data())), encryptedText.size());
            
            // Introduce a delay before checking the button state again
            uBit.sleep(1000);
        }
    }

    return 0;
}