//everyone in the class say thaaaaank you open aiiiiiiii

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdio.h>

#define SPI_PORT spi0
#define PIN_MOSI 19 //choosing "SPI0 TX" pin on pinout diagram, GPIO 19
#define PIN_SCK  18 //"SPI0 SCK"
#define PIN_LATCH 15 //arbitary GPIO pin for latch

void shiftOut(uint8_t data) {
    // Bring latch low to start sending data
    gpio_put(PIN_LATCH, 0);

    // Send one byte over SPI
    //if data is a single value i.e. just a unit8_t, the spi write function uses the address, so you take the address with &(variable)
    //chatgpt brought up that you could, with an array of bytes passed as the input, transmit them all at once, if the array is passed to the function
    //the array variable acts as a pointer to the starting address of the whole array, with the whole thing written all in one g

    //I think I would have a large array with an "active index" that would shift one way or the other depending on direction, which would then
    //be interpreted as a number of bytes / converted every 8 bits into these bytes that I would write one by one in a for loop to the shift register daisychain
    //but anyways...

    spi_write_blocking(SPI_PORT, &data, 1);

    // Latch the output to make it appear on Q0–Q7
    gpio_put(PIN_LATCH, 1);

}

int main() {
    stdio_init_all();

    // Initialize SPI
    spi_init(SPI_PORT, 1 * 1000 * 1000); // 1 MHz, which means 1 uS period?
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);

    // Latch pin setup
    gpio_init(PIN_LATCH);
    gpio_set_dir(PIN_LATCH, GPIO_OUT);

    // Set latch high initially
    gpio_put(PIN_LATCH, 1);

    printf("Starting SPI shift register test...\n");

    while (true) {
        //predicted output: should shift the 1 continuously up the chain, and then perhaps back down

        for (uint8_t i = 0; i < 8; i++) {
            uint8_t value = 1 << i;
            shiftOut(value);
            sleep_ms(200);
        }

        //the reason it chose 6 is actually interesting - it left off at 7 above, and is going to start going back down -- the new byte of information
        //is created on every interval. very cool stuff!
        for (int8_t i = 6; i >= 0; i--) {
            uint8_t value = 1 << i;
            shiftOut(value);
            sleep_ms(200);
        }
    }
}
