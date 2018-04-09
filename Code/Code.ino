// BRice & CHubbell

//=================================================================
//     Declare Global Variables & Header Files
//=================================================================
// Including Arduino's I2C library, as well as the ultrasonic,
// motorshield, and LCD libraries.
#include <Wire.h>
#include <NewPing.h>
#include <Adafruit_MotorShield.h>
#include <LiquidCrystal.h>

// Constants are defined for the physical elements connecting to the Arduino
const int PUSH_BUTTONS = A0;
const int HEAT_SENSOR = A1;
const int LIGHT_SENSOR = A2;

const int LEFT_SENSOR = 8;
const int FRONT_SENSOR = 9;
const int RIGHT_SENSOR = 10;
const int SPEAKER = 11;
const int LEFT_LIMIT = 12;
const int RIGHT_LIMIT = 13;

// Constants used numerically in the algorithm
const int MAX_DISTANCE = 1000;
const int AMOUNT_OF_PULSES = 12;

// declare states
int INPUT_STATE = 1;
int MAZE_STATE = 2;
int ERROR_STATE = 3;
int HOUSE_STATE = 4;
int LIGHT_STATE = 5;
int RHOUSE_STATE = 6;
int RMAZE_STATE = 7;
int VICTORY_STATE = 8;

// declare variables
int _currentState;

int frontReading = 0;
int rightReading = 0;
int leftReading = 0;

int buttonVal = 0;
int heatVal = 0;
int lightVal = 0;
int Lsensor = 0;
int Tsensor = 0;
int LSpeed = 132;
int RSpeed = 120;

int location;
int currentDistance;
int left;
int right;

boolean button1;
boolean button2;
boolean button3;
boolean button4;

boolean Lwall;
boolean Fwall;
boolean Rwall;

boolean headedHome = false;

int movesSoFar = 0;
const int numOfMoves = 20;
int moves[numOfMoves];

unsigned long refreshTime = 0;
unsigned long stateTime = 0;

// Create the motor shield object with the default I2C address
Adafruit_MotorShield motorShield = Adafruit_MotorShield();

// Select which 'port' for each motor.
Adafruit_DCMotor *leftMotor = motorShield.getMotor(3);
Adafruit_DCMotor *rightMotor = motorShield.getMotor(4);

// Create an object for each ultrasonic sensor to be used
NewPing sonarFront = NewPing(FRONT_SENSOR, FRONT_SENSOR, MAX_DISTANCE);
NewPing sonarLeft = NewPing(LEFT_SENSOR, LEFT_SENSOR, MAX_DISTANCE);
NewPing sonarRight = NewPing(RIGHT_SENSOR, RIGHT_SENSOR, MAX_DISTANCE);

// Initialize the LCD with the numbers of the interface pins
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);


void setup() 
{
  // Make noise
  analogWrite(SPEAKER, 20);
  
  // vary the light and temp parameters based on the sensor
  while(millis() < 3000)
  {
    lightVal = analogRead(LIGHT_SENSOR);
    heatVal = analogRead(HEAT_SENSOR);
    
    // asigns relative to room light
    if(lightVal > Lsensor)
    {
      Lsensor = lightVal;
    }
    
    // assigns relative to rooms temperature
    if(heatVal > Tsensor)
    
      Tsensor = heatVal;
    }
 

  
  // The motor shield and the serial monitor are initialized
  motorShield.begin();
  Serial.begin(9600);

  // Motors started.
  leftMotor->run(RELEASE);
  rightMotor->run(RELEASE);

  // set up the number of columns and rows on the LCD
  lcd.begin(16, 2);

  // fill the array
  for (int i = 0; i <numOfMoves; i++)
  {
    moves[i] = 0;
  }

  // quits noise
  analogWrite(SPEAKER, 0);
  


  // go to first state
  _currentState = INPUT_STATE;
}

// the loop routine runs over and over again forever:
void loop() 
{
  // Every quarter second clear the LCD and print to the serial monitor.
  if ((millis() - refreshTime > 250))
  {
    refreshTime = millis();
    // Prints the current state to the serial monitor. States are printed 
    // to the screen for troubleshooting.
    //Serial.println(currentState);
    // Clears the LCD screen.
    lcd.clear();
  }
  
  while (_currentState == INPUT_STATE)
  {
    // show state
    lcd.setCursor(0, 0);
    lcd.print("Input:");

    // read buttons
    while(button4 == false && button3 == false && button2 == false && button1 == false)
    {
      // re read
      buttonVal = analogRead(PUSH_BUTTONS);
      Serial.println(buttonVal);
    
      if (buttonVal >= 1020)
      {
        button4 = true;
      }
      else if (buttonVal >= 960 && buttonVal <= 1010)
      {
        button3 = true;
      }
      else if (buttonVal >= 500 && buttonVal <= 920)
      {
        button2 = true;
      }
      else if (buttonVal >= 9 && buttonVal <= 20)
      {
        button1 = true;
      }
    }

    if(button1 == true)
    {
      moves[movesSoFar] = 1;
      movesSoFar ++;
    }
    
    else if(button2 == true)
    {
      moves[movesSoFar] = 2;
      movesSoFar ++;
    }
    
    else if(button3 == true)
    {
      moves[movesSoFar] = 3;
      movesSoFar ++;
    }
    
    else if(button4 == true)
    {
      _currentState = MAZE_STATE;
    }

    // show moves
    lcd.setCursor(0, 1);
    lcd.print(moves[movesSoFar - 1]);

    if (movesSoFar > 1)
    {
      lcd.setCursor(8, 1);
      lcd.print(moves[movesSoFar - 2]);
    }

    while(button4 == true || button3 == true || button2 == true || button1 == true)
    {
      // re read
      buttonVal = analogRead(PUSH_BUTTONS);
    
      // debounce
      if (buttonVal <= 0)
      {
        button4 = false;
        button3 = false;
        button2 = false;
        button1 = false;
      }
    }
    
    // error check
    if (movesSoFar == numOfMoves)
    {
      // show error
      lcd.setCursor(0, 1);
      lcd.print("Out of room");
    }
  }
  
  while (_currentState == MAZE_STATE)
  {
    // make variables
    int i = 1;
    boolean hot;

    // show state
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Maze Time");

    while(i < movesSoFar && Fwall == false)
    {
      // show moves
      lcd.setCursor(0, 1);
      lcd.print(moves[i]);
      
      if (moves[i] == 3)
      {
        Serial.println("Went Left");
        goLeft();
      }
      
      else if (moves[i]  == 2)
      {
        Serial.println("Went Straight");
        goStraight(24);
      }
      
      else if (moves[i]  == 1)
      {
        Serial.println("Went Right");
        goRight();
      }
      
      Fwall = senseFWall();
      
      // incrament
      i++;
    }
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Searching...");

    // see if its hot
    hot = isHeat();
    Fwall = senseFWall();
  
    // hit a wall?
    if (Fwall == true)
    {
      // Show error message
      lcd.setCursor(0, 1);
      lcd.print("Uh oh, WALL!");

      //sound
      analogWrite(SPEAKER, 160);

      // jump out of exicution
      _currentState = ERROR_STATE;
    }
    
    // if there is heat, you know you made it
    if (Fwall == false && i <= movesSoFar + 1 && hot == true)
    {
      _currentState = HOUSE_STATE;
    }

    // maybe it just needs to move forward more?
    else
    {
      // try going forward
      while (Fwall == false && hot == false)
      {
          Fwall = senseFWall();
          hot = isHeat();
         leftMotor->run(FORWARD);
         leftMotor->setSpeed(LSpeed);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(RSpeed);
         delay(400);
         leftMotor->run(FORWARD);
         leftMotor->setSpeed(0);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(0);
         delay(100);
      }

      // found it!
      if (hot == true)
      {
        _currentState = HOUSE_STATE;
      }

      // default to wall following
      if (Fwall == true)
      {
        // Show error message
        lcd.setCursor(0, 1);
        lcd.print("Uh oh, WALLS!");
        Serial.println("Wall");
         leftMotor->run(BACKWARD);
         leftMotor->setSpeed(LSpeed);
         rightMotor->run(BACKWARD);
         rightMotor->setSpeed(RSpeed);
         delay(170);
         leftMotor->run(FORWARD);
         leftMotor->setSpeed(0);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(0);

        //sound?
        analogWrite(SPEAKER, 160);
  
        // jump out of exicution
        _currentState = ERROR_STATE;
      }
    }
  }
  
  while (_currentState == ERROR_STATE)
  {
    // default to wall following

    // show state
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Following");
    
    // create local variables
    int left = 0;
    int right = 0;
    int x = 1;
    
    // figure out where you are
    for (int i = 1; i <= movesSoFar; i++)
    {
      if (moves[i] == 1)
      {
        while (moves[x] != 3 && x <= movesSoFar)
        {
          right ++;
          Serial.println(right);
          Serial.print("right");
          x ++;
          i ++;
        }
        i ++;
      }
        
      else if (moves[i] == 3)
      {
        while (moves[x] != 1 && x <= movesSoFar)
        {
          left ++;
          Serial.println(left);
          Serial.print("left");
          x ++;
          i ++;
        }
        i ++;
      }
      
      //incrament x
      x ++;
    }

    analogWrite(SPEAKER, 0);
    
    // am I left or right?
    if (left > right)
    {
      followL();
    }
    
    else
    {
      followR();
    }
  }
  
  while (_currentState == HOUSE_STATE)
  {
    // show state
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("In the house");

    // now follow a wall?
    if (moves[0] == 1)
    {
      followR();
    }
    
    else if (moves[0]  == 3)
    {
      followL();
    }

    else
    {
      followR();
    }
  }
  
  while (_currentState == LIGHT_STATE)
  {
    // show found
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lego attached");

    // make noise
    foundItSound();

    // make sure you get it attached
    for (int i = 0; i <= 40; i++)
      {
        leftMotor->run(FORWARD);
        leftMotor->setSpeed(160);
        rightMotor->run(FORWARD);
        rightMotor->setSpeed(160);
      }
    for (int i = 0; i <= 40; i++)
      {
        leftMotor->run(BACKWARD);
        leftMotor->setSpeed(160);
        rightMotor->run(BACKWARD);
        rightMotor->setSpeed(160);
      }
    for (int i = 0; i <= 40; i++)
      {
        leftMotor->run(FORWARD);
        leftMotor->setSpeed(160);
        rightMotor->run(FORWARD);
        rightMotor->setSpeed(160);
      }

    // stop noise
    analogWrite(SPEAKER, 0);

    _currentState = RHOUSE_STATE;
  }    
  
  while (_currentState == RHOUSE_STATE)
  {
    // show state
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Leaving now");

    // trigger memory
    headedHome = true;
    
    // now follow the wall back out
    if (moves[0] == 1)
    {
      // Turn a 180
      goRight();
      goRight();

      // leave
      followL();
    }
    
    else if (moves[0]  == 3)
    {
      // Turn a 180
      goLeft();
      goLeft();

      // leave
      followR();
    }
  }
  
  while (_currentState == RMAZE_STATE)
  {
    // show state
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Maze...Again?!");

    // make variable
    int i = movesSoFar;

    // go ahead and move back through it
    while(i > 0 && Fwall == false)
    {
      // show moves
      lcd.setCursor(0, 1);
      lcd.print(moves[i]);
      
      if (moves[i] == 1)
      {
        goLeft();
      }
      
      else if (moves[i]  == 2)
      {
        goStraight(24);
      }
      
      else if (moves[i]  == 3)
      {
        goRight();
      }
      
      Fwall = senseFWall();
      
      // decrament
      i--;
    }
      
      if (Fwall == true)
      {
        // Show error message
        lcd.setCursor(0, 1);
        lcd.print("Uh oh, WALLS!");
        Serial.println("Wall");

        //sound
        analogWrite(SPEAKER, 160);

        // jump out of exicution
        _currentState = ERROR_STATE;
      }

      if (i <= 1)
      {
        _currentState = VICTORY_STATE;
      }
  }
  
  if (_currentState == VICTORY_STATE)
  {
    // show state
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("YAY!!");

    //sound
    analogWrite(SPEAKER, 10);
    
    // spin
    goLeft();
    goLeft();
    goLeft();
    goLeft();

    //sound off
    analogWrite(SPEAKER, 0);
  }
}

///////////////////////////////////////////////////
// Functions!
///////////////////////////////////////////////////

void followR()
{
  // variables
  int fWallDistance;
  int lWallDistance;
  int rWallDistance;
  int sides;
  int error = 0;
  boolean heat;
  boolean light;

  // show state
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Following R");
  
  // check for heat and light
  heat = isHeat();
  light = isLight();

  // spin so it actually sees the wall
  goLeft();
  
  //make decision
  while (heat == false && light == false)
  {

    // recheck all
    // ping and do conversion
    // Values are returned in integer values of 1 inch increments.
    fWallDistance = sonarFront.convert_in(sonarFront.ping_median(AMOUNT_OF_PULSES));
    rWallDistance = sonarRight.convert_in(sonarRight.ping_median(AMOUNT_OF_PULSES));
    lWallDistance = sonarLeft.convert_in(sonarLeft.ping_median(AMOUNT_OF_PULSES));

    // error check
    if(fWallDistance == 0)
    {
      lcd.setCursor(8, 1);
      lcd.print("F = 0");
      error ++;
    }

    else if(error >= 4)
    {
      // move the motors a bit
      for (int i = 0; i <= 100; i++)
      {
         leftMotor->run(BACKWARD);
         leftMotor->setSpeed(180);
         rightMotor->run(BACKWARD);
         rightMotor->setSpeed(180);
      }
      leftMotor->setSpeed(0);
      rightMotor->setSpeed(0);
    }
    
    // do math
    sides = rWallDistance + lWallDistance;
    
    // check for heat and light
    heat = isHeat();
    light = isLight();
  
    //if can go Right, do such
    if(rWallDistance >= 12 && error < 4)
    {
      // show state
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Right   ");

      // reset error
      error = 0;

      // move
      goRight();
      goStraight(20);

      // stay like that until you can do something different
      while(rWallDistance <= 7 && rWallDistance >= 4 && fWallDistance >= 7)
      {
          leftMotor->run(FORWARD);
         leftMotor->setSpeed(LSpeed);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(RSpeed);
         delay(30);
         leftMotor->run(FORWARD);
         leftMotor->setSpeed(0);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(0);
         delay(500);
        rWallDistance = sonarRight.convert_in(sonarRight.ping_median(AMOUNT_OF_PULSES));
        fWallDistance = sonarFront.convert_in(sonarFront.ping_median(AMOUNT_OF_PULSES));
        lcd.setCursor(0, 1);
        lcd.print("Straight");
      }
    }
    
    // just need to veer right?
    else if(rWallDistance < 12 && rWallDistance >= 5 && fWallDistance >= 6)
    {
      // show state
      lcd.setCursor(0, 1);
      lcd.print("Veer    R");

      // move the motors a bit
      for (int i = 0; i <= 80; i++)
      {
         leftMotor->run(FORWARD);
         leftMotor->setSpeed(LSpeed);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(RSpeed - 5); // 8);
      }
      leftMotor->setSpeed(0);
      rightMotor->setSpeed(0);
    }
    
    // just need to veer left?
    else if(rWallDistance <= 5 && rWallDistance >= 1 && fWallDistance >= 6)
    {
      // show state
      lcd.setCursor(0, 1);
      lcd.print("Veer    L");
      
      // move the motors a bit
      for (int i = 0; i <= 80; i++)
      {
         leftMotor->run(FORWARD);
          leftMotor->setSpeed(LSpeed - 15); // 60);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(RSpeed);
      }
      leftMotor->setSpeed(0);
      rightMotor->setSpeed(0);
    }

    // are you out of the maze?
    else if(sides >= 50)
    {
      // get me outa here
      _currentState = VICTORY_STATE;
      heat = true;
    }
    
    // last resort, go left
    else
    {
      // show state
      lcd.setCursor(0, 1);
      lcd.print("Left    ");
      
      goLeft();
      goStraight(20);

      // stay like that until you can do something different
      while(lWallDistance <= 6 && lWallDistance >= 2 && fWallDistance >= 6)
      {
        leftMotor->run(FORWARD);
         leftMotor->setSpeed(LSpeed);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(RSpeed);
         delay(30);
         leftMotor->run(FORWARD);
         leftMotor->setSpeed(0);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(0);
         delay(500);
        lWallDistance = sonarLeft.convert_in(sonarLeft.ping_median(AMOUNT_OF_PULSES));
        fWallDistance = sonarFront.convert_in(sonarFront.ping_median(AMOUNT_OF_PULSES));
      }
    }
  }
  
  // if you find heat, your in the house
  if (heat == true)
  {
    // figure out if coming or going
    if (headedHome == false)
    {
      _currentState = HOUSE_STATE;
    }
    else if (_currentState = VICTORY_STATE)
    {
      _currentState = VICTORY_STATE;
    }
    else
    {
      _currentState = RMAZE_STATE;
    }
  }

  // if you find light, your on the lego
  if (light == true)
  {
    _currentState = LIGHT_STATE;
  }
}

void followL()
{
  // variables
  int fWallDistance;
  int lWallDistance;
  int rWallDistance;
  int sides;
  int error = 0;
  boolean heat;
  boolean light;

  // show state
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Following L");
  
  // check for heat and light
  heat = isHeat();
  light = isLight();

  // spin so it actually sees the wall
  goRight();
  
  //make decision
  while (heat == false && light == false)
  {
    // recheck all
    // ping and do conversion
    // Values are returned in integer values of 1 inch increments.
    fWallDistance = sonarFront.convert_in(sonarFront.ping_median(AMOUNT_OF_PULSES));
    rWallDistance = sonarRight.convert_in(sonarRight.ping_median(AMOUNT_OF_PULSES));
    lWallDistance = sonarLeft.convert_in(sonarLeft.ping_median(AMOUNT_OF_PULSES));
    lcd.setCursor(8, 1);
    lcd.print("     ");
    
    // error check
    if(fWallDistance == 0)
    {
      lcd.setCursor(8, 1);
      lcd.print("F = 0");
      error ++;
    }

    else if(error >= 4)
    {
      // move the motors a bit
      for (int i = 0; i <= 100; i++)
      {
         leftMotor->run(BACKWARD);
         leftMotor->setSpeed(180);
         rightMotor->run(BACKWARD);
         rightMotor->setSpeed(180);
      }
      leftMotor->setSpeed(0);
      rightMotor->setSpeed(0);
    }
    
    // do math
    sides = rWallDistance + lWallDistance;
    
    // check for heat and light
    heat = isHeat();
    light = isLight();
    
    //if can go Left, do such
    if(lWallDistance >= 12 && error < 4)
    {
      // show state
      lcd.setCursor(0, 1);
      lcd.print("Left    ");

      // reset error
      error = 0;
      
      // move
      goLeft();
      goStraight(20);

      // stay like that until you can do something different
      while(lWallDistance <= 7 && lWallDistance >= 4 && fWallDistance >= 7)
      {
        leftMotor->run(FORWARD);
         leftMotor->setSpeed(LSpeed);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(RSpeed);
         delay(30);
         leftMotor->run(FORWARD);
         leftMotor->setSpeed(0);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(0);
         delay(500);
        lWallDistance = sonarLeft.convert_in(sonarLeft.ping_median(AMOUNT_OF_PULSES));
        fWallDistance = sonarFront.convert_in(sonarFront.ping_median(AMOUNT_OF_PULSES));
        lcd.setCursor(0, 1);
        lcd.print("Straight");
      }
    }
    
    // just need to veer left?
    else if(lWallDistance < 12 && lWallDistance >= 5 && fWallDistance >= 6)
    {
      // show state
      lcd.setCursor(0, 1);
      lcd.print("Veer    L");

      // move the motors a bit
      for (int i = 0; i <= 80; i++)
      {
         leftMotor->run(FORWARD);
         leftMotor->setSpeed(LSpeed - 15);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(RSpeed);
      }
      leftMotor->setSpeed(0);
      rightMotor->setSpeed(0);
    }

    // just need to veer right?
    else if(lWallDistance <= 5  && lWallDistance >= 1 && fWallDistance >= 6)
    {
      // show state
      lcd.setCursor(0, 1);
      lcd.print("Veer    R");

      // move the motors a bit
      for (int i = 0; i <= 80; i++)
      {
         leftMotor->run(FORWARD);
         leftMotor->setSpeed(LSpeed);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(RSpeed - 5);
      }
      leftMotor->setSpeed(0);
      rightMotor->setSpeed(0);
    }

    // are you out of the maze?
    else if(sides >= 50)
    {
      // get me out of here
      _currentState = VICTORY_STATE;
      heat = true;
    }
    
    // last resort, go right
    else
    {
      // show state
      lcd.setCursor(0, 1);
      lcd.print("Right   ");
      
      goRight();
      goStraight(20);

      // stay like that until you can do something different
      while(lWallDistance <= 6 && lWallDistance >= 2 && fWallDistance >= 6)
      {
          leftMotor->run(FORWARD);
         leftMotor->setSpeed(LSpeed);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(RSpeed);
         delay(30);
         leftMotor->run(FORWARD);
         leftMotor->setSpeed(0);
         rightMotor->run(FORWARD);
         rightMotor->setSpeed(0);
         delay(500);
        rWallDistance = sonarRight.convert_in(sonarRight.ping_median(AMOUNT_OF_PULSES));
        fWallDistance = sonarFront.convert_in(sonarFront.ping_median(AMOUNT_OF_PULSES));
      }
    }
  }
  
  // if you find heat, your in the house
  if (heat == true)
  {
    // figure out if coming or going
    if (headedHome == false)
    {
      _currentState = HOUSE_STATE;
    }
    else if (_currentState = VICTORY_STATE)
    {
      _currentState = VICTORY_STATE;
    }
    else
    {
      _currentState = RMAZE_STATE;
    }
  }

  // if you find light, your on the lego
  if (light == true)
  {
    _currentState = LIGHT_STATE;
  }
}

// Drives straight the sent in distance
void goStraight(int distance)
{
  // get the wall distance
  int pings = 12;
  int wallDistance = sonarFront.convert_in(sonarFront.ping_median(pings));
  int rWallDistanceInitial = (sonarRight.convert_in(sonarRight.ping_median(AMOUNT_OF_PULSES)))/ 24;
  int lWallDistanceInitial = (sonarLeft.convert_in(sonarLeft.ping_median(AMOUNT_OF_PULSES))   )/ 24;
  int rWallDistance = (sonarRight.convert_in(sonarRight.ping_median(AMOUNT_OF_PULSES)))/ 24;
  int lWallDistance = (sonarLeft.convert_in(sonarLeft.ping_median(AMOUNT_OF_PULSES)))/ 24;
  Serial.print(wallDistance);
  int finalDistance = wallDistance - distance;
  
  
  // if it wont hit
    
    // get them moving
    rightMotor->run(FORWARD);
    leftMotor->run(FORWARD);
    leftMotor->setSpeed(LSpeed);
    rightMotor->setSpeed(RSpeed);
    currentDistance = wallDistance;
    Fwall = senseFWall();

//    while ((rWallDistance ==  rWallDistanceInitial) && (lWallDistance ==  lWallDistanceInitial) && rWallDistance != 0 && lWallDistance != 0) // runs until you are "distance" (input value) closer to the wall than you started
//    {
    while ((rWallDistance ==  rWallDistanceInitial) && (lWallDistance ==  lWallDistanceInitial) && Fwall == false) // runs until you are "distance" (input value) closer to the wall than you started
    {
//      currentDistance = sonarFront.convert_in(sonarFront.ping_median(pings));
      Fwall = senseFWall();
      rWallDistance = (sonarRight.convert_in(sonarRight.ping_median(AMOUNT_OF_PULSES))) / 24;
      lWallDistance = (sonarLeft.convert_in(sonarLeft.ping_median(AMOUNT_OF_PULSES))) / 24;
      lcd.setCursor(0, 0);
      lcd.print("Go forward");
      lcd.setCursor(8, 1);
      lcd.print(currentDistance);
    }

    for(int i = 0; i <= 65; i++)
    {
      leftMotor->setSpeed(LSpeed);
      rightMotor->setSpeed(RSpeed);
    }
    rightMotor->setSpeed(0);
    leftMotor->setSpeed(0);
      
    wallDistance = sonarFront.convert_in(sonarFront.ping_median(pings));
    
    if ( wallDistance <= 2)
    {
      for(int i = 0; i <= 60; i++)
      {
        rightMotor->run(BACKWARD);
      leftMotor->run(BACKWARD);
        leftMotor->setSpeed(LSpeed);
        rightMotor->setSpeed(RSpeed);
      }
      rightMotor->setSpeed(0);
      leftMotor->setSpeed(0);
      rightMotor->run(FORWARD);
      leftMotor->run(FORWARD);
    }

//  // otherwise it hit a wall
//  else
//  {
//    lcd.setCursor(0, 1);
//    lcd.print("Failure");
////    goStraight(distance/2);
//    leftMotor->setSpeed(0);
//    rightMotor->setSpeed(0);
//  }
}

// turns 90 degrees right
void goRight()
{
  // move
  rightMotor->run(BACKWARD);
  leftMotor->run(FORWARD);
  leftMotor->setSpeed(LSpeed);
  rightMotor->setSpeed(0);
  
  // Run for a while
      for (int i = 0; i <= 190; i++)
    {
      lcd.setCursor(0,1);
      lcd.print ("Turn Right");
    }
//
  leftMotor->setSpeed(0);
  rightMotor->setSpeed(0);
}

// Turns 90 degrees left
void goLeft()
{
  // move
  rightMotor->run(FORWARD);
  leftMotor->run(BACKWARD);
  leftMotor->setSpeed(0);
  rightMotor->setSpeed(RSpeed);

  // Run for a while
    for (int i = 0; i <= 225; i++)
    {
      lcd.setCursor(0,1);
      lcd.print ("Turn Left");
    }
    
  leftMotor->setSpeed(0);
  rightMotor->setSpeed(0);
}

boolean senseLWall()
{
  ///////////////////////////////NOTE: When sonar reads 5, light sensor = 9 inches from wall.
  // declare variables
  int wallDistance;
  int hit = 12;
  boolean wall = false;
  
  // ping and do conversion
  // Values are returned in integer values of 1 inch increments.
  wallDistance = sonarLeft.convert_in(sonarLeft.ping_median(AMOUNT_OF_PULSES));
  Serial.println(wallDistance);
  
  // too close
  if(wallDistance <= hit)
  {
    wall = true;
  }
  
  // return it
  return wall;
}

boolean senseRWall()
{
  ///////////////////////////////NOTE: When sonar reads 5, light sensor = 9 inches from wall.
  // declare variables
  int wallDistance;
  int hit = 12;
  boolean wall = false;
  
  // ping and do conversion
  // Values are returned in integer values of 1 inch increments.
  wallDistance = sonarRight.convert_in(sonarRight.ping_median(AMOUNT_OF_PULSES));
  Serial.println(wallDistance);
  
  // too close
  if(wallDistance <= hit)
  {
    wall = true;
  }
  
  // return it
  return wall;
}

boolean senseFWall()
{
  ///////////////////////////////NOTE: When sonar reads 2, light sensor = 9 inches from wall.
  // declare variables
  int wallDistance = 0;
  int hit = 6;
  boolean wall = false;
  
  // ping and do conversion
  // Values are returned in integer values of 1 inch increments.
  wallDistance = sonarFront.convert_in(sonarFront.ping_median(AMOUNT_OF_PULSES));
  Serial.println(wallDistance);
  
  // too close
  if(wallDistance <= hit && wallDistance)
  {
    wall = true;
  }
  
  // return it
  return wall;
}

boolean isHeat()
{
  // declare variable
  boolean hot = false;
  
  // get data
  heatVal = analogRead(HEAT_SENSOR);
  Serial.print("HEAT: ");
  Serial.println(heatVal);
  
  // figure out what temp it should trigger
  if (heatVal >= Tsensor + 5)
  {
//    hot = true;
  }
  
  // return it
  return hot;
}

boolean isLight()
{
   // declare variable
  boolean light = false;
  
  // get data
  lightVal = analogRead(LIGHT_SENSOR);
  Serial.print("LIGHT: ");
  Serial.println(lightVal);
  
  // figure out when it should trigger
  if (lightVal >= Lsensor + 150)
  {
//    light = true;
  }
  
  // return it
  return light;
}


// to make cool sounds
void foundItSound()
{
  for (int i = 0; i <= 2000; i ++)
  {
    analogWrite(SPEAKER, 200);
  }
  for (int i = 0; i <= 2000; i ++)
  {
    analogWrite(SPEAKER, 100);
  }
  for (int i = 0; i <= 2000; i ++)
  {
    analogWrite(SPEAKER, 50);
  }
  for (int i = 0; i <= 2000; i ++)
  {
    analogWrite(SPEAKER, 100);
  }
  for (int i = 0; i <= 2000; i ++)
  {
    analogWrite(SPEAKER, 200);
  }
}
