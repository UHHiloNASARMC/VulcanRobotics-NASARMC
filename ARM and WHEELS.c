/**
 * Daryl Albano
 * 04/12/16
 * ARM and WHEELS.c
 * Vulcan Space Robotics Team
 * Main program to control Spock rover (Controls movement and bucket)
**/


#include <Servo.h>
Servo leftWheels;
Servo rightWheels;
Servo arm;
int armBrake = 13;
int AIN1 = 49;
int AIN2 = 48;
int levelLeft[3] = {1600, 1700, 1800};
int levelRight[3] = {1400, 1300, 1200};
int i = 0; // Keeps track of the speed array index
int timeInterval = 0; // Kills machine after a certain time of zero user input
int input; // Stores the user's input

void setup() {
  pinMode(armBrake, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(AIN1, OUTPUT);
  leftWheels.attach(10);
  rightWheels.attach(12);
  arm.attach(8);
  leftWheels.writeMicroseconds(1500);
  rightWheels.writeMicroseconds(1500);
	arm.writeMicroseconds(1500);
	analogWrite(armBrake, 0);
  Serial.begin(57600);
}

void loop() {
	if(Serial.available()) {
		input = Serial.read();
		do {
		if(input == 'w') forward();
		if(input == 's') backward();
		if(input == 'a') left();
		if(input == 'd') right();
		if(input == 'x') break;
		if(input == 'q') {speedDown(); break;}
		if(input == 'e') {speedUp(); break;}
		if(input == 'i') {armUp(); break;}
		if(input == 'n') {armDown(); break;}
		timeInterval++;
		} while(checkInput() == true || timeInterval < 6);
		kill();
	}
}

void speedUp() {if(i < 2) {i=i+1; reportSpeed();} else {Serial.println("Max speed reached"); reportSpeed(); return;}}

void speedDown() {if(i > 0) {i=i-1; reportSpeed();} else {Serial.println("Min speed reached"); reportSpeed(); return;}}

void kill() {
	Serial.println("Killed");
	leftWheels.writeMicroseconds(1500);
	rightWheels.writeMicroseconds(1500);
	arm.writeMicroseconds(1500);
	analogWrite(armBrake, 0);
	timeInterval = 0;
}

void reportSpeed() {
	Serial.print("Current speed left: ");
	Serial.println(levelLeft[i]);
	Serial.print("Current speed right: ");
	Serial.println(levelRight[i]);
}

void forward() {
	leftWheels.writeMicroseconds(levelRight[i]);
	rightWheels.writeMicroseconds(levelLeft[i]);
	Serial.println("Moving forward");
	delay(500);
}

void backward() {
	leftWheels.writeMicroseconds(levelLeft[i]);
	rightWheels.writeMicroseconds(levelRight[i]);
	Serial.println("Moving backwards");
	delay(500);
}

void left() {
	leftWheels.writeMicroseconds(levelLeft[i]);
	rightWheels.writeMicroseconds(levelLeft[i]);
	Serial.println("Turning left");
	delay(500);
}

void right() {
	leftWheels.writeMicroseconds(levelRight[i]);
	rightWheels.writeMicroseconds(levelRight[i]);
	Serial.println("Turning right");
	delay(500);
}

void armUp() {
	timeInterval = 0;
	do{
		if(input == 'w') forward();
		else if(input == 's') backward();
		else if(input == 'a') left();
		else if(input == 'd') right();
		else if(input == 'q') {speedDown(); break;}
		else if(input == 'e') {speedUp(); break;}
		else if(input == 'x') break;
		else {
			arm.writeMicroseconds(1650);
			analogWrite(armBrake, 255);
			Serial.println("Arm going up normal speed");
			delay(200);
		}
		timeInterval++;
	} while(checkInput() == true || timeInterval < 5);
}

void armDown() {
	timeInterval = 0;
	do{
		if(input == 'w') forward();
		else if(input == 's') backward();
		else if(input == 'a') left();
		else if(input == 'd') right();
		else if(input == 'q') {speedDown(); break;}
		else if(input == 'e') {speedUp(); break;}
		else if(input == 'x') break;
		else {
			arm.writeMicroseconds(1350);
			analogWrite(armBrake, 255);
			Serial.println("Arm going down normal speed");
			delay(200);
		}
		timeInterval++;
	} while(checkInput() == true || timeInterval < 5);
}

/**
 * Reads in new value while previous value is still in action
**/
bool checkInput() {
	if(Serial.available()) {
		input = Serial.read();
		timeInterval = 0;
		Serial.println("check made");
		return true;
	}
}



