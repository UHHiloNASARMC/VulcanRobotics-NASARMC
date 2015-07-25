//ARM and WHEEL.c
//Daryl Albano
//07/25/15
//includes code for both the ARM and the WHEELS

#include <Servo.h>
const int STBY = 10;	//HIGH disables STANDBY, not used much
Servo arm;	//Wheelchair motor
Servo armBrake; 	//Wheelchair brake
int Speed;  //Current speed of wheels
const int maxSpeed = 80;  //Max wheel speed of front and back direction
const int maxTurnSpeed = 127;  //Max wheel speed of left and right direction

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
	arm.attach(9);	//Wheelchair motor attached to pin 09
	armBrake.attach(8);	//Wheelchair brake attached to pin 08
	arm.writeMicroseconds(1500);	//Neutral speed of wheelchair motor
	armBrake.writeMicroseconds(1500);	//Neutral speed of wheelchair brake
	Serial.begin(115200);	//Baud rate; make sure it matches baud rate in serial monitor (CTRL + SHIFT + M)
	//Display controls
	while(!Serial);
	Serial.println("WHEEL CONTROL: W -forward, A -left, S -backward, D -right, X -STOP");
	Serial.println("\nARM CONTROL: U -up from dig to dump, I -up increment, N -down from dump to halfway, M -down increment, V -STOP");
	Serial.println();
}
void loop()
{
	if(Serial.available())
	{
		int input = Serial.read();
//ARM******************************************************************
		
		//Move up from dig to dump, enter 'u'
		if(input == 'u')	//Set to "u"
		{
			/*Ethan Note: Need to play with the duration becasuse it tends to over extend itself i.e. turns to far back and then with current program
			the poter isn't given enough power to bring it back down.*/
			
			digitalWrite(STBY, HIGH);			//Disable standby, no effect
			Serial.println("UP FROM DIG TO DUMP");
			armBrake.writeMicroseconds(1650);	//Speed control (change the value to control speed (1500-2000))
			delay(250);        					//Delay between brake and motor
			arm.writeMicroseconds(1650);		//Speed control (change the value to control speed (1500-2000))
			delay(3500);						//Runtime for motor running (change the value to control motor's runtime))
			armStop();							//Stop arm motor and brake
		}
		
		//Move up increment, no delay between brake and motor, enter 'i'
		if(input == 'i')
		{
			/*Ethan Note: 7 incrument pulses can be used to lift the digger from the bottom to dump position*/
			Serial.println("UP INCREMENT");
			armBrake.writeMicroseconds(1650);	//Speed control (change the value to control speed (1500-2000))
			arm.writeMicroseconds(1650);		//Speed control (change the value to control speed (1500-2000))
			delay(500);							//Runtime for motor running (change the value to control motor's runtime))
			armStop();							//Stop arm motor and brake	
		}
		
		//Move up from dig to halfway, enter 'o'
		if(input == 'o')
		{
			/*Ethan Note: Perfect,requires no change to timing or power*/
			Serial.println("UP FROM DIG TO HALFWAY");
			armBrake.writeMicroseconds(1650);	//Speed control (change the value to control speed (1500-2000))
			arm.writeMicroseconds(1650);		//Speed control (change the value to control speed (1500-2000))
			delay(750);							//Runtime for motor running (change the value to control motor's runtime))
			armStop();							//Stop arm motor and brake
		}
		
		//Move down from dump to halfway, enter 'n'
		if(input == 'n')
		{
			/*Ethan Notes:changed delay from 2100 to 2800 milliseconds which puts the arm just below parallel with ground fro dump position mark(in blue on digger support arm).
			Anything further than that line will require more power to the wheel chair motor. Will impliment mechanical limiter for digger arm.*/
			Serial.println("DOWN FROM DUMP TO HALFWAY");
			armBrake.writeMicroseconds(1400);	//Speed control (change the value to control speed (1500-2000))	
			arm.writeMicroseconds(1400);		//Speed control (change the value to control speed (1500-2000))
			delay(2800);						//Runtime for motor running (change the value to control motor's runtime))
			armStop();							//Stop arm motor and brake	
		}
		
		//Move down increment, enter 'm'
		if(input == 'm')
		{
			Serial.println("MOVE DOWN INCREMENT");
			armBrake.writeMicroseconds(1400);		//Speed control (change value to control speed (1500-2000))
			arm.writeMicroseconds(1400);			//Speed control (change value to control speed (1500-2000))
			delay(250);								//Runtime for motor running (change the value to control motor's runtime))
			armStop();								//Stop arm motor and brake
		} 
		
		//ATTN: Move down from halfway to ground, pulse the brakes
		//ATTN: DOWN TO DIG, move wheels forward to adjust bucket
		
		//Stop arm motor and brake, doesn't have an effect during operation, safety button IF SHIT HITS THE FAN
		if(input == 'v')
		{
			armStop();		//Stop arm motor and brake
		}
//*********************************************************************
		
//WHEEL****************************************************************
		
		//Move Forward until user enters 'v', enter 'w'
		if(input == 'w')
		{
			wheelStop();									//Stop wheels automatically
			Serial.println("FORWARD, enter 'v' to stop");
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
				//Controls speed via PWM, "Speed" corresponds to the "for loop" at top
				analogWrite(PWMA, Speed);
				analogWrite(PWMB, Speed);
				analogWrite(PWMC, Speed);
				analogWrite(PWMD, Speed);
				delay(10);
			}
		}

		//Move backward until user enters 'v', enter 's'
		if(input == 's')
		{
			wheelStop();									//Stop wheels automatically
			Serial.println("BACKWARD, enter 'v' to stop");
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
				analogWrite(PWMA, Speed);
				analogWrite(PWMB, Speed);
				analogWrite(PWMC, Speed);
				analogWrite(PWMD, Speed);
				delay(10);
			}
		}
		
		//Turn left until user enters 'v', enter 'a'
		if(input == 'a')
		{
			wheelStop();
			Serial.println("LEFT, enter 'v' to stop");
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
				analogWrite(PWMA, Speed);
				analogWrite(PWMB, Speed);
				analogWrite(PWMC, Speed);
				analogWrite(PWMD, Speed);
				delay(10);
			}
		}
		
		//Turn right until user enters 'v', enter 'd'
		if(input == 'd')
		{
			wheelStop();
			Serial.println("RIGHT, enter 'v' to stop");
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
				analogWrite(PWMA, Speed);
				analogWrite(PWMB, Speed);
				analogWrite(PWMC, Speed);
				analogWrite(PWMD, Speed);
				delay(10);
			}
		}

		//STOP, enter 'x'
		if (input == 'x')
		{
			wheelStop();
		}
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





//This is the armStop() function, stops arm, sets brake and motor to 1500 (neutral)
void armStop()
{
	Serial.println("ARM STOP");
	armBrake.writeMicroseconds(1500);
	arm.writeMicroseconds(1500);
}
