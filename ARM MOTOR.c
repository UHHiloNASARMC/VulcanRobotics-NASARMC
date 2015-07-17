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

int armSpeed;	//Current arm speed
int maxArmSpeed = 255;	//Max arm speed

//Motor ARM
int PWM1 = 8;	//Speed
int PWM2 = 9;   //Speed
int DIR1 = 33;	//Direction
int DIR2 = 32;	//Direction

void setup()
{
  pinMode(STBY, OUTPUT);
  
  //Motor ARM
  pinMode(PWM1, OUTPUT);
  pinMode(PWM2, OUTPUT);
  pinMode(DIR1, OUTPUT);
  pinMode(DIR2, OUTPUT);
  
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Z - up, C - down, v - stop");
}

void loop()
{
  
  if(Serial.available())
  {
    int direction = Serial.read();
    


        //Move ARM up
        if(direction == 'z')
        {
                armStop();
                for(armSpeed == 0; armSpeed < maxArmSpeed; armSpeed++)
                {
                        digitalWrite(STBY, HIGH);

                        digitalWrite(DIR1, HIGH);
                        digitalWrite(DIR2, HIGH);
                        analogWrite(PWM1, armSpeed);
                        delay(10);
                }
        }
	
	//Move ARM down
	if(direction == 'c')
	{
		armStop();
		for(armSpeed == 0; armSpeed < maxArmSpeed; armSpeed++)
		{
			digitalWrite(STBY, HIGH);
	
			digitalWrite(DIR1, LOW);
			digitalWrite(DIR2, HIGH);
			analogWrite(PWM1, armSpeed);
			delay(10);
		}
	}

	//STOP ARM
	if(direction == 'v')
	{
		armStop();
	}
  }
}


void armStop()
{
	for(armSpeed == maxArmSpeed; armSpeed > 0; armSpeed--)
	{
		analogWrite(PWM1, armSpeed);
                digitalWrite(DIR2, LOW);
		delay(10);
	}
}
	
	
	
