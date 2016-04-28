/**
 * Daryl Albano
 * 04/12/16
 * ARM and WHEELS.c
 * Vulcan Space Robotics Team
 * Main program to control Spock rover
**/


#include <Servo.h>
Servo leftWheels;
Servo rightWheels;
Servo arm;
int PWMA = 13;
int AIN1 = 49;
int AIN2 = 48;
int levelLeft[3] = {1600, 1800, 2000};
int levelRight[3] = {1400, 1200, 1000};
int i = 0;

void setup() {
  pinMode(PWMA, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(AIN1, OUTPUT);
  leftWheels.attach(10);
  rightWheels.attach(12);
  arm.attach(8);
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
    	leftWheels.writeMicroseconds(levelLeft[i]);
    	rightWheels.writeMicroseconds(levelRight[i]);
      delay(500);
			kill();
    	leftWheels.writeMicroseconds(1500);
    	rightWheels.writeMicroseconds(1500);
    }
		
		/**
     * Move backwards
    **/
    if(input == 's') {
  		leftWheels.writeMicroseconds(levelRight[i]);
			rightWheels.writeMicroseconds(levelLeft[i]);
      delay(500);
			kill();
    	leftWheels.writeMicroseconds(1500);
    	rightWheels.writeMicroseconds(1500);
    }


    /**
     * Turn left
    **/
    if(input == 'a') {
      leftWheels.writeMicroseconds(1650);
      rightWheels.writeMicroseconds(1650);
      delay(500);
			kill();
	   	leftWheels.writeMicroseconds(1500);
    	rightWheels.writeMicroseconds(1500);
    }

    /**
     * Turn right
    **/
    if(input == 'd') {
      leftWheels.writeMicroseconds(1350);
      rightWheels.writeMicroseconds(1350);
      delay(500);
			kill();
	   	leftWheels.writeMicroseconds(1500);
    	rightWheels.writeMicroseconds(1500);
    }

    /**
     * arm up
    **/
    if(input == 'i') {
      arm.writeMicroseconds(1650);
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      analogWrite(PWMA, 255);
      delay(100);
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, LOW);
      analogWrite(PWMA, 0);
      arm.writeMicroseconds(1500);
    }

    /**
     * arm down
    **/
    if(input == 'n') {
      arm.writeMicroseconds(1350);
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
			analogWrite(PWMA, 255);
      delay(50);
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, LOW);
      analogWrite(PWMA, 0);
      arm.writeMicroseconds(1500);
    }

		/**
		 * kill
		**/
    if(input == 'x') {
      leftWheels.writeMicroseconds(1500);
      rightWheels.writeMicroseconds(1500);
    }
		/**
		 * decrease speed
		**/
		if(input == 'q') {
			speedDown();
		}
		
		/**
		 * increase speed
		**/
		if(input == 'e') {
			speedUp();		
		}
  }
}

void speedUp() {
	if(i < 3) i = i+1;
	else {
		Serial.println("Max speed reached");		
		return;
	}
}

void speedDown() {
	if(i < 3) i = i-1;
	else {
		Serial.println("Min speed reached");	
		return;
	}
}

void kill() {
	leftWheels.writeMicroseconds(1500);
	rightWheels.writeMicroseconds(1500);
}

void reportSpeed() {
	Serial.print("Current speed left: ");
	Serial.println(levelLeft[i]);
	Serial.print("Current speed right: ");
	Serial.println(levelRight[i]);
}

