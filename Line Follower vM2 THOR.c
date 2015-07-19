#pragma config(Sensor, in1,    Left,                sensorLineFollower)
#pragma config(Sensor, in2,    Middle,              sensorLineFollower)
#pragma config(Sensor, in3,    Right,               sensorLineFollower)
#pragma config(Sensor, in8,    top,                 sensorPotentiometer)
#pragma config(Sensor, dgtl1,  toplEncoder,         sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  toprEncoder,         sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  sonarCM,             sensorSONAR_cm)
#pragma config(Motor,  port1,           bottomlDrive,  tmotorNormal, openLoop, reversed)
#pragma config(Motor,  port3,           toplDrive,     tmotorNormal, openLoop, reversed)
#pragma config(Motor,  port4,           leftrArm,      tmotorNormal, openLoop, reversed)
#pragma config(Motor,  port5,           leftSwiffer,   tmotorNormal, openLoop, reversed)
#pragma config(Motor,  port6,           rightSwiffer,  tmotorNormal, openLoop, reversed)
#pragma config(Motor,  port7,           rightrArm,     tmotorNormal, openLoop, reversed)
#pragma config(Motor,  port8,           rightlArm,     tmotorNormal, openLoop)
#pragma config(Motor,  port9,           toprDrive,     tmotorNormal, openLoop)
#pragma config(Motor,  port10,          bottomrDrive,  tmotorNormal, openLoop, reversed)

int WhiteCalibrate = SensorValue(Middle);
int BlackCalibrate = SensorValue(Left);
int Threshold = (WhiteCalibrate - BlackCalibrate)/2-100;
int WhiteTop = WhiteCalibrate + Threshold;
int WhiteBottom = WhiteCalibrate - Threshold;
int BlackTop = WhiteCalibrate + Threshold;
int BlackBottom = WhiteCalibrate - Threshold;
bool FastSpeed = 70;
bool MidSpeed = 60;
bool SlowSpeed = 50;
task main()
{
  while(true)
  {
    motor[port1] = vexRT[Ch3];
    motor[port3] = vexRT[Ch3];
    motor[port9] = vexRT[Ch2];
    motor[port10] = vexRT[Ch2];
    while(vexRT[Btn8D] == 1)
    {
      //If the only middle sensor is white, go forward
      if((SensorValue(Middle) < BlackBottom) && (SensorValue(Left) > BlackBottom) && (SensorValue(Right) > BlackBottom))
      {
        motor[toprDrive] = FastSpeed;
        motor[toplDrive] = FastSpeed;
        motor[bottomrDrive] = FastSpeed;
        motor[bottomlDrive] = FastSpeed;
      }
      //If the Middle and Left sensor read White, turn left
      if((SensorValue(Left) < BlackBottom) && (SensorValue(Middle) < BlackBottom) && (SensorValue(Right) > BlackBottom))
      {
        motor[toprDrive] = FastSpeed;
        motor[bottomrDrive] = FastSpeed;
        motor[toplDrive] = MidSpeed;
        motor[toplDrive] = MidSpeed;
      }
      //If the Middle and Right reads Black and the Left reads White, turn left faster
      else if((SensorValue(Left) < BlackBottom) && (SensorValue(Middle) > BlackBottom) && (SensorValue(Right) > BlackBottom))
      {
        motor[toprDrive] = FastSpeed;
        motor[bottomrDrive] = FastSpeed;
        motor[toplDrive] = SlowSpeed;
        motor[bottomlDrive] = SlowSpeed;
      }
      //If Middle and Right sensor read White, turn right
      if ((SensorValue(Left) < BlackBottom) && (SensorValue(Middle) < BlackBottom) && (SensorValue(Right) > BlackBottom))
      {
        motor[toprDrive] = MidSpeed;
        motor[bottomrDrive] = MidSpeed;
        motor[toplDrive] = FastSpeed;
        motor[bottomlDrive] = FastSpeed;
      }
      //If the Middle and Left reads Black and the Right reads White, turn right faster
      else if ((SensorValue(Left) > BlackBottom) && (SensorValue(Middle) > BlackBottom) && (SensorValue(Right) < BlackBottom))
      {
        motor[toprDrive] = SlowSpeed;
        motor[bottomrDrive] = SlowSpeed;
        motor[toplDrive] = FastSpeed;
        motor[bottomlDrive] = FastSpeed;
      }
      //If the left, right, and center are all white
      else if ((SensorValue(Left) < BlackBottom) && (SensorValue(Middle) < BlackBottom) && (SensorValue(Right) < BlackBottom))
      {
        motor[port1] = -50;
        motor[port3] = -50;
        motor[port9] = -50;
        motor[port10] = -50;
        waitInMilliseconds(500);
        motor[port1] = 0;
        motor[port3] = 0;
        motor[port9] = 0;
        motor[port10] = 0;
        break;
      }
    }
  }
}
