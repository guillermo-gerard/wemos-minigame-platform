#ifndef JuegoTest_h
#define JuegoTest_h

#include "Arduino.h"
#include <ESP_EEPROM.h>
#include "IGame.h"
#include <U8g2lib.h>

class JuegoTest : public IGame
{
public:
    JuegoTest(int screenWidth,
            int screenHeight,
            int highScoreMemPos,
            U8G2_SH1106_128X64_NONAME_1_HW_I2C *Display,
            uint8_t startButton,
            uint8_t leftButton,
            uint8_t righButton);

private:
    int screenWidth;
    int screenHeight;

    uint8_t startButton;
    uint8_t leftButton;
    uint8_t righButton;

    unsigned long lastFrameMillis;

    unsigned long score;
    unsigned long highScore;
    int highScoreMemPos;

    U8G2_SH1106_128X64_NONAME_1_HW_I2C *Display;

public:
    //IGame implementations
    void WelcomeScreen() override;
    bool RunFrame() override;
    void EndGameAnimation(void) override;
    void CountBackwardsAndStart(int) override;
    char* GetName() override;

private:
    void ShowStart(void);
    void Start(void);
  };

#endif