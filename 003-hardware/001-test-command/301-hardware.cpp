
#include "MicroBit.h"
MicroBit uBit; 

#define MY_APP_ID           4000
#define SOMETHING           1


void onSomething(MicroBitEvent e)
{
    uBit.display.scrollAsync("Something!");
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

int main()
{
 uBit.init();
 uBit.radio.setGroup(83);
 uBit.messageBus.listen(MY_APP_ID, SOMETHING, onSomething);
 uBit.radio.enable();
 while(1)
        uBit.sleep(1000);
}
