#include <SyRenSimplified.h>		// Header file from the SyRen website
									// Not included in Arduino library

SyRenSimplified ST; 

void setup()
{
  SyRenTXPinSerial.begin(38400);	// This is the baud rate you chose with the DIP switches.  
  
  ST.motor(1, 0);	// Parameters (pin, speed)
}

void loop()
{
  ST.motor(1,0);	// Sets motor at 0 speed
  delay(100);		// Delay is in milliseconds (100 milliseconds)

  ST.motor(1,127);	// Sets motor at 127 speed (Forward)
  delay(4000);		// Delay is in milliseconds (4 seconds)
  ST.motor(1,0);	// Sets motor at 0 speed
  delay(100);		// Delay is in milliseconds (100 milliseconds)
  ST.motor(1,-20);  // Allows DC motor to move by having a negative vector move in the opposite direction
  
  delay(4000);		// Delay is in milliseconds
  ST.motor(1,0);	// Sets motor at 0 speed
  delay(100);		// Delay is in milliseconds (100 milliseconds)
  
  ST.motor(1,20);   // Allows DC motor to move by having a negative vector move in the opposite direction
  delay(4000);		// Delay is in milliseconds (4 seconds)
  ST.motor(1,-127);	// Sets motor at -127 speed (Reverse)
  delay(4000);		// Delay is in milliseconds (4 seconds)
}