#ifndef PongGame_h
#define PongGame_h

#include "Arduino.h"
#include <ESP_EEPROM.h>
#include "IGame.h"
#include <U8g2lib.h>

class PongGame : public IGame
{
public:
    PongGame(int screenWidth,
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

    int arkaLength;
    int arkaWidth;

    int ballDiameter;

    unsigned long lastFrameMillis;

    int ballSpeed;
    int arkaSpeed;
    unsigned long millisFactor;

    unsigned long score;
    unsigned long highScore;
    int highScoreMemPos;

    //positions on screen
    int arkaX;
    int arkaY;
    int ballX;
    int ballY;

    U8G2_SH1106_128X64_NONAME_1_HW_I2C *Display;

    const int UP = 0;
    const int DOWN = 1;
    const int RIGHT = 0;
    const int LEFT = 1;

    int DIRECTION_X;
    int DIRECTION_Y;

public:
    //IGame implementations
    void WelcomeScreen() override;
    bool RunFrame() override;
    void EndGameAnimation(void) override;
    void CountBackwardsAndStart(int) override;

private:
    void DrawFrame();
    void ShowStart(void);
    void Start(void);
    void SetInitialPositionsAndDirections();
    int CalculateBallX(unsigned long delta);
    int CalculateBallY(unsigned long delta);
};

#endif