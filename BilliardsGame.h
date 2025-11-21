#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Ball.h"
#include "Table.h"
#include "Hole.h"
#include "CueStick.h"

enum GameState {
    BREAK,
    OPEN_TABLE,
    SOLID_TURN,
    STRIPES_TURN,
    BALL_IN_HAND,
    GAME_OVER
};

enum BallGroup {
    NONE,
    SOLID,
    STRIPES,
    EIGHT_BALL,
    CUE_BALL
};

class BilliardsGame {
public:
    BilliardsGame(int windowW = 1080, int windowH = 600, float border = 50.f);
    void run();

private:
    // window & table
    sf::RenderWindow window;
    int windowWidth;
    int windowHeight;
    float borderThickness;
    Table table;
    float tableLeft, tableTop, tableRight, tableBottom;

    // entities
    std::vector<Ball> balls;
    std::vector<Hole> holes;
    CueStick cue;

    // state
    GameState currentGameState;
    int currentPlayer;
    BallGroup playerGroup[3];

    // shot related
    bool shotMade;
    bool cueBallPocketed;
    std::vector<int> pocketedObjBallsThisTurn;
    bool isFoul;

    // new rule variables
    bool firstBallHitThisShot;
    bool ballHitRailAfterContact;
    int firstObjectBallHit;

    // input/aim
    bool isDragging;
    sf::Vector2f dragStartPos;
    float currentCueAngle;

    // UI
    sf::Font font;
    sf::Text statusText;
    sf::RectangleShape p1Status, p2Status;
    sf::Text p1Text, p2Text;

    // helpers
    BallGroup getBallGroup(int ballNumber) const;
    std::string getGroupStr(BallGroup group) const;
    void handleBallCollision(Ball &a, Ball &b);
    bool allBallsStopped(const std::vector<Ball>& balls) const;
    std::vector<Ball> createRack(float tableLeft, float tableTop, float tableRight, float tableBottom);

    // loop helpers
    void processEvents();
    void update(float dt);
    void render();

    // logic helper
    void handleTurnEnd();
};
