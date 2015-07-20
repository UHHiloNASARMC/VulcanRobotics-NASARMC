#include <Servo.h>


const int STBY = 10;  //Standby
const int button = 2;  //Button pin
int buttonState = 0;
Servo arm;
Servo armBrake;

int armSpeed;	//Current arm speed
int forwardArmSpeed = 2000;	//Move arm in positive direction
int reverseArmSpeed = 1000;	//Move arm in negative direction


void setup()
{
  pinMode(STBY, OUTPUT);
  pinMode(button, INPUT);
  arm.attach(9);
  armBrake.attach(8);
  arm.writeMicroseconds(1500);
  arm.writeMicroseconds(1500);
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Z - up, C - down, V - stop");
}

void loop()
{
  
  if(Serial.available())
  {
    int direction = Serial.read();
    buttonState = digitalRead(button);
    if(buttonState == 0)
    {
    
	    //Move ARM up
	    if(direction == 'z')
	    {
	    	armStop();
                armBrake.writeMicroseconds(2000);
                Serial.println("UP");
	    	arm.writeMicroseconds(2000);
		delay(3000);
		armStop();
	    }
			
	     //Move ARM down
	     if(direction == 'c')
	     {
	         armStop();
                 armBrake.writeMicroseconds(1000);
                 Serial.println("DOWN");
	         arm.writeMicroseconds(1000);
		 delay(100);
		 armStop();
	     }
	
        //ARM STOP
        if(direction == 'v')
        {
  	  armStop();
          Serial.println("STOP");
        }
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
	
	
	

