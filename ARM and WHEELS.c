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
int levelLeft[3] = {1650, 1800, 1825};
int levelRight[3] = {1450, 1200, 1125};
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
  Serial.begin(57600);
}

void loop() {
  if(Serial.available()) {
		//Serial.println("Enter a command: ");
    int input = Serial.read();
    
    /**
     * Move forward
    **/
    if(input == 'w') {
			//Serial.println("Moving forward...");
    	leftWheels.writeMicroseconds(levelRight[i]);
    	rightWheels.writeMicroseconds(levelLeft[i]);
      delay(500);
			kill();
    	leftWheels.writeMicroseconds(1500);
    	rightWheels.writeMicroseconds(1500);
    }
		
		/**
     * Move backwards
    **/
    if(input == 's') {
  		leftWheels.writeMicroseconds(levelLeft[i]);
			rightWheels.writeMicroseconds(levelRight[i]);
      delay(500);
			kill();
    	leftWheels.writeMicroseconds(1500);
    	rightWheels.writeMicroseconds(1500);
    }


    /**
     * Turn left
    **/
    if(input == 'a') {
      leftWheels.writeMicroseconds(levelLeft[i]);
      rightWheels.writeMicroseconds(levelLeft[i]);
      delay(500);
			kill();
	   	leftWheels.writeMicroseconds(1500);
    	rightWheels.writeMicroseconds(1500);
    }

    /**
     * Turn right
    **/
    if(input == 'd') {
      leftWheels.writeMicroseconds(levelRight[i]);
      rightWheels.writeMicroseconds(levelRight[i]);
      delay(500);
			kill();
	   	leftWheels.writeMicroseconds(1500);
    	rightWheels.writeMicroseconds(1500);
    }


	 /**
     * Move forward + turn left
    **/
    if(input == 'A') {
			//Serial.println("Moving forward...");
    	leftWheels.writeMicroseconds(levelRight[i]+150);
    	rightWheels.writeMicroseconds(levelLeft[i]);
      delay(500);
			kill();
    	leftWheels.writeMicroseconds(1500);
    	rightWheels.writeMicroseconds(1500);
    }


		/**
     * Move forward + turn left
    **/
    if(input == 'D') {
			//Serial.println("Moving forward...");
    	leftWheels.writeMicroseconds(levelRight[i]);
    	rightWheels.writeMicroseconds(levelLeft[i]-150);
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
			
			//brake
			delay(30);      
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
			analogWrite(PWMA, 255);

      delay(200);

      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, LOW);
      analogWrite(PWMA, 0);
			//delay(50);
      arm.writeMicroseconds(1500);
    }

		if(input == 'p') {
			arm.writeMicroseconds(1750);
			
			delay(30);
			digitalWrite(AIN1, HIGH);
			digitalWrite(AIN2, LOW);
			analogWrite(PWMA, 255);

			delay(200);

			digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, LOW);
      analogWrite(PWMA, 0);
			//delay(30);
      arm.writeMicroseconds(1500);								
		}

		//toggle brake
		if(input == 'b') {
			digitalWrite(AIN1, HIGH);
			digitalWrite(AIN2, LOW);
			analogWrite(PWMA, 255);
			delay(100);
			digitalWrite(AIN1, LOW);
			digitalWrite(AIN2, LOW);
			analogWrite(PWMA, 0);
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
	reportSpeed();
	if(i <= 2) i = i+1;
	else {
		Serial.println("Max speed reached");		
		return;
	}
}

void speedDown() {
	reportSpeed();
	if(i >= 0) i = i-1;
	else {
		Serial.println("Min speed reached");	
		return;
	}
}

void kill() {
	leftWheels.writeMicroseconds(1500);
	rightWheels.writeMicroseconds(1500);
	//Serial.println("Killed");
}

void reportSpeed() {
	Serial.println("Current speed left: ");
	Serial.println(levelLeft[i]);
	Serial.println("Current speed right: ");
	Serial.println(levelRight[i]);
}

