//ARM and WHEEL.c
//Daryl Albano
//07/25/15
//includes code for both the ARM and the WHEELS

#include <Servo.h>
const int STBY = 10;			//HIGH disables STANDBY, not used much
Servo arm;						//Servo object for wheelchair arm
Servo armBrake; 				//Servo object for wheelchair brake
int Speed;  					//Current speed of wheels
const int maxSpeed = 80;  		//Max wheel speed of front and back direction
const int maxTurnSpeed = 127;	//Max wheel speed of left and right direction

//Motor A, BOARD 1, Front Left
int PWMA = 2;					//Speed control, connect to pin 2
int AIN1 = 52;					//Direction, connect to pin 52
int AIN2 = 53;					//Direction, connect to pin 53

//Motor B, BOARD 1, Front Right
int PWMB = 3; 					//Speed Control, connect to pin 3
int BIN1 = 50; 					//Direction, connect to pin 50
int BIN2 = 51; 					//Direction, connect to pin 51

//Motor C, BOARD 2, Back Left
int PWMC = 4;					//Speed control, connect to pin 4
int CIN1 = 22;					//Direction, connect to pin 22
int CIN2 = 23;					//Direction, connect to pin 23

//Motor D, BOARD 2, Back Right
int PWMD = 5;					//Speed Control, connect to pin 5
int DIN1 = 24;					//Direction, connect to pin 24
int DIN2 = 25; 					//Direction, connect to pin 25

void setup()
{
	pinMode(STBY, OUTPUT);
	
	//Initialize pins for WHEELS
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

	//Initialize pins for ARM
	arm.attach(9);						//Wheelchair motor attached to pin 09
	armBrake.attach(8);					//Wheelchair brake attached to pin 08
	arm.writeMicroseconds(1500);		//Neutral speed of wheelchair motor
	armBrake.writeMicroseconds(1500);	//Neutral speed of wheelchair brake

	Serial.begin(115200);				//Baud rate; make sure it matches baud rate in serial monitor (CTRL + SHIFT + M)
	
	//Displays controls
	while(!Serial);
	Serial.println("WHEEL CONTROLS: W -forward, A -left, S -backwards, D -right, X -STOP");
	Serial.println("\nARM CONTROLS UP: I -up increment 1650, U -up increment 2000");
	Serial.println("\nARM CONTROLS DOWN: M -down increment FAST, N -down increment SLOW");
	Serial.println();
}
void loop()
{
	if(Serial.available())
	{
		int input = Serial.read();				//Reads in user input
//ARM******************************************************************
		
		//MOVE UP increment 1650, no delay between brake and motor, enter 'i'
		if(input == 'i')
		{
			Serial.println("UP INCREMENT 1650");
			armBrake.writeMicroseconds(1650);	//Speed control (change the parameter to control speed (1500-2000))
			arm.writeMicroseconds(1650);		//Speed control (change the parameter to control speed (1500-2000))
			delay(500);							//Runtime for motor running (change the parameter to control motor's runtime))
			armStop();							//Stop arm motor and brake	
		}
		

		//MOVE UP increment 2000, no delay between brake and motor, enter 'u'
		if(input == 'u')
		{
			Serial.println("UP INCREMENT 2000");
			armBrake.writeMicroseconds(2000);	//Speed control (change the parameter to control speed (1500-2000))
			arm.writeMicroseconds(2000);		//Speed control (change the parameter to control speed (1500-2000))
			delay(500);							//Runtime for motor running (change the parameter to control motor's runtime))
			armStop();							//Stop arm motor and brake
		}

		//MOVE DOWN increment FAST, enter 'm'
		if(input == 'm')
		{
			Serial.println("DOWN INCREMENT FAST");
			armBrake.writeMicroseconds(1300);		//Speed control (change the parameter to control speed (1000-1500))
			arm.writeMicroseconds(1300);			//Speed control (change the parameter to control speed (1000-1500))
			delay(250);								//Runtime for motor running (change the parameter to control motor's runtime))
			armStop();								//Stop arm motor and brake
		} 

		//MOVE DOWN increment SLOW, enter 'n'
		if(input == 'n')
		{
			Serial.println("DOWN INCREMENT SLOW");
			armBrake.writeMicroseconds(1400);		//Speed control (change the parameter to control speed(1000-1500))
			arm.writeMicroseconds(1400);			//Speed control (change the parameter to control speed(1000-1500))
			delay(250);								//Runtime for motor running (change the parameter to control motot's runtime))
			armStop();								//Stop arm motor and brake
		}
		
		//Stop arm motor and brake, doesn't have an effect during operation, safety button IF SHIT HITS THE FAN
		if(input == 'v') armStop();		//Stop arm motor and brake

//*********************************************************************
		
//WHEEL****************************************************************
		
		//MOVE FORWARD: enter 'w', STOP: enter 'x'
		if(input == 'w')
		{
			wheelStop();									//Stop wheels automatically
			Serial.println("FORWARD, enter 'x' to stop");
			for(Speed == 0; Speed < maxSpeed; Speed++)		//Slowly accelerate from 0 to maxSpeed
			{
				digitalWrite(STBY, HIGH);					//disables standby	
				//Back Left
				digitalWrite(AIN1, HIGH);
				digitalWrite(AIN2, LOW);
				//Front Left
				digitalWrite(BIN1, HIGH);
				digitalWrite(BIN2, LOW);
				//Front Right
				digitalWrite(CIN1, LOW);
				digitalWrite(CIN2, HIGH);
				//Back Right
				digitalWrite(DIN1, LOW);
				digitalWrite(DIN2, HIGH);
				//Speed
				analogWrite(PWMA, Speed);
				analogWrite(PWMB, Speed);
				analogWrite(PWMC, Speed);
				analogWrite(PWMD, Speed);
				delay(10);
			}
		}

		//MOVE BACKWARD: enter 's', STOP: enter 'x'
		if(input == 's')
		{
			wheelStop();									//Stop wheels automatically
			Serial.println("BACKWARD, enter 'x' to stop");
			for(Speed == 0; Speed < maxSpeed; Speed++)		//Slowly accelerates from 0 to max speed	
			{
				digitalWrite(STBY, HIGH);
				//Back Left
				digitalWrite(AIN1, LOW);
				digitalWrite(AIN2, HIGH);
				//Front Left
				digitalWrite(BIN1, LOW);
				digitalWrite(BIN2, HIGH);
				//Front Right
				digitalWrite(CIN1, HIGH);
				digitalWrite(CIN2, LOW);
				//Back Right
				digitalWrite(DIN1, HIGH);
				digitalWrite(DIN2, LOW);
				//Speed
				analogWrite(PWMA, Speed);
				analogWrite(PWMB, Speed);
				analogWrite(PWMC, Speed);
				analogWrite(PWMD, Speed);
				delay(10);
			}
		}
		
		//TURN LEFT: enter 'a', STOP: enter 'z'
		if(input == 'a')
		{
			wheelStop();
			Serial.println("LEFT, enter 'z' to stop");
			for(Speed == 0; Speed < maxTurnSpeed; Speed++)
			{
				digitalWrite(STBY, HIGH);
				//Back Left
				digitalWrite(AIN1, LOW);
				digitalWrite(AIN2, HIGH);
				//Front Left
				digitalWrite(BIN1, LOW);
				digitalWrite(BIN2, HIGH);
				//Front Right
				digitalWrite(CIN1, LOW);
				digitalWrite(CIN2, HIGH);
				//Back Right
				digitalWrite(DIN1, LOW);
				digitalWrite(DIN2, HIGH);
				//Speed
				analogWrite(PWMA, Speed);
				analogWrite(PWMB, Speed);
				analogWrite(PWMC, Speed);
				analogWrite(PWMD, Speed);
				delay(10);
			}
		}
		
		//TURN RIGHT: enter 'd', STOP: enter 'z'
		if(input == 'd')
		{
			wheelStop();
			Serial.println("RIGHT, enter 'z' to stop");
			for(Speed == 0; Speed < maxTurnSpeed; Speed++)
			{
				digitalWrite(STBY, HIGH);
				//Back Left
				digitalWrite(AIN1, HIGH);
				digitalWrite(AIN2, LOW);
				//Front Left
				digitalWrite(BIN1, HIGH);
				digitalWrite(BIN2, LOW);
				//Front Right
				digitalWrite(CIN1, HIGH);
				digitalWrite(CIN2, LOW);
				//Back Right
				digitalWrite(DIN1, HIGH);
				digitalWrite(DIN2, LOW);
				//Speed
				analogWrite(PWMA, Speed);
				analogWrite(PWMB, Speed);
				analogWrite(PWMC, Speed);
				analogWrite(PWMD, Speed);
				delay(10);
			}
		}

		//STOPS WHEELS, enter 'x'
		if (input == 'x') wheelStop();

		//STOPS TURN WHEELS, enter 'z'
		if(input == 'z') wheelTurnStop();
	}
	//*********************************************************************
}

//This is the wheelStop() function, slowly deccelerates Speed from maxSpeed to 0
void wheelStop()
{
	Serial.println("WHEELS STOP");
	for(Speed == maxSpeed; Speed > 0; Speed--)
	{
		analogWrite(PWMA, Speed);
		analogWrite(PWMB, Speed);
		analogWrite(PWMC, Speed);
		analogWrite(PWMD, Speed);
		delay(10);
	}
}

//This is the wheelTurnStop() function, slowly deccelerates Speed from maxTurnSpeed to 0
void wheelTurnStop()
{
	Serial.println("WHEELS TURN STOP");
	for(Speed == maxTurnSpeed; Speed > 0; Speed--)
	{
		analogWrite(PWMA, Speed);
		analogWrite(PWMB, Speed);
		analogWrite(PWMC, Speed);
		analogWrite(PWMD, Speed);
		delay(10);
	}
}

//This is the armStop() function, stops arm, sets brake and motor to 1500 (neutral)
void armStop()
{
	Serial.println("ARM STOP");
	armBrake.writeMicroseconds(1500);
	arm.writeMicroseconds(1500);
}
