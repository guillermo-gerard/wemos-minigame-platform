#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <U8g2lib.h>

#include <PongGame.h>
#include <JuegoTest.h>

#define DEBUG

const int highScoreMemPosGame0 = 0x00;
const int highScoreMemPosGame1 = 0x04;
const int highScoreMemPosGame2 = 0x08;
const int highScoreMemPosGame3 = 0x0B;

const uint8_t startButton = D7;
const uint8_t leftButton = D5;
const uint8_t rightButton = D6;

U8G2_SH1106_128X64_NONAME_1_HW_I2C Display(U8G2_R0);

const int screenWidth = 128;
const int screenHeight = 64;

PongGame Pong(screenWidth, screenHeight, highScoreMemPosGame0, &Display, 
startButton, leftButton, rightButton);

JuegoTest Test(screenWidth, screenHeight, highScoreMemPosGame1, &Display, 
startButton, leftButton, rightButton);

PongGame Pong2(screenWidth, screenHeight, highScoreMemPosGame0, &Display, 
startButton, leftButton, rightButton);


IGame* games[] = {&Pong, &Test, &Pong2};
int gameCount = 3;

int selectedGameIndex = 0;

unsigned long inactivityCounterStartMillis;
unsigned const long inactiviySecondsBeforeGoingToBed = 100;
bool isGameLost = false;

enum class GameStatus
{
  STATUS_MENU,
  STATUS_WELCOME,
  STATUS_WAITING,
  STATUS_PLAYING,
  STATUS_LOST
};

GameStatus STATUS_CURRENT = GameStatus::STATUS_MENU;

unsigned long startDebounce = 0;
unsigned const int debounceTime = 500;

#ifdef DEBUG
  void LogDebug(String text){
    Serial.println(text);
  }
#endif
#ifndef DEBUG
void LogDebug(String text){;}
#endif

void WaitForStart();
void RunMenu();

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
  Serial.println("starting");

  EEPROM.begin(16);

  pinMode(startButton, INPUT);
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);

  Display.begin();
  Display.setFont(u8g2_font_5x7_mf);
  Display.enableUTF8Print();

  inactivityCounterStartMillis = millis();
  games[selectedGameIndex]->WelcomeScreen();
}

void loop()
{
  switch (STATUS_CURRENT){
    case GameStatus::STATUS_MENU:
      RunMenu();
      return;
    case GameStatus::STATUS_WAITING:
      WaitForStart();
      return;
    case GameStatus::STATUS_PLAYING:
      isGameLost = games[selectedGameIndex]->RunFrame();
      if(isGameLost){
        STATUS_CURRENT = GameStatus::STATUS_LOST;
      }
      return;
    case GameStatus::STATUS_LOST:
      games[selectedGameIndex]->EndGameAnimation();
      isGameLost = false;
      STATUS_CURRENT = GameStatus::STATUS_WAITING;
      inactivityCounterStartMillis = millis();
      return;
    case GameStatus::STATUS_WELCOME:
      return;
  }
}


void RunMenu(){

  Display.firstPage();
  do
  {
    Display.setCursor(1, 32);
    Display.setFont(u8g2_font_fub14_tr);
    Display.print(games[selectedGameIndex]->GetName());
    
    if (selectedGameIndex > 0)
    {
      Display.setCursor(1, 60);
      Display.print("<");
    }

    if(selectedGameIndex < gameCount-1){
      Display.setCursor(110, 60);
      Display.print(">");
    }
  } while (Display.nextPage());

  if(selectedGameIndex > 0 && !digitalRead(leftButton)){
    selectedGameIndex--;
  }
 
  if(millis() - debounceTime < startDebounce){
    return;
  }
 
  if(selectedGameIndex < gameCount-1 && !digitalRead(rightButton)){
    startDebounce = millis();
    selectedGameIndex++;
  }
  if(!digitalRead(startButton)){
    STATUS_CURRENT = GameStatus::STATUS_WAITING;
  }
}

void WaitForStart()
{
  if (!digitalRead(startButton))
  {
    games[selectedGameIndex]->CountBackwardsAndStart(3);
    STATUS_CURRENT = GameStatus::STATUS_PLAYING;
    return;
  }
 
 // LogDebug(String("testing if it is time to go to bed"));
  //going to sleep if the device is inactive
  if(millis() - inactivityCounterStartMillis > inactiviySecondsBeforeGoingToBed * 1000){
    ESP.deepSleep(0);
  }
}