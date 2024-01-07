#include "MicroBit.h"

MicroBit uBit;

void runLed() {
    // Variable holding the speed
      int duty = 0;
      // The instructions are inside a forever loop, the motor will speed up and slow down forever.
      while(1)
      {
          // The first loop writes the value of the variable “duty” to Pin P0 and increases by 1 in every step until reaching the maximum value of the fan motor speed (1023).
          while(duty < 1023)
          {
            uBit.io.P0.setAnalogValue(duty);
            duty++;
            uBit.sleep(10);
          }
          // The second loop writes the value of the variable “duty” to Pin P0 and decreases by 1 in every step until reaching the reaching 0 (stopping the fan).
          while(duty > 0)
          {
            uBit.io.P0.setAnalogValue(duty);
            duty--;
            uBit.sleep(10);
          }
      }
}

void runFan() {
    // Define LED pins
    MicroBitPin P0(MICROBIT_ID_IO_P0, MICROBIT_PIN_P0, PIN_CAPABILITY_DIGITAL);
    MicroBitPin P1(MICROBIT_ID_IO_P1, MICROBIT_PIN_P1, PIN_CAPABILITY_DIGITAL);
    MicroBitPin P2(MICROBIT_ID_IO_P2, MICROBIT_PIN_P2, PIN_CAPABILITY_DIGITAL);
    bool yes = true;
    uint64_t start_time = system_timer_current_time(); // Record the start time

    while (yes) {
        // Measure elapsed time in milliseconds
        uint64_t current_time = system_timer_current_time();
        uint64_t elapsed_time = (current_time - start_time) / 1000;

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
        uBit.sleep(1000); // Convert to milliseconds

        if (elapsed_time >= 25000) { // 25000 milliseconds = 25 seconds
            // Turn off all LEDs and exit the loop after 25 seconds
            P0.setDigitalValue(0);
            P1.setDigitalValue(0);
            P2.setDigitalValue(0);
            yes = false;
            break;

        }

        
    }
}

void runLightSensor() {
    int light = 0;
    uint64_t start_time = system_timer_current_time(); // Record the start time

    while (true) {
        uint64_t current_time = system_timer_current_time();
        uint64_t elapsed_time = (current_time - start_time) / 1000; // Convert to milliseconds

        if (elapsed_time >= 20000) { // 20000 milliseconds = 20 seconds
            // Clear the display and exit the loop after 20 seconds
            uBit.display.clear();
            break;
        }

        light = uBit.io.P0.getAnalogValue();

        if (light >= 200) {
            // Display a sun image
            MicroBitImage sun("0,255,255,255,0\n255,0,0,0,255\n255,0,0,0,255\n255,0,0,0,255\n0,255,255,255,0\n");
            uBit.display.print(sun);
        } else {
            // Display a moon image
            MicroBitImage moon("0,0,255,255,255\n0,255,255,0,0\n255,255,0,0,0\n255,255,255,0,0\n0,255,255,255,255\n");
            uBit.display.print(moon);
        }
    }
}


int main() {
    // Initialization
    uBit.init();

    // Call the function to run LED sequence for 8 seconds
    runLed();
    uBit.sleep(20000);
    runFan();
    uBit.sleep(5000);

    runLightSensor();

    // Main event loop
    while (1) {
        uBit.sleep(100);
    }

    return 0;
}
