//everyone in the class say thaaaaank you open aiiiiiiii
//#include <stdbool.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>

#include <time.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdio.h>

#define SPI_PORT spi0
#define PIN_MOSI 19 //choosing "SPI0 TX" pin on pinout diagram, GPIO 19
#define PIN_SCK  18 //"SPI0 SCK"
#define PIN_LATCH 15 //arbitary GPIO pin for latch

#define PIN_BUTTON_RXN 22 //physical pin 29 / gpio 22
#define PIN_BUTTON_RESET 13 //physical pin 17 / gpio 13
#define PIN_LED 14 //gpio 14, physical pin 19

int current_time = 0;
int flash_timestamp = 0;
int rxn_time = 0;

int longest_delay = 15000; // longest amount of time to wait before LED flashes to test rxn time, fairly long....
                           // could create some weighting function to make it more likely to give a little lower with some small chance
                           // it generates an extremely long delay
int min_delay = 3000;

int delay_in_ms = 0; //maybe has to be initialized in main()

bool flashed = false;
bool reacted = false;

void shiftOut(uint8_t data) {
    // Bring latch low to start sending data
    gpio_put(PIN_LATCH, 0);

    // Send one byte over SPI
    spi_write_blocking(SPI_PORT, &data, 1);

    // Latch the output to make it appear on Q0–Q7
    gpio_put(PIN_LATCH, 1);

}


void light_all() {
  shiftOut(0xFF);
  shiftOut(0xFF);
}

void lights_off() {
  shiftOut(0x00);
  shiftOut(0x00);
}

void setSevenSeg(int decimalValue) {

  //so far, boldly assuming that the value will fit in 8 bits
  uint8_t ones = decimalValue % 10;
  uint8_t tens = ((decimalValue % 100) - ones)/10;
  uint8_t hundreds = ((decimalValue % 1000) - ones - tens)/100;
  uint8_t thousands = ((decimalValue % 10000) - ones - tens - hundreds)/1000;

  //left-shifting by four to create a byte consisting of the BCD values of a pair of digits
  thousands = thousands << 4;
  tens = tens << 4;

  shiftOut(hundreds+thousands);
  shiftOut(ones+tens);

}

void buttons_isr(uint gpio, uint32_t events) {
  //first, a simple toggling of LED output:
  if(gpio == PIN_BUTTON_RXN) {
    reacted = true;
  }

  if(gpio == PIN_BUTTON_RESET) {
    reacted = false;
    
    srand(to_ms_since_boot(get_absolute_time())); //ensures that you don't get the same pattern for every time you turn on the pico I reckon
    
    setSevenSeg(0);
    
    flashed = false;
    
    flash_timestamp = 0;   
    
    rxn_time = 0;
    
    gpio_put(PIN_LED, false);
    
    delay_in_ms = to_ms_since_boot(get_absolute_time()) + (rand() % (longest_delay + 1 - min_delay)) + min_delay; //new random delay post current time
  }
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
  
    //LED PIN SETUP
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);
    gpio_put(PIN_LED, 0); //initially, LED should be off, is toggled whenever button input goes high

    gpio_init(PIN_BUTTON_RXN);
    gpio_set_dir(PIN_BUTTON_RXN, GPIO_IN);
    
    gpio_init(PIN_BUTTON_RESET);
    gpio_set_dir(PIN_BUTTON_RESET, GPIO_IN);

    gpio_set_irq_enabled_with_callback(PIN_BUTTON_RXN, GPIO_IRQ_LEVEL_HIGH, true, &buttons_isr);
    gpio_set_irq_enabled_with_callback(PIN_BUTTON_RESET, GPIO_IRQ_LEVEL_HIGH, true, &buttons_isr);

    //from stdlib
    srand(time(NULL)); //uses current system time or some such thing in order to initiate a seed for the random number sequence
                       //doesn't really work since the code is running on the pico -- there must be another way
                       //another idea is naturally random numbers could be created with a flashing led in front of a photoresistor being read 
                       //as the game progresses --- besides perhaps the easier software solution of redefining this seed based on some other timer?
                       //or would that still end up deterministic...
    
    delay_in_ms = (rand() % (longest_delay + 1 - min_delay)) + min_delay;
    
    //start at zero and increment up from there
    int test_four_digit = 0;    
    setSevenSeg(test_four_digit); 
    
    while (true) {
      current_time = to_ms_since_boot(get_absolute_time());
      if (reacted == false) {  
        if(rxn_time <= 9999) {
          setSevenSeg(rxn_time); 
        }

        if(current_time >= delay_in_ms && gpio_get(PIN_LED) == false) {
          gpio_put(PIN_LED, true);
          flash_timestamp = current_time;     
          flashed = true;
        }

        if (flashed == true) {
          rxn_time = current_time - flash_timestamp;
        }

      } else {
        sleep_ms(10); //wait for interrupt of user resetting with 2nd button?
      }
    }
}
