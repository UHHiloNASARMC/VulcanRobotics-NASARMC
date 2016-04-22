/**
 * Daryl Albano
 * 04/12/16
 * ARM and WHEELS.c
 * Vulcan Space Robotics Team
 * Main program to control Spock rover
**/


#include<Servo.h>
Servo leftWheels;
Servo rightWheels;
Servo arm;
int PWMA = 13;
int AIN1 = 49;
int AIN2 = 48;

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
      leftWheels.writeMicroseconds(1350);
      rightWheels.writeMicroseconds(1650);
      delay(1000);
      leftWheels.writeMicroseconds(1500);
      rightWheels.writeMicroseconds(1500);
    }

    /**
     * Turn left
    **/
    if(input == 'a') {
      leftWheels.writeMicroseconds(1650);
      rightWheels.writeMicroseconds(1650);
      delay(1000);
      leftWheels.writeMicroseconds(1500);
      rightWheels.writeMicroseconds(1500);
    }

    /**
     * Move backwards
    **/
    if(input == 's') {
      leftWheels.writeMicroseconds(1650);
      rightWheels.writeMicroseconds(1350);
      delay(1000);
      leftWheels.writeMicroseconds(1500);
      rightWheels.writeMicroseconds(1500);
    }

    /**
     * Turn right
    **/
    if(input == 'd') {
      leftWheels.writeMicroseconds(1350);
      rightWheels.writeMicroseconds(1350);
      delay(1000);
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
      delay(50);
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, LOW);
      analogWrite(PWMA, 0);
      arm.writeMicroseconds(1500);
    }

    if(input == 'x') {
      leftWheels.writeMicroseconds(1500);
      rightWheels.writeMicroseconds(1500);
    }
  }
}



