#include <Servo.h>


const int STBY = 10;  //Standby
//const int button = 2;  //Button pin
//int buttonState = 0;
Servo arm;
Servo armBrake;


void setup()
{
  pinMode(STBY, OUTPUT);
  //pinMode(button, INPUT);
  arm.attach(9);
  armBrake.attach(8);
  arm.writeMicroseconds(1500);
  arm.writeMicroseconds(1500);
  Serial.begin(9600);
  while(!Serial);
  Serial.println("u - up halfway, i - up 3/4 way, o - up small, n - down, V - stop");
}

void loop()
{
  
  if(Serial.available())
  {
    int direction = Serial.read();
    //buttonState = digitalRead(button);
    
	//Move ARM up halfway
	if(direction == 'u')
	{
		armStop();
		armBrake.writeMicroseconds(2000);
		Serial.println("UP");
		arm.writeMicroseconds(2000);
		delay(3000);
		armStop();
	}
		
	 //Move ARM down
	 if(direction == 'n')
	 {
		armStop();
		armBrake.writeMicroseconds(1000);
		Serial.println("DOWN");
		arm.writeMicroseconds(1000);
		delay(100);
		armStop();
	 }
	 
	 //Move ARM up 3/4 way
	if(direction == 'i')
	{
		armStop();
		armBrake.writeMicroseconds(2000);
		Serial.println("UP");
		arm.writeMicroseconds(2000);
		delay(4000);
		armStop();
	}
	
	//Move ARM up in small increments
	if(direction == 'o')
	{
		armStop();
		armBrake.writeMicroseconds(2000);
		Serial.println("UP");
		arm.writeMicroseconds(2000);
		delay(500);
		armStop();
	}

	//ARM STOP
	if(direction == 'v')
	{
		armStop();
		Serial.println("STOP");
	}

	else
	{
		armStop();
	}
    
  }
}


void armStop()
{
	arm.writeMicroseconds(1500);
    	armBrake.writeMicroseconds(1500);
}
	
	
	

