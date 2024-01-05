//Reciever Code
#include <iostream>
#include <string>
#include <vector>
#include "MicroBit.h"
#include <cstring>

#include "aes.c"
#include "aes.h"
MicroBit uBit;




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
    uint8_t receivedData[512];  // Adjust the size as needed
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
    std::string salt = decryptedText.substr(2,34);
    uBit.serial.printf("\r\n salt Text: %s \r\n", salt.c_str());
    // if (strcmp(saltID.c_str(),"salt123") == 0){
    //   std::string salt = decryptedText.substr(7);
    //   uBit.display.print("X");
    //   uBit.serial.printf("\r\n Salt: %s \r\n", salt.c_str());
    // }

    /**
     * Show Commands
     */
    // Use decrypted string to determine the button press
    if (cmd.find("ax") != std::string::npos) {
        uBit.display.print("A");
         uBit.serial.printf("\nDecrypted Text: %s \r\n", decryptedText.c_str());

    } else if (decryptedText.find("bx") != std::string::npos) {
        uBit.display.print("B");
         uBit.serial.printf("\nDecrypted Text: %s \r\n", decryptedText.c_str());

    } else {
        uBit.display.print("?");
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