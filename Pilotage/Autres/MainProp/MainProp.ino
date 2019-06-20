/*
Programme de test Proportion 
Pascal DENICOU
*/

//***************************************************************************************************************************************
// M A I N 
//***************************************************************************************************************************************

int MotorPositionAmplitude = 254;
int index = 0;
int angle = 0;

//***************************************************************************************************************************************
void setup() 
//***************************************************************************************************************************************
{
  Serial.begin(9600);
  while (!Serial) {
     // wait for serial port to connect. Needed for native USB port only
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------
void ProportionalModule(  int enginePower,
                          int turningAngle,
                          int proportionalMode,
                          int* leftEnginePower,
                          int* rightEnginePower)
//---------------------------------------------------------------------------------------------------------------------------------------
{
  // enginePower is % from 0 to 100

  *leftEnginePower = enginePower;
  *rightEnginePower = enginePower;

  if (enginePower != 0)
  {
    *leftEnginePower  = enginePower + (turningAngle * proportionalMode * 2 / enginePower);
    *rightEnginePower = enginePower - (turningAngle * proportionalMode * 2 / enginePower);
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------
int PropulsionLeftEngineStop()
//---------------------------------------------------------------------------------------------------------------------------------------
{
  Serial.println("leftEngineStop");
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
int PropulsionRightEngineStop()
//---------------------------------------------------------------------------------------------------------------------------------------
{
  Serial.println("rightEngineStop");
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
int PropulsionRightEngineTurnLeft(int value)
//---------------------------------------------------------------------------------------------------------------------------------------
{
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
int PropulsionLeftEngineTurnRight(int value)
//---------------------------------------------------------------------------------------------------------------------------------------
{
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
int PropulsionLeftEngineTurnLeft(int value)
//---------------------------------------------------------------------------------------------------------------------------------------
{
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
int PropulsionRightEngineTurnRight(int value)
//---------------------------------------------------------------------------------------------------------------------------------------
{
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
void Ok()
//---------------------------------------------------------------------------------------------------------------------------------------
{
  //Serial.println("Ok");
}

//---------------------------------------------------------------------------------------------------------------------------------------
void ErrorValueOutOfRange()
//---------------------------------------------------------------------------------------------------------------------------------------
{
  //Serial.println("ErrorValueOutOfRange");
}

//---------------------------------------------------------------------------------------------------------------------------------------
void ApplyProportionalPower(int leftEnginePower,
                            int rightEnginePower)
//---------------------------------------------------------------------------------------------------------------------------------------
{
  bool noErrorRight;
  bool noErrorLeft;
  // LEFT ENGINE
  if (leftEnginePower > 0)
  {
    noErrorLeft = (PropulsionLeftEngineTurnLeft((leftEnginePower * MotorPositionAmplitude) / 100) == 0);
  }
  else
    if (leftEnginePower < 0)
    {
      noErrorLeft = (PropulsionLeftEngineTurnRight((leftEnginePower * MotorPositionAmplitude) / 100) == 0);
    }
    else
    {
      noErrorLeft = (PropulsionLeftEngineStop() == 0);
    }

  // RIGHT ENGINE
  if (rightEnginePower > 0)
  {
    noErrorRight = (PropulsionRightEngineTurnRight((rightEnginePower * MotorPositionAmplitude) / 100) == 0);
  }
  else
    if (rightEnginePower < 0)
    {
      noErrorRight = (PropulsionRightEngineTurnLeft((rightEnginePower * MotorPositionAmplitude) / 100) == 0);
    }
    else
    {
      noErrorRight = (PropulsionRightEngineStop() == 0);
    }

  if (noErrorRight || noErrorLeft)
  {
    Ok();
  }
  else ErrorValueOutOfRange();
}

//---------------------------------------------------------------------------------------------------------------------------------------
void ForwardEngine(int enginePower)
//---------------------------------------------------------------------------------------------------------------------------------------
{
  // enginePower is % from 0 to 100
  // MotorPositionAmplitude is acceleration amplitude (ex:254 = 100%)

  bool noErrorRight;
  bool noErrorLeft;
  int leftEnginePower;
  int rightEnginePower;

  ProportionalModule( enginePower,
                      angle, //_turningAngle,
                      index,  //_proportionalMode,
                      &leftEnginePower,
                      &rightEnginePower);
  
  Serial.print("leftEnginePower ");
  Serial.println(leftEnginePower);
  Serial.print("rightEnginePower ");
  Serial.println(rightEnginePower);
  ApplyProportionalPower(leftEnginePower, rightEnginePower);

  //_enginePower = enginePower;
}

//***************************************************************************************************************************************
void loop()
{
  for (angle = 0;angle < 90;angle+=5)
  {
    for (index = 0; index < 6;index++)
    {
      Serial.print("Angle = ");
      Serial.println(angle);
      Serial.print("Mode = ");
      Serial.println(index);

      Serial.println("ForwardEngine(100)");
      ForwardEngine(100);
      delay(200); 
 
      Serial.println("ForwardEngine(30)");
      ForwardEngine(30);
      delay(200);
    }
  }  
}
//---------------------------------------------------------------------------------------------------------------------------------------

