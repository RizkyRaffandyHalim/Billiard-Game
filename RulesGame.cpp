#include "RulesGame.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// Constructor
RulesGame::RulesGame(int windowW, int windowH, float border)
: windowWidth(windowW), windowHeight(windowH), borderThickness(border),
  window(sf::VideoMode(windowW, windowH), "Simulation Billiard Game"),
  table(windowW, windowH, borderThickness),
  cueBall(12.f, {0.f, 0.f}, sf::Color::White), // Init CueBall
  currentGameState(BREAK),
  currentPlayer(1),
  shotMade(false),
  cueBallPocketed(false),
  isFoul(false),
  firstBallHitThisShot(false),
  ballHitRailAfterContact(false),
  firstObjectBallHit(-1),
  isDragging(false)
{
    // table bounds
    tableLeft = borderThickness;
    tableTop = borderThickness;
    tableRight = windowWidth - borderThickness;
    tableBottom = windowHeight - borderThickness;

    window.setFramerateLimit(60);

    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error loading font." << std::endl;
    }

    // Setup Rack dan Holes
    createRack(tableLeft, tableTop, tableRight, tableBottom);

    float holeRadius = 28.f;
    holes = {
        {tableLeft, tableTop, holeRadius}, {tableRight, tableTop, holeRadius},
        {tableLeft, tableBottom, holeRadius}, {tableRight, tableBottom, holeRadius},
        {(tableLeft + tableRight) / 2.f, tableTop, holeRadius * 0.8f},
        {(tableLeft + tableRight) / 2.f, tableBottom, holeRadius * 0.8f}
    };

    // UI Setup
    statusText.setFont(font);
    statusText.setCharacterSize(24);
    statusText.setFillColor(sf::Color::White);
    statusText.setPosition(10.f, 10.f);

    p1Status = sf::RectangleShape(sf::Vector2f(150.f, 30.f));
    p1Status.setPosition(50.f, windowHeight - 40.f);
    p1Status.setOutlineThickness(2.f);
    p1Status.setOutlineColor(sf::Color::White);

    p2Status = sf::RectangleShape(sf::Vector2f(150.f, 30.f));
    p2Status.setPosition(windowWidth - 200.f, windowHeight - 40.f);
    p2Status.setOutlineThickness(2.f);
    p2Status.setOutlineColor(sf::Color::White);

    p1Text.setFont(font);
    p1Text.setCharacterSize(18);
    p1Text.setPosition(p1Status.getPosition().x + 5.f, p1Status.getPosition().y + 3.f);

    p2Text.setFont(font);
    p2Text.setCharacterSize(18);
    p2Text.setPosition(p2Status.getPosition().x + 5.f, p2Status.getPosition().y + 3.f);

    playerGroup[1] = NONE;
    playerGroup[2] = NONE;
}

// Helpers
BallGroup RulesGame::getBallGroup(int ballNumber) const {
    if (ballNumber == 0) return CUE_BALL;
    if (ballNumber == 8) return EIGHT_BALL;
    if (ballNumber >= 1 && ballNumber <= 7) return SOLID;
    if (ballNumber >= 9 && ballNumber <= 15) return STRIPES;
    return NONE;
}

std::string RulesGame::getGroupStr(BallGroup group) const {
    if (group == SOLID) return "SOLID";
    if (group == STRIPES) return "STRIPES";
    if (group == EIGHT_BALL) return "8-BALL";
    return "NONE";
}

// Collision
void RulesGame::handleBallCollision(Ball &a, Ball &b) {
    if (a.isPocketed || b.isPocketed) return;

    sf::Vector2f diff = b.shape.getPosition() - a.shape.getPosition();
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    float radiusSum = a.shape.getRadius() + b.shape.getRadius();

    if (dist < radiusSum && dist > 0.f) {
        // LOGIKA RULES
        if (a.ballNumber == 0 || b.ballNumber == 0) { 
            firstBallHitThisShot = true;
            if (firstObjectBallHit == -1) {
                if (a.ballNumber == 0 && b.ballNumber > 0) firstObjectBallHit = b.ballNumber;
                else if (b.ballNumber == 0 && a.ballNumber > 0) firstObjectBallHit = a.ballNumber;
            }
        }

        // PHYSICS RESPONSE
        sf::Vector2f norm = diff / dist;
        sf::Vector2f tangent(-norm.y, norm.x);

        float v1n = a.physics.velocity.x * norm.x + a.physics.velocity.y * norm.y;
        float v1t = a.physics.velocity.x * tangent.x + a.physics.velocity.y * tangent.y;
        float v2n = b.physics.velocity.x * norm.x + b.physics.velocity.y * norm.y;
        float v2t = b.physics.velocity.x * tangent.x + b.physics.velocity.y * tangent.y;

        float m1 = a.physics.mass, m2 = b.physics.mass;
        float v1nAfter = (v1n * (m1 - m2) + 2 * m2 * v2n) / (m1 + m2);
        float v2nAfter = (v2n * (m2 - m1) + 2 * m1 * v1n) / (m1 + m2);

        sf::Vector2f v1nVec = v1nAfter * norm;
        sf::Vector2f v1tVec = v1t * tangent;
        sf::Vector2f v2nVec = v2nAfter * norm;
        sf::Vector2f v2tVec = v2t * tangent;

        a.physics.velocity = v1nVec + v1tVec;
        b.physics.velocity = v2nVec + v2tVec;

        float overlap = radiusSum - dist + 0.1f;
        a.shape.move(-norm * overlap / 2.f);
        b.shape.move(norm * overlap / 2.f);
    }
}

bool RulesGame::allBallsStopped() const {
    if (cueBall.isMoving()) return false;
    for (const auto& b : objectBalls)
        if (!b.isPocketed && b.isMoving()) return false;
    return true;
}

// Create Rack
void RulesGame::createRack(float tableLeft, float tableTop, float tableRight, float tableBottom) {
    objectBalls.clear();
    float radius = 12.f;
    float centerX = (tableLeft + tableRight) / 2.f;
    float centerY = (tableTop + tableBottom) / 2.f;

    // Cue Ball
    sf::Vector2f cuePos(centerX - 250.f, centerY);
    cueBall.resetPosition(cuePos);
    cueBall.ballNumber = 0;

    // Object Balls
    sf::Vector2f apex(centerX + 200.f, centerY);
    float spacing = radius * 2.f * 0.87f;

    std::vector<int> ballOrder = { 1, 9, 8, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15 };
    int ballIndex = 0;

    for (int row = 0; row < 5; ++row) {
        float rowX = apex.x + row * spacing;
        float yOffset = (row * spacing) / 2.f;
        for (int col = 0; col <= row; ++col) {
            int ballNumber = ballOrder[ballIndex++];
            sf::Color ballColor;
            if (ballNumber == 0) continue;

            if (ballNumber == 8) ballColor = sf::Color::Black;
            else if (ballNumber >= 1 && ballNumber <= 7) ballColor = sf::Color(255, 165, 0);
            else ballColor = sf::Color::Blue;

            float y = apex.y - yOffset + col * spacing;
            objectBalls.emplace_back(radius, sf::Vector2f(rowX, y), ballColor, ballNumber, (ballNumber >= 9 && ballNumber <= 15));
        }
    }
}

// Process Events
void RulesGame::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();

        // Reset
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
            createRack(tableLeft, tableTop, tableRight, tableBottom);
            currentGameState = BREAK;
            currentPlayer = 1;
            playerGroup[1] = NONE;
            playerGroup[2] = NONE;
            firstBallHitThisShot = false;
            ballHitRailAfterContact = false;
            firstObjectBallHit = -1;
        }

        // Mouse Input
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mp = (sf::Vector2f)sf::Mouse::getPosition(window);

            if (currentGameState == BALL_IN_HAND) {
                if (mp.x > tableLeft + 5.f && mp.x < tableRight - 5.f &&
                    mp.y > tableTop + 5.f && mp.y < tableBottom - 5.f) {
                    cueBall.resetPosition(mp);
                    currentGameState = (playerGroup[currentPlayer] == SOLID) ? SOLID_TURN : STRIPES_TURN;
                    if (playerGroup[currentPlayer] == NONE) currentGameState = OPEN_TABLE;
                    
                    // Reset pelanggaran saat bola diletakkan kembali
                    firstBallHitThisShot = false;
                    ballHitRailAfterContact = false;
                    firstObjectBallHit = -1;
                }
            } else if (allBallsStopped() && currentGameState != GAME_OVER) {
                // Logic Drag Start
                sf::Vector2f diff = mp - cueBall.shape.getPosition();
                if (std::sqrt(diff.x * diff.x + diff.y * diff.y) < 30.f) {
                    isDragging = true;
                    cueBall.cueStick.startDrag(mp); 
                }
            }
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            if (isDragging && allBallsStopped() && currentGameState != BALL_IN_HAND && currentGameState != GAME_OVER) {
                
                float power = cueBall.cueStick.getPower();
                sf::Vector2f dir = cueBall.cueStick.getShootDirection();

                if (power > 0) {
                    cueBall.physics.velocity = dir * power;
                    shotMade = true;

                    // Reset variable rules SEBELUM tembakan dimulai
                    cueBallPocketed = false;
                    pocketedObjBallsThisTurn.clear();
                    isFoul = false;
                    firstBallHitThisShot = false;
                    ballHitRailAfterContact = false;
                    firstObjectBallHit = -1;
                }

                isDragging = false;
                cueBall.cueStick.endDrag();
            }
        }
    }
}

// Update
void RulesGame::update(float dt) {
    bool canAim = allBallsStopped();

    // 1. Update Physics CueBall
    if (!cueBall.isPocketed && cueBall.isMoving()) {
        sf::Vector2f pos = cueBall.shape.getPosition();
        float r = cueBall.shape.getRadius();
        if (pos.x - r <= tableLeft || pos.x + r >= tableRight ||
            pos.y - r <= tableTop || pos.y + r >= tableBottom) {
            if (firstBallHitThisShot) ballHitRailAfterContact = true;
        }
    }
    cueBall.update(dt, tableLeft, tableTop, tableRight, tableBottom);

    // 2. Update Physics ObjectBalls
    for (auto &b : objectBalls) {
        if (!b.isPocketed && b.isMoving()) {
            sf::Vector2f pos = b.shape.getPosition();
            float r = b.shape.getRadius();
            if (pos.x - r <= tableLeft || pos.x + r >= tableRight ||
                pos.y - r <= tableTop || pos.y + r >= tableBottom) {
                if (firstBallHitThisShot) ballHitRailAfterContact = true;
            }
        }
        b.update(dt, tableLeft, tableTop, tableRight, tableBottom);
    }

    // 3. Update Cue Stick Visuals
    bool showCue = canAim && currentGameState != BALL_IN_HAND && currentGameState != GAME_OVER;
    cueBall.updateCue(window, showCue); 

    // 4. Collisions
    for (auto &b : objectBalls) handleBallCollision(cueBall, b);
    for (size_t i = 0; i < objectBalls.size(); ++i) {
        for (size_t j = i + 1; j < objectBalls.size(); ++j)
            handleBallCollision(objectBalls[i], objectBalls[j]);
    }

    // 5. Pocketing
    for (auto &h : holes) {
        if (h.checkBallInHole(cueBall)) cueBallPocketed = true;
        for (auto &b : objectBalls) {
            if (h.checkBallInHole(b)) pocketedObjBallsThisTurn.push_back(b.ballNumber);
        }
    }

    // 6. Resolve Turn
    if (shotMade && canAim) {
        handleTurnEnd();
        shotMade = false;
    }

    // 7. Cue Ball Reset if Scratch
    if (cueBall.isPocketed && currentGameState != GAME_OVER) {
        float centerX = (tableLeft + tableRight) / 2.f;
        float centerY = (tableTop + tableBottom) / 2.f;
        cueBall.resetPosition(sf::Vector2f(centerX - 100.f, centerY));
        cueBall.physics.velocity = {0.f, 0.f};
    }
}

// Turn End Logic
void RulesGame::handleTurnEnd() {
    BallGroup currentGroup = playerGroup[currentPlayer];
    bool pocketedMyBall = false;
    bool pocketed8Ball = false;

    if (cueBallPocketed) isFoul = true;
    if (!isFoul && !firstBallHitThisShot) isFoul = true;

    if (!isFoul && currentGroup != NONE) {
        BallGroup firstHitGroup = getBallGroup(firstObjectBallHit);
        if (firstHitGroup != currentGroup && firstHitGroup != EIGHT_BALL) isFoul = true;
    }

    if (!isFoul && pocketedObjBallsThisTurn.empty() && !ballHitRailAfterContact) isFoul = true;

    for (int number : pocketedObjBallsThisTurn) {
        if (number == 8) { pocketed8Ball = true; break; }
    }

    if (pocketed8Ball) {
        int myRemaining = 0;
        for(const auto& b : objectBalls) {
            BallGroup bg = getBallGroup(b.ballNumber);
            if(currentGroup != NONE && bg == currentGroup && !b.isPocketed) myRemaining++;
        }

        if (myRemaining == 0 && !isFoul) currentGameState = GAME_OVER;
        else {
            currentGameState = GAME_OVER;
            currentPlayer = (currentPlayer == 1) ? 2 : 1; 
        }
    }

    if (currentGameState == BREAK || currentGameState == OPEN_TABLE) {
        for (int number : pocketedObjBallsThisTurn) {
            BallGroup pocketedGroup = getBallGroup(number);
            if (pocketedGroup == SOLID || pocketedGroup == STRIPES) {
                playerGroup[currentPlayer] = pocketedGroup;
                playerGroup[(currentPlayer == 1) ? 2 : 1] = (pocketedGroup == SOLID) ? STRIPES : SOLID;
                currentGameState = (pocketedGroup == SOLID) ? SOLID_TURN : STRIPES_TURN;
                pocketedMyBall = true;
                break;
            }
        }
    }

    if (currentGameState == SOLID_TURN || currentGameState == STRIPES_TURN) {
        for (int number : pocketedObjBallsThisTurn) {
            if (getBallGroup(number) == currentGroup) { pocketedMyBall = true; break; }
        }
    }

    if (currentGameState != GAME_OVER) {
        if (isFoul) {
            currentPlayer = (currentPlayer == 1) ? 2 : 1;
            currentGameState = BALL_IN_HAND;
        } else if (!pocketedMyBall && !pocketed8Ball) {
            currentPlayer = (currentPlayer == 1) ? 2 : 1;
            if (playerGroup[currentPlayer] == SOLID) currentGameState = SOLID_TURN;
            else if (playerGroup[currentPlayer] == STRIPES) currentGameState = STRIPES_TURN;
            else currentGameState = OPEN_TABLE;
        }
    }

    if (cueBallPocketed) {
        cueBall.physics.velocity = {0.f, 0.f};
        cueBall.isPocketed = false;
    }
}

// Render
void RulesGame::render() {
    window.clear(sf::Color(50, 50, 50));
    table.draw(window);
    for (auto &h : holes) h.draw(window);
    
    for (auto &b : objectBalls) b.draw(window, font);
    cueBall.draw(window, font);

    // UI LOGIC
    std::string statusStr;
    if (currentGameState == GAME_OVER) {
        statusStr = "GAME OVER! Player " + std::to_string(currentPlayer) + " MENANG!";
    } else if (currentGameState == BALL_IN_HAND) {
        statusStr = "Player " + std::to_string(currentPlayer) + ": FOUL! BALL IN HAND. Klik untuk menaruh bola.";
        
        int foulerId = (currentPlayer == 1) ? 2 : 1;
        BallGroup foulerGroup = playerGroup[foulerId];

        if (cueBallPocketed) {
            statusStr += "\n(Foul: Bola Putih Masuk Lubang / Scratch)";
        } else if (!firstBallHitThisShot) {
            statusStr += "\n(Foul: Tidak Mengenai Bola Apapun)";
        } else if (foulerGroup != NONE && firstObjectBallHit != -1) {
            BallGroup hitGroup = getBallGroup(firstObjectBallHit);
            
            if (hitGroup != foulerGroup && hitGroup != EIGHT_BALL) {
                statusStr += "\n(Foul: Menabrak Bola Lawan Terlebih Dahulu)";
            }
        } 
        
        // Cek kondisi foul terakhir
        if (statusStr.find("Foul:") == std::string::npos) {
             if (pocketedObjBallsThisTurn.empty() && !ballHitRailAfterContact) {
                statusStr += "\n(Foul: Tidak Ada Bola Masuk & Tidak Ada Bola Sentuh Rail)";
             }
        }

    } else {
        std::string stateName;
        if (currentGameState == BREAK) stateName = "BREAK";
        else if (currentGameState == OPEN_TABLE) stateName = "OPEN TABLE";
        else if (currentGameState == SOLID_TURN) stateName = "SOLID TURN";
        else if (currentGameState == STRIPES_TURN) stateName = "STRIPES TURN";

        statusStr = "PLAYER " + std::to_string(currentPlayer) + " (" + stateName + ")";
        if (playerGroup[1] != NONE) {
            statusStr += "\nP1: " + getGroupStr(playerGroup[1]) + " | P2: " + getGroupStr(playerGroup[2]);
        }
    }

    statusText.setString(statusStr);
    window.draw(statusText);

    sf::Color p1GroupColor = sf::Color(100, 100, 100);
    if (playerGroup[1] == SOLID) p1GroupColor = sf::Color(255, 165, 0);
    else if (playerGroup[1] == STRIPES) p1GroupColor = sf::Color::Blue;

    sf::Color p2GroupColor = sf::Color(100, 100, 100);
    if (playerGroup[2] == SOLID) p2GroupColor = sf::Color(255, 165, 0);
    else if (playerGroup[2] == STRIPES) p2GroupColor = sf::Color::Blue;

    p1Status.setFillColor((currentPlayer == 1) ? p1GroupColor : sf::Color::Black);
    p2Status.setFillColor((currentPlayer == 2) ? p2GroupColor : sf::Color::Black);

    p1Text.setFillColor((currentPlayer == 1) ? sf::Color::Black : sf::Color::White);
    p2Text.setFillColor((currentPlayer == 2) ? sf::Color::Black : sf::Color::White);

    p1Text.setString("P1: " + getGroupStr(playerGroup[1]));
    p2Text.setString("P2: " + getGroupStr(playerGroup[2]));

    window.draw(p1Status);
    window.draw(p1Text);
    window.draw(p2Status);
    window.draw(p2Text);

    window.display();
}

void RulesGame::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        processEvents();
        float dt = clock.restart().asSeconds();
        update(dt);
        render();
    }
}