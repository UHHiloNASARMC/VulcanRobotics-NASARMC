/**
 * Daryl Albano
 * 04/12/16
 * ARM and WHEELS.c
 * Vulcan Space Robotics Team
 * Main program to control Spock rover (Controls movement and bucket)
**/


#include <Servo.h>
Servo leftWheels;	// Talon: servo object for left wheels (right now, orientation is misleading)
Servo rightWheels;	// Talon: servo object for right wheels (right now, orientation is misleading)
Servo arm;	// Talon: servo object for the arm motor
int armBrake = 13;	// Pololu: provides voltage for the arm's brake
int AIN1 = 49;	// Pololu: direction for arm's brake TODO: Test if this is necessary
int AIN2 = 48;	// Pololu: direction for arm's brake TODO: Test if this is necessary
int levelLeft[3] = {1600, 1700, 1825};	// speed array to adjust speed on left side (orientation is misleading)
int levelRight[3] = {1400, 1300, 1175};	// speed array to adjust speed on right side (orientation is misleading)
int i = 0;	// Keeps track of the speed array index
int timeInterval = 0;	// Kills machine after a certain time of zero user input
int input;	// Stores the user's input
int potPin = 0;

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
		input = Serial.read();	// read in user input from serial monitor
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
		timeInterval++;	// increment timeInterval
		} while(checkInput() == true || timeInterval < 6);	// loop will exit once it reaches time limit. Loop can also continue if user inputs new character
		kill();
	}
}

/**
 * increases speed
**/
void speedUp() {if(i < 2) {i=i+1; reportSpeed();} else {Serial.println("Max speed reached"); reportSpeed(); return;}}


/**
 * decreases speed
**/
void speedDown() {if(i > 0) {i=i-1; reportSpeed();} else {Serial.println("Min speed reached"); reportSpeed(); return;}}

/**
 * kills all motors
**/
void kill() {
	Serial.println("Killed");
	leftWheels.writeMicroseconds(1500);
	rightWheels.writeMicroseconds(1500);
	arm.writeMicroseconds(1500);
	analogWrite(armBrake, 0);
	timeInterval = 0;
}

/**
 * retrieves the current speed of wheel motors
**/
void reportSpeed() {
	Serial.print("Current speed left: ");
	Serial.println(levelLeft[i]);
	Serial.print("Current speed right: ");
	Serial.println(levelRight[i]);
}

/**
 * moves the robot forward
**/
void forward() {
	leftWheels.writeMicroseconds(levelRight[i]);
	rightWheels.writeMicroseconds(levelLeft[i]);
	Serial.println("Moving forward");
	delay(500);
}

/**
 * moves the robot backwards
**/
void backward() {
	leftWheels.writeMicroseconds(levelLeft[i]);
	rightWheels.writeMicroseconds(levelRight[i]);
	Serial.println("Moving backwards");
	delay(500);
}

/**
 * robot turns left
**/
void left() {
	leftWheels.writeMicroseconds(levelLeft[i]);
	rightWheels.writeMicroseconds(levelLeft[i]);
	Serial.println("Turning left");
	delay(500);
}

/**
 * robot turns right
**/
void right() {
	leftWheels.writeMicroseconds(levelRight[i]);
	rightWheels.writeMicroseconds(levelRight[i]);
	Serial.println("Turning right");
	delay(500);
}

/**
 * lifts arm motor up TODO: implement potentiometer
**/
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
		else if(input == 'n') armDown();
		else {
			arm.writeMicroseconds(1750);
			analogWrite(armBrake, 255);
			Serial.println("Arm going up normal speed");
			delay(20);
		}
		timeInterval++;
	} while(checkInput() == true || analogRead(potPin)<300);	// loop will exit once it reaches time limit. Loop can also continue if user inputs new character
	kill();
}

/**
 * moves arm down TODO: implement potentiometer
**/
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
		else if(input == 'i') armUp();
		else {
			arm.writeMicroseconds(1350);
			analogWrite(armBrake, 255);
			Serial.println("Arm going down normal speed");
			delay(200);
		}
		timeInterval++;
	} while((checkInput() == true || timeInterval < 5) && analogRead(potPin) > 70);	// loop will exit once it reaches time limit. Loop can also continue if user inputs new character
	kill();
}

/**
 * Reads in new value while previous value is still in action
 * If no value is sent, current input will remain the same and it will not return true
 * If any character is inserted, it will return true
**/
bool checkInput() {
	if(Serial.available()) {
		input = Serial.read();
		timeInterval = 0;
		Serial.println("check made");
		return true;
	}
}



