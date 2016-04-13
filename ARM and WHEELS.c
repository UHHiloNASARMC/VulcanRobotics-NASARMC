/**
 * Daryl Albano
 * 04/12/16
 * ARM and WHEELS.c
 * Vulcan Space Robotics Team
 * Main program to control Spock rover
**/


#include<servo.h>
Servo leftWheels;
Servo rightWheels;
Servo arm;

void setup() {
	int PWMA = 2;
	int AIN1 = 52;
	int AIN2 = 53;

	leftWheels.attach(8);
	rightWheels.attach(9);
	arm.attach(10);
	leftWheels.writeMicroseconds(1500);
	rightWheels.writeMicroseconds(1500);

	Serial.begin(115200);
}

void loop() {
	if(Serial.available()) {
		int input = Serial.read();
		
		/**
		 * Move forward
		**/
		if(input == 'w') {
			leftWheels.writeMicroseconds(1650);
			rightWheels.writeMicroseconds(1650);
			delay(1000);
		}

		/**
		 * Turn left
		**/
		if(input == 'a') {
			leftWheels.writeMicroseconds(1350);
			rightWheels.writeMicroseconds(1350);
			delay(1000);
		}

		/**
		 * Move backwards
		**/
		if(input == 's') {
			leftWheels.writeMicroseconds(1350);
			rightWheels.writeMicroseconds(1650);
			delay(1000);
		}

		/**
		 * Turn right
		**/
		if(input == 'd') {
			leftWheels.writeMicroseconds(1650);
			rightWheels.writeMicroseconds(1350);
			delay(1000);
		}

		/**
		 * arm up
		**/
		if(input == 'i') {
			arm.writeMicroseconds(1650);
			AIN1=HIGH;
			AIN2=HIGH;
			delay(500);
			AIN1=LOW;
			AIN2=LOW;
		}

		/**
		 * arm up
		**/
		if(input == 'n') {
			arm.writeMicroseconds(1350);
			AIN1=HIGH;
			AIN2=HIGH;
			delay(500);
			AIN1=LOW;
			AIN2=LOW;
		}
}



