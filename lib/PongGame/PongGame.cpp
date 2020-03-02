#include "PongGame.h"

PongGame::PongGame(int screenWidth, int screenHeight, int highScoreMemPos,
                   U8G2_SH1106_128X64_NONAME_1_HW_I2C *Display,
                   uint8_t startButton, uint8_t leftButton, uint8_t righButton)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    this->highScoreMemPos = highScoreMemPos;
    this->startButton = startButton;
    this->leftButton = leftButton;
    this->righButton = righButton;
    this->Display = Display;

    arkaLength = 20;
    arkaWidth = 5;

    ballDiameter = 6;

    ballSpeed = 2;
    arkaSpeed = 4;
    millisFactor = 50;

    arkaY = screenHeight - 5 - arkaWidth;
}

void PongGame::SetInitialPositionsAndDirections()
{
    randomSeed(analogRead(0));
    DIRECTION_X = (int)random(2);
    DIRECTION_Y = (int)random(2);

    arkaX = (screenWidth / 2) - (arkaLength / 2);

    ballX = (screenWidth / 2) - (ballDiameter / 2);
    ballY = (screenHeight / 2) - 5 - (ballDiameter / 2);
}

void PongGame::WelcomeScreen()
{
    EEPROM.get(highScoreMemPos, highScore);

    Display->firstPage();
    do
    {
        Display->setCursor(4, 22);
        Display->setFont(u8g2_font_fub20_tr);
        Display->print("ArduNoid");
        Display->setFont(u8g2_font_4x6_mf);
        Display->drawStr(10, 45, "Press start button to begin");
    } while (Display->nextPage());
}

void PongGame::CountBackwardsAndStart(int seconds)
{
    for (int i = seconds; i > 0; i--)
    {
        Display->firstPage();
        do
        {
            Display->setCursor(40, 32);
            Display->setFont(u8g2_font_fub20_tr);
            Display->print(i);
            Display->print("...");
        } while (Display->nextPage());
        delay(1000);
    }

    ShowStart();
    Start();
}

void PongGame::ShowStart(void)
{
    SetInitialPositionsAndDirections();
    Display->firstPage();
    do
    {
        Display->setCursor(15, 32);
        Display->setFont(u8g2_font_fub20_tr);
        Display->print("START!");
    } while (Display->nextPage());
    delay(1000);
}

void PongGame::Start(void)
{
    lastFrameMillis = millis();
}

void PongGame::DrawFrame()
{
    Display->firstPage();
    do
    {
        Display->drawRFrame(arkaX - arkaLength / 2, arkaY, arkaLength, arkaWidth, 2);
        Display->drawCircle(ballX, ballY, ballDiameter / 2, U8G2_DRAW_ALL);
        Display->setFont(u8g2_font_4x6_mf);
        Display->drawStr(1, 6, (String("Score: ") + String(score)).c_str());
        Display->drawStr(90, 6, (String("High: ") + String(highScore)).c_str());
    } while (Display->nextPage());
}

void PongGame::EndGameAnimation()
{
    Display->firstPage();
    do
    {
        Display->setCursor(1, 32);
        Display->setFont(u8g2_font_fub14_tr);
        Display->print("GAME OVER");
        if (score > highScore)
        {
            Display->setFont(u8g2_font_4x6_mf);
            Display->drawStr(27, 60, "New high score!!");
        }
    } while (Display->nextPage());

    if (score > highScore)
    {
        highScore = score;
        EEPROM.put(highScoreMemPos, highScore);
        EEPROM.commit();
    }

    delay(3000);
    WelcomeScreen();
    score = 0;
}

bool PongGame::RunFrame()
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

    Display->drawRFrame(arkaX - arkaLength / 2, arkaY, arkaLength, arkaWidth, 2);

    int ballX = CalculateBallX(delta);
    int ballY = CalculateBallY(delta);

    if ((ballY + ballDiameter / 2) >= arkaY && (ballX - ballDiameter / 2) > (arkaX - arkaLength / 2) && (ballX - ballDiameter / 2) < (arkaX + arkaLength / 2))
    {
        //collition detected
        DIRECTION_Y = UP;
        score++;
    }

    if ((ballY + ballDiameter / 2) >= screenHeight)
    {
        //end game
        return true;
    }

    DrawFrame();
    lastFrameMillis = currentMillis;
    return false;
}

int PongGame::CalculateBallX(unsigned long delta)
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
    else if (DIRECTION_X == LEFT)
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

int PongGame::CalculateBallY(unsigned long delta)
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

    else if (DIRECTION_Y == DOWN)
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

char *PongGame::GetName() { return (char *)"Pong"; }
