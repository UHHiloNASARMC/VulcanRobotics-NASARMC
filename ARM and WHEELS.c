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
int PWMA = 13;
int AIN1 = 49;
int AIN2 = 48;
int levelLeft[3] = {1600, 1700, 1800};
int levelRight[3] = {1400, 1300, 1200};
/**
* Keeps track of speed
**/
int i = 0;
int timeInterval = 0;
int input;
bool checkAvail;

void setup() {
  pinMode(PWMA, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(AIN1, OUTPUT);
  leftWheels.attach(10);
  rightWheels.attach(12);
  arm.attach(8);
  leftWheels.writeMicroseconds(1500);
  rightWheels.writeMicroseconds(1500);
  Serial.begin(57600);
}

void loop() {
	if(Serial.available()) {
		input = Serial.read();
		do {
		if(input == 'w') forward();
		else if(input == 's') backward();
		else if(input == 'a') left();
		else if(input == 'd') right();
		} while(checkInput() == true);
	}
}

void speedUp() {
	if(i < 2) {
		i = i+1;
		reportSpeed();
	}

	else {
		Serial.println("Max speed reached");
		reportSpeed();
		return;
	}
}

void speedDown() {
	if(i > 0) {
		i = i-1;
		reportSpeed();	
	}
	else {
		Serial.println("Min speed reached");
		reportSpeed();
		return;
	}
}

void kill() {
	Serial.println("Killed");
	leftWheels.writeMicroseconds(1500);
	rightWheels.writeMicroseconds(1500);
	timeInterval = 0;
}

void reportSpeed() {
	Serial.println("Current speed left: ");
	Serial.println(i);
	Serial.println(levelLeft[i]);
	Serial.println("Current speed right: ");
	Serial.println(levelRight[i]);
}

/**
 * Since the following functions are similar, consider passing speed + direction as parameters
**/
void forward() {
	while(timeInterval != 6) {
		leftWheels.writeMicroseconds(levelRight[i]);
		rightWheels.writeMicroseconds(levelLeft[i]);
		Serial.println("Moving forward");
		timeInterval = timeInterval + 1;	
		if(checkInput() == true) {
			kill();
			return;
		}
		delay(500);
	}
	kill();
}

void backward() {
	while(timeInterval != 6) {
		leftWheels.writeMicroseconds(levelLeft[i]);
		rightWheels.writeMicroseconds(levelRight[i]);
		Serial.println("Moving backwards");
		timeInterval = timeInterval + 1;
		if(checkInput() == true) {
			kill();
			return;
		}
		delay(500);
	}
	kill();
}

void left() {
	while(timeInterval != 6) {
		leftWheels.writeMicroseconds(levelLeft[i]);
		rightWheels.writeMicroseconds(levelLeft[i]);
		Serial.println("Turning left");
		timeInterval = timeInterval + 1;
		if(checkInput() == true) {
			kill();
			return;
		}	
		delay(500);
	}
	kill();
}

void right() {
	while(timeInterval != 6) {
  	leftWheels.writeMicroseconds(levelRight[i]);
  	rightWheels.writeMicroseconds(levelRight[i]);
		Serial.println("Turning right");
		timeInterval = timeInterval + 1;
		if(checkInput() == true) {
			kill();
			return;
		} 
		delay(500);
	}
	kill();
}

bool checkInput() {
	if(Serial.available()) {
		input = Serial.read();
		Serial.println("check made");
		if(input == '\n') {return false; checkAvail = false;}
		else {checkAvail = true; return true;}
	}
	else return false;
}



