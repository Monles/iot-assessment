
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
    std::string k2 = md5(sa);
    uBit.serial.printf("\r\n MD5 salt Text: %s \r\n", salt.c_str());

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
    if (decryptedText.substr(0,2) == "ax") {
      uBit.display.print("A");
      uBit.serial.printf("\r\n ax : %s \r\n", cmd.c_str());
      uBit.serial.printf("\r\n salt : %s \r\n", salt.c_str());
      

    }
    if (decryptedText.substr(0,2) == "bx") {
        uBit.display.print("B");

        uBit.serial.printf("\r\n bx : %s \r\n", cmd.c_str());
        uBit.serial.printf("\r\n salt : %s \r\n", salt.c_str());

    } 

    // Print the decrypted string
    // std::cout << "Decrypted Text: " << decryptedText << std::endl;
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