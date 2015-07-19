#include <Servo.h> 

Servo myservo;

void setup() 
{ 
  Serial.begin(9600);
  myservo.attach(9);
  myservo.writeMicroseconds(1500);  // set motor to neutral
} 

void loop() 
{
  if (Serial.available() > 0)
  {
    int inByte = Serial.read();
    // do something different depending on the character received.  
    // The switch statement expects single number values for each case;
    // in this exmaple, though, you're using single quotes to tell
    // the controller to get the ASCII value for the character.  For 
    // example 'a' = 97, 'b' = 98, and so forth:

    switch (inByte) 
    {
      case 'f':
      Serial.print("Moving Forward");
      Serial.print('\n');
      myservo.writeMicroseconds(2000);  // set motor to forward
      delay(1000);
      Serial.print("Stopping");
      Serial.print('\n');
      myservo.writeMicroseconds(1500);  // set motor to neutral
      break;
      case 'r':
      Serial.print("Moving Reverse");
      Serial.print('\n');
      myservo.writeMicroseconds(1000);  // set motor to reverse
      delay(1000);
      Serial.print("Stopping");
      Serial.print('\n');
      myservo.writeMicroseconds(1500);  // set motor to neutral
      break;
    }
  }
} 
