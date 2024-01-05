//Reciever Code
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

    std::string cmd = decryptedText.substr(0,1);
    uBit.serial.printf("\r\n cmd Text: %s \r\n", cmd.c_str());
    std::string firstSalt = decryptedText.substr(1,17);
    uBit.serial.printf("\r\n First salt half Text: %s \r\n", firstSalt.c_str());
    std::string xorSalt = decryptedText.substr(17,26);
    uBit.serial.printf("\r\n XORed salt Text: %s \r\n", xorSalt.c_str());
    std::string secondSalt = saltXOR(xorSalt, firstSalt);
    uBit.serial.printf("\r\n Second salt half Text: %s \r\n",secondSalt.c_str());
    std::string salt = firstSalt + secondSalt;
    uBit.serial.printf("\r\n salt Text: %s \r\n", salt.c_str());

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