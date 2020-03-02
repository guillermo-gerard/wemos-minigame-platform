#include "JuegoTest.h"

JuegoTest::JuegoTest(int screenWidth, int screenHeight, int highScoreMemPos,
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
}

void JuegoTest::WelcomeScreen()
{
    EEPROM.get(highScoreMemPos, highScore);

    Display->firstPage();
    do
    {
        Display->setCursor(4, 22);
        Display->setFont(u8g2_font_8x13B_mr);
        Display->print("Juego test");
        Display->setFont(u8g2_font_4x6_mf);
        Display->drawStr(10, 45, "Press start button to begin");
    } while (Display->nextPage());
}

void JuegoTest::CountBackwardsAndStart(int seconds)
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

void JuegoTest::ShowStart(void)
{
    Display->firstPage();
    do
    {
        Display->setCursor(15, 32);
        Display->setFont(u8g2_font_fub20_tr);
        Display->print("START!");
    } while (Display->nextPage());
    delay(1000);
}

void JuegoTest::Start(void)
{
    lastFrameMillis = millis();
}

void JuegoTest::EndGameAnimation()
{
    Display->firstPage();
    do
    {
        Display->setCursor(1, 32);
        Display->setFont(u8g2_font_fub14_tr);
        Display->print("GAME OVER");
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

bool JuegoTest::RunFrame()
{
    Display->setFont(u8g2_font_8x13B_mr);
    Display->firstPage();
    do
    {
        Display->drawStr(1, 6, (String("Score: ") + String(score)).c_str());
        Display->drawStr(90, 6, (String("High: ") + String(highScore)).c_str());
        Display->drawStr(10, 60, "Jugando");

    } while (Display->nextPage());

    delay(3000);
    return true;
}

char *JuegoTest::GetName() { return (char *)"Test"; }
