//Sender Code
#include <iostream>
#include <string>
#include <vector>
#include "MicroBit.h"
#include <cstring>
#include <cstdint>
#include <iomanip>
#include <sstream>

MicroBit uBit;

#include "aes.c"
#include "aes.h"


// Function to perform XOR operation on two strings in hexadecimal representation
std::string saltXOR(const std::string& str1, const std::string& str2) {
    std::stringstream result;
    for (size_t i = 0; i < str1.length() && i < str2.length(); ++i) {
        result << std::hex << (std::stoi(str1.substr(i, 1), nullptr, 16) ^
                   std::stoi(str2.substr(i, 1), nullptr, 16));
    }
    return result.str();
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

// Function to encrypt a string using TinyAES
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

int main() {
    uBit.init();
    uBit.display.print("T");
    uBit.radio.enable();

    // 128-bit key and IV (for AES-128)
    uint8_t key[32] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x97, 0x18, 0x09, 0xcf, 0x4f, 0x3c, 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x97, 0x18, 0x09, 0xcf, 0x4f, 0x3c};

    uint8_t iv[16] = {0x00};

    std::string salt = "ceb20772e0c9d240c75eb26b0e37abee";
    std::string firstSalt = salt.substr(0, salt.length() / 2);
    std::string secondSalt = salt.substr(salt.length() / 2);
    uBit.serial.printf("\r\n firstSalt: %s\r\n",firstSalt.c_str());
    uBit.serial.printf("\r\n secondSalt: %s\r\n", secondSalt.c_str());

    std::string xorSalt = saltXOR(firstSalt, secondSalt);
    uBit.serial.printf("\r\n xorSalt: %s\r\n", xorSalt.c_str());


    std::string originalSalt = saltXOR(xorSalt, secondSalt);
    originalSalt = firstSalt + originalSalt;
    uBit.serial.printf("\r\n  originalSalt: %s\r\n", originalSalt.c_str());


    
    // Add a head at the top of salt, so we can identify it in the receiver's side
    uBit.sleep(1000);
    

    while (1) {

        
            
        // Check if button A is pressed
        if (uBit.buttonA.isPressed()) {

            
             
            
            std::string header = "a";
            std::string originalText = header +  firstSalt + xorSalt;
            // std::string originalTexts2 = header + salt2;
            std::string encryptedText = encrypt(originalText, key, iv);
            // std::string encryptedTexts2 = encrypt(originalTexts2, key, iv);

            // Print the original and encrypted strings
            uBit.serial.printf("\r\n Original Text: %s\r\n", originalText.c_str());
            uBit.serial.printf("\r\n\r Encrypted Text: %s\r\n", encryptedText.c_str());

            // // // Decrypt the received message
            // std::string decryptedText = decrypt(encryptedText, key, iv);
            // uBit.serial.printf("\r\n*Decrypted Text: %s\r\n", decryptedText.c_str());
            // Send the encrypted message
            uBit.radio.datagram.send(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(encryptedText.data())), encryptedText.size());
            // uBit.radio.datagram.send(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(encryptedTexts2.data())), encryptedTexts2.size());

           

            uBit.sleep(1000);

            
        }

        // Check if button B is pressed
        if (uBit.buttonB.isPressed()) {

            
             
            
            std::string header = "b";
            std::string originalText = header +  firstSalt + xorSalt;

            // std::string originalTexts2 = header + salt2;
            std::string encryptedText = encrypt(originalText, key, iv);
            // std::string encryptedTexts2 = encrypt(originalTexts2, key, iv);

            // Print the original and encrypted strings
            uBit.serial.printf("\r\n Original Text: %s\r\n", originalText.c_str());
            uBit.serial.printf("\r\n\r Encrypted Text: %s\r\n", encryptedText.c_str());

            // // // Decrypt the received message
            // std::string decryptedText = decrypt(encryptedText, key, iv);
            // uBit.serial.printf("\r\n*Decrypted Text: %s\r\n", decryptedText.c_str());
            // Send the encrypted message
            uBit.radio.datagram.send(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(encryptedText.data())), encryptedText.size());
            // uBit.radio.datagram.send(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(encryptedTexts2.data())), encryptedTexts2.size());


            
    
            // Introduce a delay before checking the button state again
            uBit.sleep(1000);
        }
    }

    return 0;
}