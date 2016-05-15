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
int levelArm[3] = {150, 200, 250};	// speed arry to adjust speed on arm
int i = 0;	// Keeps track of the speed array index
int j = 0;	// Keeps track of arm speed array index
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
  Serial.begin(9600);
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
			if(input == 'Q') {armSpeedUp(); break;}
			if(input == 'E') {armSpeedDown(); break;}
			if(input == 'i') {armUp(); break;}
			if(input == 'n') {armDown(); break;}
			if(input == 'j') {dump(); break;}
			if(input == 'k') {setArmDrive(); break;}
			if(input == 'l') {setArmTransport(); break;}
			if(input == '?') {printControls(); break;}

			/**
			 * Test cases
			**/
			if(input == 't') {testPot250(); break;}
			if(input == 'g') {testPot500();break;}
			if(input == 'b') {testPot750(); break;}

			timeInterval++;	// increment timeInterval
		} while(checkInput() == true || timeInterval < 6);	// loop will exit once it reaches time limit. Loop can also continue if user inputs new character
		kill();
	}
}

/**
 * increases speed
**/
void speedUp() {if(i<2) {i=i+1; reportSpeed();} else {Serial.println("Max speed reached"); reportSpeed(); return;}}


/**
 * decreases speed
**/
void speedDown() {if(i>0) {i=i-1; reportSpeed();} else {Serial.println("Min speed reached"); reportSpeed(); return;}}

/**
 * increases spped for arm
**/
void armSpeedUp() {if(j<2) {j=j+1; reportSpeed();} else{Serial.println("Max speed reached"); reportSpeed(); return;}}

/**
 * decreases speed for arm
**/
void armSpeedDown() {if(j>0) {j=j+1; reportSpeed();} else{Serial.println("Min speed reached"); reportSpeed(); return;}}

/**
 * kills all motors
**/
void kill() {
	brakeLock();
	leftWheels.writeMicroseconds(1500);
	rightWheels.writeMicroseconds(1500);
	arm.writeMicroseconds(1500);
	analogWrite(armBrake, 0);
	timeInterval = 0;
	Serial.println("Killed");
}

/**
 * retrieves the current speed of wheel motors and arm motor
**/
void reportSpeed() {
	Serial.print("Current speed left: ");
	Serial.println(levelLeft[i]);
	Serial.print("Current speed right: ");
	Serial.println(levelRight[i]);
	Serial.print("Current arm speed: ");
	Serial.println(levelArm[j]);
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
 * lifts arm motor up
**/
void armUp() {
	int maxHeight = analogRead(potPin) + 100;
	do{
		if(readInput() == 1) continue;
		else if(readInput() == 2) break;
		else {
			arm.writeMicroseconds(1500 + levelArm[j]);
			brakeUnlock();
			Serial.println("Arm going up normal speed");
			delay(200);
		}
	} while(analogRead(potPin) < maxHeight);	// loop will exit once it reaches time limit. Loop can also continue if user inputs new character
	kill();
}

/**
 * moves arm down by static number
**/
void armDown() {
	int minHeight = analogRead(potPin) - 100;
	do{
		if(readInput() == true) continue;
		else if(readInput() == 2) break;
		else {
			arm.writeMicroseconds(1500 - levelArm[j]);
			analogWrite(armBrake, 255);
			brakeUnlock();
			Serial.println("Arm going down normal speed");
			delay(200);
		}
	} while(analogRead(potPin) > minHeight);	// loop will exit once it reaches time limit. Loop can also continue if user inputs new character
	kill();
}

/**
 * Sets arm to dump position
**/
void dump() {
	do{
		if(readInput() == true) continue;
		else if(readInput() == 2) break;
		else {
			if(analogRead(potPin) < 860) {
				arm.writeMicroseconds(1500 + levelArm[j]);
				brakeUnlock();
				Serial.println("Setting arm to dump position");
				delay(200);
			}
			else {
				arm.writeMicroseconds(1500 - levelArm[j]);
				brakeUnlock();
				Serial.println("Setting arm to dump position");
				delay(200);
			}
		}
	} while(analogRead(potPin) != 860);	// loop will exit once it reaches time limit. Loop can also continue if user inputs new character
	kill();
}

/**
 * Sets arm to drive position
**/
void setArmDrive() {
	do{
		if(readInput() == true) continue;
		else if(readInput() == 2) break;
		else {
			if(analogRead(potPin) > 40) {
				arm.writeMicroseconds(1500 - levelArm[j]);
				brakeUnlock();
				Serial.println("Setting arm to drive position");
				delay(200);
			}
			else {
				arm.writeMicroseconds(1500 + levelArm[j]);
				brakeUnlock();
				Serial.println("Setting arm to drive position");
				delay(200);
			}
		}
	} while(analogRead(potPin) != 40);	// loop will exit once it reaches time limit. Loop can also continue if user inputs new character
	kill();
}

void setArmTransport() {
	do{
		if(readInput() == true) continue;
		else if(readInput() == 2) break;
		else {
			if(analogRead(potPin) > 420) {
				arm.writeMicroseconds(1500 - levelArm[j]);
				brakeUnlock();
				Serial.println("Setting arm to transport position");
				delay(200);
			}
			else {
				arm.writeMicroseconds(1500 + levelArm[j]);
				brakeUnlock();
				Serial.println("Setting arm to transport position");
				delay(200);
			}
		}
	} while(analogRead(potPin) != 420);
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

/**
 * Unlocks brake for the wheelchair motor
 **/
void brakeUnlock() {
	digitalWrite(AIN1, HIGH);
	digitalWrite(AIN2, LOW);
	analogWrite(armBrake, 255);
}

/**
 * Locks brake for the wheelchair motor
 **/
void brakeLock() {
	digitalWrite(AIN1, LOW);
	digitalWrite(AIN2, LOW);
	analogWrite(armBrake, 0);
}

/**
 * For the novice user, this function prints out the available controls
**/
void printControls() {
	Serial.println("w - Forward");
	Serial.println("a - Left");
	Serial.println("s - Backwards");
	Serial.println("d - Right");
	Serial.println("i - Raise arm up");
	Serial.println("n - Lower arm down");
	Serial.println("j - Set arm to dump position");
	Serial.println("k - Set arm to drive position");
	Serial.println("x - KILL EVERYTHING!!!");
	Serial.println("q - decrease wheel speed");
	Serial.println("e - increase wheel speed");
	Serial.println("Q - decrease arm speed");
	Serial.println("E - increase arm speed");
}

int readInput() {
	if(Serial.available()) {
		input = Serial.read();
		if(input == 'w') forward();
		else if(input == 's') backward();
		else if(input == 'a') left();
		else if(input == 'd') right();
		else if(input == 'x') {kill(); return 2;}
		else if(input == 'q') {speedDown(); return 0;}
		else if(input == 'e') {speedUp(); return 0;}
		else if(input == 'Q') {armSpeedUp(); return 0;}
		else if(input == 'E') {armSpeedDown(); return 0;}
		else if(input == '?') {printControls(); return 2;}
		else return 0;
	}
	return 1;
}




/**
 * Test cases
 **/
void testBrake() {
	digitalWrite(AIN1, HIGH);
	digitalWrite(AIN2, LOW);
	analogWrite(armBrake, 255);
	delay(2000);
	digitalWrite(AIN1, LOW);
	digitalWrite(AIN2, LOW);
	analogWrite(armBrake, 0);
	
}

void testPot750() {
	Serial.println("pot750");
	while(analogRead(potPin) < 750) brakeUnlock();
	kill();
}

void testPot250() {
	Serial.println("pot250");
	while(analogRead(potPin) < 250) brakeUnlock();
	kill();
}

void testPot500() {
	Serial.println("pot500");
	while(analogRead(potPin) < 500) brakeUnlock();
	kill();
}



