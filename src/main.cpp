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

SPIFlash flash(18);
SFE_BMP180 pressure;

struct FlightData {
  double pressure;
  double temperature;
  double time;
};

FlightData data;
uint32_t addr = 0;
uint16_t sentinel = 0x0fac;
unsigned long start = millis();

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
  while (flash.readByte(addr) == 0x00) {
    addr += sizeof(FlightData) + 1;
  }

  Serial.println("Found open address:");
  Serial.println(addr);

  if (!!Serial) {
    // In flight

    storePressureAndTemperature();
    data.time = millis() - start;

    flash.writeByte(addr, 0x00);
    flash.writeAnything(addr + 1, data);
    addr += sizeof(FlightData) + 1;
  } else {
    // Plugged in

    Serial.print("Time,Pressure,Temperature\n");
    while (flash.readByte(addr) != 0x00) {
      flash.readByte(addr);
      flash.readAnything(addr + 1, data);
    
      printDouble(data.time, 6);
      Serial.print(",");
      printDouble(data.pressure, 6);
      Serial.print(",");
      printDouble(data.temperature, 6);
      Serial.print("\n");
      addr += sizeof(FlightData) + 1;
    }

    Serial.print(" --- Would you like to erase the chip? (yes/no)");

    while (!Serial.available()) {
      // Wait
    }

    if (Serial.readString().equals("yes")) {
      Serial.println("Erasing... (this may take a few minutes)");
      flash.eraseChip();
      Serial.println("Finished erasing.");
      addr = 0x00;
    }
  }
}

void storePressureAndTemperature() {
  char status;
  double T,P;

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Use '&T' to provide the address of T to the function.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Use '&P' to provide the address of P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          data.temperature = T;
          data.pressure = P;
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}