/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "platform/mbed_thread.h"

/*
example of driving maxim chip for Glasgow Uni Projects.

Dr J.J.Trinder 2013,14
jon.trinder@glasgow.ac.uk

 
*/

//DEFINE STATEMENTS FOR 8x8 LED DISPLAY

#define max7219_reg_noop         0x00
#define max7219_reg_digit0       0x01
#define max7219_reg_digit1       0x02
#define max7219_reg_digit2       0x03
#define max7219_reg_digit3       0x04
#define max7219_reg_digit4       0x05
#define max7219_reg_digit5       0x06
#define max7219_reg_digit6       0x07
#define max7219_reg_digit7       0x08
#define max7219_reg_decodeMode   0x09
#define max7219_reg_intensity    0x0a
#define max7219_reg_scanLimit    0x0b
#define max7219_reg_shutdown     0x0c
#define max7219_reg_displayTest  0x0f

#define LOW 0
#define HIGH 1




//  TIMER

Timer timeCount;

// TICKER FOR TIMER INTERRUPT
Ticker t;

// VARIABLES 

AnalogIn Ain(PTB1); //input signal for ADC
AnalogOut Aout(PTE30);

SPI max72_spi(PTD2, NC, PTD1);
DigitalOut load(PTD0); //will provide the load signal

// ARRAYS

char  pattern_diagonal[8] = { 0x01, 0x2,0x4,0x08,0x10,0x20,0x40,0x80};
char  pattern_square[8] = { 0xff, 0x81,0x81,0x81,0x81,0x81,0x81,0xff};  
char  pattern_star[8] = { 0x04, 0x15, 0x0e, 0x1f, 0x0e, 0x15, 0x04, 0x00};
char a_pattern[8] = {0xff, 0x7e, 0x3c, 0x18, 0x18, 0x18, 0x7e, 0x81};
char isix_pattern[16] = {0xff, 0x7e, 0x3c, 0x18, 0x18, 0x18, 0x7e, 0x81, 0xff, 0x7e, 0x3c, 0x18, 0x18, 0x18, 0x7e, 0x81};

// POINTERS 

int *sample;
int *data;


/*
Write to the maxim via SPI
args register and the column data
*/
void write_to_max( int reg, int col)
{
    load = LOW;            // begin
    max72_spi.write(reg);  // specify register
    max72_spi.write(col);  // put data
    load = HIGH;           // make sure data is loaded (on rising edge of LOAD/CS)
}

//writes 8 bytes to the display  
void pattern_to_display(char *testdata){
    int cdata; 
    for(int idx = 0; idx <= 7; idx++) {
        cdata = testdata[idx]; 
        write_to_max(idx+1,cdata);
    }
}

void write_sixteen(char *testdata){
    int cdata; 
    for(int idx = 0; idx <= 15; idx++) {
        cdata = testdata[idx]; 
        write_to_max(idx+1,cdata);
    }
}  
 
// setup matrix display of 8x8 bit display
void setup_dot_matrix ()
{
    // initiation of the max 7219
    // SPI setup: 8 bits, mode 0
    max72_spi.format(8, 0);
     
  
  
       max72_spi.frequency(100000); //down to 100khx easier to scope ;-)
      

    write_to_max(max7219_reg_scanLimit, 0x07);
    write_to_max(max7219_reg_decodeMode, 0x00);  // using an led matrix (not digits)
    write_to_max(max7219_reg_shutdown, 0x01);    // not in shutdown mode
    write_to_max(max7219_reg_displayTest, 0x00); // no display test
    for (int e=1; e<=8; e++) {    // empty registers, turn all LEDs off
        write_to_max(e,0);
    }
   // maxAll(max7219_reg_intensity, 0x0f & 0x0f);    // the first 0x0f is the value you can set
     write_to_max(max7219_reg_intensity,  0x08);     
 
}

//clear the display
void clear(){
     for (int e=1; e<=8; e++) {    // empty registers, turn all LEDs off
        write_to_max(e,0);
    }
}

// Analogue to Digital Converter
int adc(){
    
    unsigned int i = 0;
    
    i = Ain.read_u16();
    Aout.write_u16(i);

    printf("%d,", i); // removed wait(): we can implement it using tick in the main function.
    
    return i;
    }

// Enter data in the Data_Array using the adc() function
void populate_data(int Data_Array[8]){
            for(int i = 0; i < sizeof(Data_Array); i++){
                 Data_Array[i] = adc();
                }
           
    }

// filter data
int filter_data(int data, int prev, int a){
    
    int new_data = a * data + (1-a)*prev;
    
    return new_data;
    
    }
    
int detrend_data(){
    
    return 0;
    }
    
int normalise_range(){
    return 0;
    }
    
int pulse_rate(){
    return 0;
    }
    
    

int main()
{
    
    //start timer
    timeCount.start();
    
    clear();
    
    setup_dot_matrix ();      /* setup matrix */
    
    //Array to store data from ADC !!!!
    int Data_Array[16];
    
    int a = 0.5;
    int new_data[sizeof(Data_Array)];
    
    int roll = 5;
    int roll_datasum = 0;
    int roll_data[sizeof(Data_Array)];
    
    int digi_data[sizeof(Data_Array)];
    
    while(1){
        
        
        
        populate_data(Data_Array);
        
        wait_us(20000);
        
        //filter 
        
        new_data[0] = a * Data_Array[0];
        for(int i = 1; i < sizeof(Data_Array); i++){
            new_data[i] = filter_data(new_data[i], new_data[i-1], a);
        }
        
        //rolling average
        for(int i = roll; i < sizeof(new_data); i++){
            /*for(int z = i; z < i+roll; i++){
                roll_datasum = roll_datasum + new_data[z];
            }*/
            
            roll_data[i] = (new_data[i-1] + new_data[i-2] + new_data[i-3] + new_data[i-4] + new_data[i-5])/roll;
        }
        
        //turn to digital
        for(int i = 0; i < sizeof(roll_data); i++){
            if (roll_data[i] < 39870){
                digi_data[i] = 1;
            }else if (roll_data[i] < 42750){
                digi_data[i] = 2;
            }else if (roll_data[i] < 45620){
                digi_data[i] = 3;
            }else if (roll_data[i] < 48500){
                digi_data[i] = 4;
            }else if (roll_data[i] < 51370){
                digi_data[i] = 5;
            }else if (roll_data[i] < 54250){
                digi_data[i] = 6;
            }else{
                digi_data[i] = 7;
                }
            }
        
        }
    
    /*loop through patterns
    while(1){
    //da_star();
    pattern_to_display(pattern_diagonal);
    wait_ms(1000);
    pattern_to_display(pattern_square);
    wait_ms(1000);
    pattern_to_display(pattern_star);
    wait_ms(1000);
    pattern_to_display(a_pattern);
    wait_ms(1000);
    clear(); */

    }
