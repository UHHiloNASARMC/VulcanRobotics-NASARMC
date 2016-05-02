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
/**
* Keeps track of speed
**/
int i = 0;
int timeInterval = 0;
int input;

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

    /**
     * Move forward
    **/
    if(input == 'w') {
			while(timeInterval != 6) {
				forward();
				Serial.println("Moving forward");
				timeInterval = timeInterval + 1;	
				if(readInput() == true) break;
			}
			kill();
		}
		
		/**
     * Move backwards
    **/
    if(input == 's') {
			while(timeInterval != 6) {
				Serial.println("Moving backwards");
				backward();
				timeInterval = timeInterval + 1;
				if(readInput() == true) break;
			}
			kill();
		}


    /**
     * Turn left
    **/
    if(input == 'a') {
			while(timeInterval != 6) {
				Serial.println("Turning left");
				left();
				timeInterval = timeInterval + 1;
				if(readInput() == true) break;
			}
			kill();
		}

    /**
     * Turn right
    **/
    if(input == 'd') {
			while(timeInterval != 6) {
				Serial.println("Turning right");
				right();
				timeInterval = timeInterval + 1;
				if(readInput() == true) break;
			}
			kill();
		}


	 /**
     * Move forward + turn left (work on)
    **/
    if(input == 'A') {
    	leftWheels.writeMicroseconds(levelRight[i]+150);
    	rightWheels.writeMicroseconds(levelLeft[i]);
      delay(500);
			kill();
    	leftWheels.writeMicroseconds(1500);
    	rightWheels.writeMicroseconds(1500);
    }


		/**
     * Move forward + turn left (work on)
    **/
    if(input == 'D') {
    	leftWheels.writeMicroseconds(levelRight[i]);
    	rightWheels.writeMicroseconds(levelLeft[i]-150);
      delay(500);
			kill();
    	leftWheels.writeMicroseconds(1500);
    	rightWheels.writeMicroseconds(1500);
    }

    /**
     * arm up normal (work on)
    **/
    if(input == 'i') {
      arm.writeMicroseconds(1650);
			//delay(30);      
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

		/**
		 * arm up fast (work on)
		**/
		if(input == 'p') {
			arm.writeMicroseconds(1750);
			//delay(30);
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

		/**
		 * toggle brake (Testing)
		**/
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
     * arm down normal (work on)
    **/
    if(input == 'n') {
      arm.writeMicroseconds(1350);
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
		 * kill switch
		**/
    if(input == 'x') {
      leftWheels.writeMicroseconds(1500);
      rightWheels.writeMicroseconds(1500);
    }
		/**
		 * decrease speed
		**/
		if(input == 'q') {
			Serial.println("Speed decreased");
			speedDown();
		}
		
		/**
		 * increase speed
		**/
		if(input == 'e') {
			Serial.println("Speed increased");
			speedUp();		
		}
  }
}

void speedUp() {
	if(i <= 2) {
		i = i+1;
		reportSpeed();
	}

	else {
		Serial.println("Max speed reached");		
		return;
	}
}

void speedDown() {
	if(i >= 0) {
		i = i-1;
		reportSpeed();	
	}
	else {
		Serial.println("Min speed reached");	
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
	Serial.println(levelLeft[i]);
	Serial.println("Current speed right: ");
	Serial.println(levelRight[i]);
}

void forward() {
	leftWheels.writeMicroseconds(levelRight[i]);
	rightWheels.writeMicroseconds(levelLeft[i]);
  delay(500);
}

void backward() {
	leftWheels.writeMicroseconds(levelLeft[i]);
	rightWheels.writeMicroseconds(levelRight[i]);
  delay(500);
}

void left() {
  leftWheels.writeMicroseconds(levelLeft[i]);
	rightWheels.writeMicroseconds(levelLeft[i]);
	delay(500);
}

void right() {
  leftWheels.writeMicroseconds(levelRight[i]);
  rightWheels.writeMicroseconds(levelRight[i]);
  delay(500);
}

bool readInput() {
	somewhere:
	if(Serial.available()) {
		input = Serial.read();
		return true;
	}
}



