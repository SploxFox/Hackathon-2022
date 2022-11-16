/* Pro Micro Test Code
   by: Nathan Seidle
   modified by: Jim Lindblom
   SparkFun Electronics
   date: September 16, 2013
   license: Public Domain - please use this code however you'd like.
   It's provided as a learning tool.

   This code is provided to show how to control the SparkFun
   ProMicro's TX and RX LEDs within a sketch. It also serves
   to explain the difference between Serial.print() and
   Serial1.print().
*/
#include <Arduino.h>
#include <SPIMemory.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <stdio.h>
#include <math.h>


SPIFlash flash;

struct FlightData {
  double pressure;
  double temperature;
  double time;
};

FlightData data;
uint32_t addr = 0;

void setup() {
  Serial.begin(9600); //This pipes to the serial monitor
  Serial.println("Initialize Serial Monitor");

  Serial1.begin(9600); //This is the UART, pipes to sensors attached to board
  Serial1.println("Initialize Serial Hardware UART Pins");

  flash.begin();
}

// https://forum.arduino.cc/t/printing-a-double-variable/44327/7
void printDouble( double val, byte precision){
  // prints val with number of decimal places determine by precision
  // precision is a number from 0 to 6 indicating the desired decimial places
  // example: printDouble( 3.1415, 2); // prints 3.14 (two decimal places)

  Serial.print (int(val));  //prints the int part
  if( precision > 0) {
    Serial.print("."); // print the decimal point
    unsigned long frac;
    unsigned long mult = 1;
    byte padding = precision -1;
    while(precision--)
       mult *=10;
       
    if(val >= 0)
      frac = (val - int(val)) * mult;
    else
      frac = (int(val)- val ) * mult;
    unsigned long frac1 = frac;
    while( frac1 /= 10 )
      padding--;
    while(  padding--)
      Serial.print("0");
    Serial.print(frac,DEC) ;
  }
}

void loop() {
  /*while (flash.readByte(addr) != 0xFF) {
    addr++;
  }

  Serial.println("Found open address %06x", addr);*/

  if (!!Serial) {
    // In flight

    flash.writeByte(addr, 0);
    flash.writeAnything(addr + 1, data);
    addr += sizeof(FlightData) + 1;
  } else {
    // Plugged in

    Serial.println("Time,Pressure,Temperature");
    while (flash.readByte(addr) != 0x00) {
      flash.readByte(addr);
      flash.readAnything(addr + 1, data);
      
      Serial.println(data.time, data.pressure, data.temperature);
      addr += sizeof(FlightData) + 1;
    }

    while (true) {

    }
  }
}




