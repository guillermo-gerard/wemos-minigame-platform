#ifndef IGame_h
#define IGame_h

#include "Arduino.h"

// This astract class is the interface for all the games to be created,
// respect the contract and you'll have a functional game
class IGame
{
    // pure virtual functions because this is an abstract class
public:
    //Show the welcome screen (typically, this shows the name of the game along with instructions on how to play)
    virtual void WelcomeScreen(void) = 0;
    //Draw a frame of the game
    virtual bool RunFrame() = 0;
    //Show the "Game over message"
    virtual void EndGameAnimation(void) = 0;
    //Maybe this can be moved to a Base class, I don't see any reason for this to be different for each game
    virtual void CountBackwardsAndStart(int) = 0;

private:
};

#endif
