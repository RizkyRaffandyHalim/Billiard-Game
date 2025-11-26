#pragma once
#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>
#include "Ball.h"
#include "Score.h"
#include "CueBall.h"
#include "Table.h"
#include "Hole.h"

enum GameState {
    BREAK,
    OPEN_TABLE,
    SOLID_TURN,
    STRIPES_TURN,
    BALL_IN_HAND,
    GAME_OVER
};

class GameManager {
public:
    GameManager(int windowW = 1080, int windowH = 600, float border = 50.f);
    void run();

private:
    // Window & Dimensions
    sf::RenderWindow window;
    int windowWidth, windowHeight;
    float tableLeft, tableTop, tableRight, tableBottom;
    float borderThickness;

    // Sub-Systems
    Score score;
    Table table;

    // Entities
    CueBall cueBall;
    std::vector<Ball> objectBalls; 
    std::vector<Hole> holes;
    std::map<int, sf::Texture> ballTextures;

    // Game State
    GameState currentGameState;
    int currentPlayer;
    BallGroup playerGroup[3];

    // Turn Logic Variables
    bool shotMade;
    bool cueBallPocketed;
    std::vector<int> pocketedObjBallsThisTurn;
    std::vector<int> pocketedOrder; 
    bool isFoul;
    bool firstBallHitThisShot;
    bool ballHitRailAfterContact;
    int firstObjectBallHit;
    bool isDragging;
    
    sf::Font font;

    // Helper Methods
    BallGroup getBallGroup(int ballNumber) const;
    bool allBallsStopped() const;
    void createRack(float tableLeft, float tableTop, float tableRight, float tableBottom);

    // Loop Methods
    void processEvents();
    void update(float dt);
    void render();

    // Logic Methods
    void handleTurnEnd();
    std::string getStatusMessage(sf::Color& outColor); 
};

#endif // GAMEMANAGER_H