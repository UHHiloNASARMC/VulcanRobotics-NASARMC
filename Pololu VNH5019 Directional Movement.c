/*
  Adapted form Arduino Starter Kit example
 Project 9  - Motorized Pinwheel
 
 Driving a PWM modulated bidirectional motor
 
 Created 10 February 2015
 Modified for 4 motors July 1, 2015
 by Elliot H. Sanders
 
 This example code is part of the public domain 
 */
int STBY = 10;  //Standby
int Speed;  //Current speed
int maxSpeed = 80;  //Adjust forward and backward speed
int maxTurnSpeed = 127;  //Adjust left and right turn speed

//Motor A, BOARD 1, Front Left
int PWMA = 2; //Speed control 
int AIN1 = 52; //Direction
int AIN2 = 53; //Direction
//Motor B, BOARD 1, Front Right
int PWMB = 3; // Speed Control 
int BIN1 = 50; // Direction 
int BIN2 = 51; // Direction 
//Motor C, BOARD 2, Back Left
int PWMC = 4; //Speed control 
int CIN1 = 22; //Direction
int CIN2 = 23; //Direction
//Motor D, BOARD 2, Back Right
int PWMD = 5; // Speed Control 
int DIN1 = 24; // Direction 
int DIN2 = 25; // Direction

void setup()
{
  pinMode(STBY, OUTPUT);
  // initialize the motor pins
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMC, OUTPUT);
  pinMode(CIN1, OUTPUT);
  pinMode(CIN2, OUTPUT);
  pinMode(PWMD, OUTPUT);
  pinMode(DIN1, OUTPUT);
  pinMode(DIN2, OUTPUT);
  
  Serial.begin(115200);
  while(!Serial);
  Serial.println("W -forward, A -left, S -backward, D -right, X -STOP");
}

void loop()
{
  
	  if(Serial.available())
	  {
		int direction = Serial.read();
    
		//Move Forward
		if(direction == 'w')
		{
			stop();
			for(Speed == 0; Speed < maxSpeed; Speed++)
			{
			  digitalWrite(STBY, HIGH);
			  
			  //Front Left
			  digitalWrite(AIN1, HIGH);
			  digitalWrite(AIN2, LOW);
					  //Front Right
			  digitalWrite(BIN1, HIGH);
			  digitalWrite(BIN2, LOW);
			  
			  //Back Left
			  digitalWrite(CIN1, HIGH);
			  digitalWrite(CIN2, LOW);
					  //Back Right
			  digitalWrite(DIN1, HIGH);
			  digitalWrite(DIN2, LOW);
			  
			  analogWrite(PWMA, Speed);
			  analogWrite(PWMB, Speed);
			  analogWrite(PWMC, Speed);
			  analogWrite(PWMD, Speed);
			  delay(10);
			}
		}
		
		//Move backward
		if(direction == 's')
		{
			stop();
			for(Speed == 0; Speed < maxSpeed; Speed++)
			{
			  digitalWrite(STBY, HIGH);
			  
			  //Front Left
			  digitalWrite(AIN1, LOW);
			  digitalWrite(AIN2, HIGH);
					  //Front Right
			  digitalWrite(BIN1, LOW);
			  digitalWrite(BIN2, HIGH);
			  
			  //Back Left
			  digitalWrite(CIN1, LOW);
			  digitalWrite(CIN2, HIGH);
					  //Back Right
			  digitalWrite(DIN1, LOW);
			  digitalWrite(DIN2, HIGH);
			  
			  analogWrite(PWMA, Speed);
			  analogWrite(PWMB, Speed);
			  analogWrite(PWMC, Speed);
			  analogWrite(PWMD, Speed);
			  delay(10);
			}
		}
		
		//Turn left
		if(direction == 'a')
		{
			stop();
			for(Speed == 0; Speed < maxTurnSpeed; Speed++)
			{
			  digitalWrite(STBY, HIGH);
			  
			  //Front Left
			  digitalWrite(AIN1, LOW);
			  digitalWrite(AIN2, HIGH);
					  //Front Right
			  digitalWrite(BIN1, HIGH);
			  digitalWrite(BIN2, LOW);
			  
			  //Back Left
			  digitalWrite(CIN1, LOW);
			  digitalWrite(CIN2, HIGH);
					  //Back Right
			  digitalWrite(DIN1, HIGH);
			  digitalWrite(DIN2, LOW);
			  
			  analogWrite(PWMA, Speed);
			  analogWrite(PWMB, Speed);
			  analogWrite(PWMC, Speed);
			  analogWrite(PWMD, Speed);
			  delay(10);
			}
		}
		
		//Turn right
		if(direction == 'd')
		{
			stop();
			for(Speed == 0; Speed < maxTurnSpeed; Speed++)
			{
			  digitalWrite(STBY, HIGH);
				
			  //Front Left
			  digitalWrite(AIN1, HIGH);
			  digitalWrite(AIN2, LOW);
			  //Front Right
			  digitalWrite(BIN1, LOW);
			  digitalWrite(BIN2, HIGH);
				
			  //Back Left
			  digitalWrite(CIN1, HIGH);
			  digitalWrite(CIN2, LOW);
			  //Back Right
			  digitalWrite(DIN1, LOW);
			  digitalWrite(DIN2, HIGH);
				
			  analogWrite(PWMA, Speed);
			  analogWrite(PWMB, Speed);
			  analogWrite(PWMC, Speed);
			  analogWrite(PWMD, Speed);
			  delay(10);
			}
		}
    
		//STOP
		if (direction == 'x')
		{
		  stop();
		}
	}
}


void stop(){
//enable standby  
	for(Speed == maxSpeed; Speed > 0; Speed--)
	{
	 analogWrite(PWMA, Speed);
	 analogWrite(PWMB, Speed);
	 analogWrite(PWMC, Speed);
	 analogWrite(PWMD, Speed);
	 delay(10);
	}
}
	
	
	
