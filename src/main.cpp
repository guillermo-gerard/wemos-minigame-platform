#include <Arduino.h>
#include <ESP_EEPROM.h>
#include <ESP8266WiFi.h>

#include <U8g2lib.h>

#define DEBUG

const int highScoreMemPos = 0x00;

const uint8_t startButton = D7;
const uint8_t leftButton = D5;
const uint8_t righButton = D6;

U8G2_SH1106_128X64_NONAME_1_HW_I2C Display(U8G2_R0);

//Config values follows
//Interchange W and H to work in portrait mode
const int screenWidth = 128;
const int screenHeight = 64;

const int arkaLength = 20;
const int arkaWidth = 5;

const int ballDiameter = 6;

const int ballSpeed = 2;
const int arkaSpeed = 4;
const unsigned long millisFactor = 50;

unsigned long lastFrameMillis;
unsigned long inactivityCounterStartMillis;
unsigned const long inactiviySecondsBeforeGoingToBed = 10;

unsigned long score;
unsigned long highScore;

//positions on screen
int arkaX;
const int arkaY = screenHeight - 5 - arkaWidth;;
int ballX;
int ballY;

void SetInitialPositions(void);
void WelcomeScreen(void);
void CountBackwards(void);
void DrawFrame(int ballX, int ballY, int arkaX, int arkaY);
int CalculateBallX(unsigned long delta);
int CalculateBallY(unsigned long delta);
void ShowStart(void);
void Start(void);
void WaitForStart(void);
void RunFrame(void);
void EndGameAnimation(void);

int STATUS_CURRENT = 0;
const int STATUS_WELCOME = 0;
const int STATUS_WAITING = 1;
const int STATUS_PLAYING = 2;
const int STATUS_LOST = 3;

const int UP = 0;
const int DOWN = 1;
int DIRECTION_X = UP;

const int RIGHT = 0;
const int LEFT = 1;
int DIRECTION_Y = RIGHT;

#ifdef DEBUG
  void LogDebug(String text){
    Serial.println(text);
  }
#endif
#ifndef DEBUG
void LogDebug(String text){;}
#endif

void setup()
{
  //the following turn the modem off to save power big time
    WiFi.disconnect();
    WiFi.forceSleepBegin();
    delay(1); //For some reason the modem won't go to sleep unless you do a delay(non-zero-number) -- no delay, no sleep and delay(0), no sleep

  //How to turn the modem back on:
    // WiFi.forceSleepEnd();
    // delay(1);
  
  #ifdef DEBUG
  Serial.begin(9600);
  #endif

  EEPROM.begin(16);

  pinMode(startButton, INPUT);
  pinMode(leftButton, INPUT);
  pinMode(righButton, INPUT);

  Display.begin();
  Display.setFont(u8g2_font_5x7_mf);
  Display.enableUTF8Print();

  WelcomeScreen();
  STATUS_CURRENT = STATUS_WAITING;

  randomSeed(analogRead(0));
  DIRECTION_X = random(2);
  DIRECTION_Y = random(2);
  SetInitialPositions();

  EEPROM.get(highScoreMemPos, highScore);
}

void loop()
{
  switch (STATUS_CURRENT)
  {
  case STATUS_WAITING:
    WaitForStart();
    return;
  case STATUS_PLAYING:
    RunFrame();
    return;
  case STATUS_LOST:
    EndGameAnimation();
    return;
  }
}

void SetInitialPositions(){
  arkaX = (screenWidth / 2) - (arkaLength / 2);
  
  ballX = (screenWidth / 2) - (ballDiameter / 2);
  ballY = (screenHeight / 2) - 5 - (ballDiameter / 2);
}

void WelcomeScreen()
{
  inactivityCounterStartMillis = millis();
  Display.firstPage();
  do
  {
    Display.setCursor(4, 22);
    Display.setFont(u8g2_font_fub20_tr);
    Display.print("ArduNoid");
    Display.setFont(u8g2_font_4x6_mf);
    Display.drawStr(10, 45, "Press start button to begin");
  } while (Display.nextPage());
}

void CountBackwards(int seconds)
{
  for (int i = seconds; i > 0; i--)
  {
    Display.firstPage();
    do
    {
      Display.setCursor(40, 32);
      Display.setFont(u8g2_font_fub20_tr);
      Display.print(i);
      Display.print("...");
    } while (Display.nextPage());
    delay(1000);
  }
}

void EndGameAnimation()
{
  Display.firstPage();
  do
  {
    Display.setCursor(1, 32);
    Display.setFont(u8g2_font_fub14_tr);
    Display.print("GAME OVER");
    if(score > highScore){
      Display.setFont(u8g2_font_4x6_mf);
      Display.drawStr(27, 60, "New high score!!");
    }
  } while (Display.nextPage());

  if(score > highScore){
    highScore = score;
    EEPROM.put(highScoreMemPos, highScore);  // long - so 8 bytes
    EEPROM.commit();
  }

  delay(3000);
  WelcomeScreen();
  score = 0;
  STATUS_CURRENT = STATUS_WAITING;
}

void WaitForStart()
{
  if (!digitalRead(startButton))
  {
    CountBackwards(3);
    ShowStart();
    Start();
    STATUS_CURRENT = STATUS_PLAYING;
    return;
  }
 
  LogDebug(String("testing if it is time to go to bed"));
  //going to sleep if the device is inactive
  if(millis() - inactivityCounterStartMillis > inactiviySecondsBeforeGoingToBed * 1000){
    ESP.deepSleep(0);
  }
}

void ShowStart(void)
{
  SetInitialPositions();
  Display.firstPage();
  do
  {
    Display.setCursor(15, 32);
    Display.setFont(u8g2_font_fub20_tr);
    Display.print("START!");
  } while (Display.nextPage());
  delay(1000);
}

void Start(void)
{
  lastFrameMillis = millis();
}

void RunFrame()
{
  unsigned long currentMillis = millis();
  unsigned long delta = currentMillis - lastFrameMillis;

  if (!digitalRead(leftButton))
  {
    arkaX = arkaX - ((arkaSpeed * delta) / millisFactor);
    if (arkaX < arkaLength / 2)
      arkaX = arkaLength / 2;
  }
  else if (!digitalRead(righButton))
  {
    arkaX = arkaX + ((arkaSpeed * delta) / millisFactor);
    if (arkaX >= screenWidth - arkaLength / 2)
      arkaX = screenWidth - arkaLength / 2;
  }

  int ballX = CalculateBallX(delta);
  int ballY = CalculateBallY(delta);

  Display.drawRFrame(arkaX - arkaLength / 2, arkaY, arkaLength, arkaWidth, 2);

  if ((ballY + ballDiameter / 2) >= arkaY && (ballX - ballDiameter / 2) > (arkaX - arkaLength / 2) && (ballX - ballDiameter / 2) < (arkaX + arkaLength / 2))
  {
    //collition detected
    DIRECTION_Y = UP;
    score++;
  }

  if ((ballY + ballDiameter / 2) >= screenHeight)
  {
    //end game
    STATUS_CURRENT = STATUS_LOST;
    return;
  }

  //TODO: check ball hit or ball lost
  DrawFrame(ballX, ballY, arkaX, arkaY);

  lastFrameMillis = currentMillis;
}

int CalculateBallX(unsigned long delta)
{
  if (DIRECTION_X == RIGHT)
  {
    ballX = ballX + ((ballSpeed * delta) / millisFactor);
    if (ballX >= (screenWidth - ballDiameter / 2))
    {
      DIRECTION_X = LEFT;
      ballX = screenWidth - ballDiameter / 2;
    }
  }

  if (DIRECTION_X == LEFT)
  {
    ballX = ballX - ((ballSpeed * delta) / millisFactor);
    if (ballX <= (ballDiameter / 2))
    {
      DIRECTION_X = RIGHT;
      ballX = ballDiameter / 2;
    }
  }
  return ballX;
}

int CalculateBallY(unsigned long delta)
{
  if (DIRECTION_Y == UP)
  {
    ballY = ballY - ((ballSpeed * delta) / millisFactor);
    if (ballY <= (ballDiameter / 2))
    {
      DIRECTION_Y = DOWN;
      ballY = ballDiameter / 2;
    }
  }

  if (DIRECTION_Y == DOWN)
  {
    ballY = ballY + ((ballSpeed * delta) / millisFactor);
    if (ballY >= (screenHeight - ballDiameter / 2))
    {
      DIRECTION_Y = UP;
      ballY = screenHeight - ballDiameter / 2;
    }
  }
  return ballY;
}

void DrawFrame(int ballX, int ballY, int arkaX, int arkaY)
{
  Display.firstPage();
  do
  {
    Display.drawRFrame(arkaX - arkaLength / 2, arkaY, arkaLength, arkaWidth, 2);
    Display.drawCircle(ballX, ballY, ballDiameter / 2, U8G2_DRAW_ALL);
    Display.setFont(u8g2_font_4x6_mf);
    Display.drawStr(1,6, (String("Score: ") + String(score)).c_str());
    Display.drawStr(90,6, (String("High: ") + String(highScore)).c_str());
  } while (Display.nextPage());
}
