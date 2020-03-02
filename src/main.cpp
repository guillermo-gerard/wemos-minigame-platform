#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <U8g2lib.h>

#include <PongGame.h>

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

//TODO: Replace the following with menu selector
IGame* CurrentGame = &Pong;

unsigned long inactivityCounterStartMillis;
unsigned const long inactiviySecondsBeforeGoingToBed = 10;
bool isGameLost = false;

enum class GameStatus
{
  STATUS_WELCOME,
  STATUS_WAITING,
  STATUS_PLAYING,
  STATUS_LOST
};

GameStatus STATUS_CURRENT = GameStatus::STATUS_WAITING;

#ifdef DEBUG
  void LogDebug(String text){
    Serial.println(text);
  }
#endif
#ifndef DEBUG
void LogDebug(String text){;}
#endif

void WaitForStart();

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
  Serial.begin(115200);
  #endif

  EEPROM.begin(16);

  pinMode(startButton, INPUT);
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);

  Display.begin();
  Display.setFont(u8g2_font_5x7_mf);
  Display.enableUTF8Print();

  inactivityCounterStartMillis = millis();
  CurrentGame->WelcomeScreen();
}

void loop()
{
  switch (STATUS_CURRENT){
    case GameStatus::STATUS_WAITING:
      WaitForStart();
      return;
    case GameStatus::STATUS_PLAYING:
      isGameLost = CurrentGame->.RunFrame();
      if(isGameLost){
        STATUS_CURRENT = GameStatus::STATUS_LOST;
      }
      return;
    case GameStatus::STATUS_LOST:
      CurrentGame->EndGameAnimation();
      isGameLost = false;
      STATUS_CURRENT = GameStatus::STATUS_WAITING;
      inactivityCounterStartMillis = millis();
      return;
    case GameStatus::STATUS_WELCOME:
      return;
  }
}

void WaitForStart()
{
  if (!digitalRead(startButton))
  {
    CurrentGame->CountBackwardsAndStart(3);
    STATUS_CURRENT = GameStatus::STATUS_PLAYING;
    return;
  }
 
 // LogDebug(String("testing if it is time to go to bed"));
  //going to sleep if the device is inactive
  if(millis() - inactivityCounterStartMillis > inactiviySecondsBeforeGoingToBed * 1000){
    ESP.deepSleep(0);
  }
}