#include "mbed.h"

DigitalOut led1(LED1);
DigitalIn button(USER_BUTTON);
Serial pc(USBTX,USBRX);

// main() runs in its own thread in the OS
int main() {
    while (true) {
        led1 = !led1;
        wait(1);
        pc.printf("Ciao Antonio\n");
    }
}
