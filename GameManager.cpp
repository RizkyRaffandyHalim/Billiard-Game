#include "GameManager.h"
#include <cmath>
#include <iostream>

GameManager::GameManager(int windowW, int windowH, float border)
: windowHeight(windowH), borderThickness(border),
  table(windowW, windowH, border), 
  cueBall(12.f, {0.f, 0.f}, sf::Color::White),
  score(windowW + 80 + 100, windowH, 100.f, 60.f, 80.f, 100.f) 
{
    int baseWindowWidth = windowW;
    float topUIHeight = 100.f;
    float bottomUIHeight = 60.f;
    float leftPanelWidth = 80.f;
    float rightPanelWidth = 100.f;

    windowWidth = baseWindowWidth + (int)leftPanelWidth + (int)rightPanelWidth;
    window.create(sf::VideoMode(windowWidth, windowHeight + topUIHeight + bottomUIHeight), "Simulation Billiard Game");
    window.setFramerateLimit(60);

    // Batas Pantulan Bola
    float cushionOffset = 14.f; 

    tableLeft   = borderThickness + leftPanelWidth + cushionOffset;
    tableTop    = borderThickness + topUIHeight + cushionOffset; 
    tableRight  = (float)windowWidth - borderThickness - rightPanelWidth - cushionOffset;
    tableBottom = ((float)windowHeight + topUIHeight) - borderThickness - cushionOffset;

    float visualTableLeft = borderThickness + leftPanelWidth;
    float visualTableTop = borderThickness + topUIHeight;
    float playAreaWidth = ((float)windowWidth - borderThickness - rightPanelWidth) - visualTableLeft;
    float playAreaHeight = (((float)windowHeight + topUIHeight) - borderThickness) - visualTableTop;
    
    table.felt.setPosition(visualTableLeft, visualTableTop);
    table.felt.setSize(sf::Vector2f(playAreaWidth, playAreaHeight));

    // Warna cushion
    table.setupCushions(cushionOffset, sf::Color(0, 30, 50));
    
    // Init State
    currentGameState = BREAK;
    currentPlayer = 1;
    shotMade = false;
    cueBallPocketed = false;
    isFoul = false;
    firstBallHitThisShot = false;
    ballHitRailAfterContact = false;
    firstObjectBallHit = -1;
    isDragging = false;

    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error loading font." << std::endl;
    }

    // Load Assets untuk UI
    score.loadAssets(font);

    // LOAD BALL TEXTURES
    for (int i = 0; i <= 15; i++) {
        std::string filename = "assets/Ball" + std::to_string(i) + ".png";
        sf::Texture tex;
        if (!tex.loadFromFile(filename)) {
            sf::Image img; img.create(64, 64, sf::Color::White);
            tex.loadFromImage(img);
        }
        tex.setSmooth(true);
        ballTextures[i] = tex;
    }

    createRack(tableLeft, tableTop, tableRight, tableBottom);
    
    //Hole Positions
    float holeOffset = 0.f;
    float centerOffset = -10.f;
    float holeRadius = 28.f;

    holes = {
        {tableLeft  + holeOffset, tableTop    + holeOffset, holeRadius}, 
        {tableRight - holeOffset, tableTop    + holeOffset, holeRadius},
        {tableLeft  + holeOffset, tableBottom - holeOffset, holeRadius}, 
        {tableRight - holeOffset, tableBottom - holeOffset, holeRadius},

        {(tableLeft + tableRight) / 2.f, tableTop    + centerOffset, holeRadius},
        {(tableLeft + tableRight) / 2.f, tableBottom - centerOffset, holeRadius}
    };

    playerGroup[1] = NONE;
    playerGroup[2] = NONE;
}

BallGroup GameManager::getBallGroup(int ballNumber) const {
    if (ballNumber == 0) return CUE_BALL;
    if (ballNumber == 8) return EIGHT_BALL;
    if (ballNumber >= 1 && ballNumber <= 7) return SOLID;
    if (ballNumber >= 9 && ballNumber <= 15) return STRIPES;
    return NONE;
}

bool GameManager::allBallsStopped() const {
    if (cueBall.isMoving()) return false;
    for (const auto& b : objectBalls)
        if (!b.isPocketed && b.isMoving()) return false;
    return true;
}

void GameManager::createRack(float tableLeft, float tableTop, float tableRight, float tableBottom) {
    objectBalls.clear();
    pocketedOrder.clear(); 

    float radius = 12.f;
    float centerX = (tableLeft + tableRight) / 2.f;
    float centerY = (tableTop + tableBottom) / 2.f;

    sf::Vector2f cuePos(centerX - 250.f, centerY);
    cueBall.resetPosition(cuePos);
    cueBall.ballNumber = 0;

    sf::Vector2f apex(centerX + 200.f, centerY);
    float spacing = radius * 2.f * 0.87f;

    std::vector<int> ballOrder = { 1, 9, 8, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15 };
    int ballIndex = 0;

    for (int row = 0; row < 5; ++row) {
        float rowX = apex.x + row * spacing;
        float yOffset = (row * spacing) / 2.f;
        for (int col = 0; col <= row; ++col) {
            int ballNumber = ballOrder[ballIndex++];
            objectBalls.emplace_back(radius, sf::Vector2f(rowX, apex.y - yOffset + col * spacing), sf::Color::White, ballNumber, (ballNumber >= 9 && ballNumber <= 15));
        }
    }
}

void GameManager::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();

        // Reset Debug
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
            createRack(tableLeft, tableTop, tableRight, tableBottom);
            currentGameState = BREAK;
            currentPlayer = 1;
            playerGroup[1] = NONE; playerGroup[2] = NONE;
            firstBallHitThisShot = false; ballHitRailAfterContact = false; firstObjectBallHit = -1;
        }

        // Mouse Press
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mp = window.mapPixelToCoords(pixelPos);

            if (currentGameState == BALL_IN_HAND) {
                if (mp.x > tableLeft + 5.f && mp.x < tableRight - 5.f &&
                    mp.y > tableTop + 5.f && mp.y < tableBottom - 5.f) {
                    cueBall.resetPosition(mp);
                    currentGameState = (playerGroup[currentPlayer] == SOLID) ? SOLID_TURN : STRIPES_TURN;
                    if (playerGroup[currentPlayer] == NONE) currentGameState = OPEN_TABLE;
                    
                    firstBallHitThisShot = false; ballHitRailAfterContact = false; firstObjectBallHit = -1;
                }
            } else if (allBallsStopped() && currentGameState != GAME_OVER) {
                sf::Vector2f diff = mp - cueBall.shape.getPosition();
                if (std::sqrt(diff.x * diff.x + diff.y * diff.y) < 30.f) {
                    isDragging = true;
                    cueBall.cueStick.startDrag(mp); 
                }
            }
        }

        // Mouse Release
        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            if (isDragging && allBallsStopped() && currentGameState != BALL_IN_HAND && currentGameState != GAME_OVER) {
                float power = cueBall.cueStick.getPower();
                sf::Vector2f dir = cueBall.cueStick.getShootDirection();

                if (power > 0) {
                    cueBall.physics.velocity = dir * power;
                    shotMade = true;
                    cueBallPocketed = false;
                    pocketedObjBallsThisTurn.clear();
                    isFoul = false; firstBallHitThisShot = false; ballHitRailAfterContact = false; firstObjectBallHit = -1;
                }
                isDragging = false;
                cueBall.cueStick.endDrag();
            }
        }
    }
}

void GameManager::update(float dt) {
    bool canAim = allBallsStopped();

    // PHYSICS
    
    // Cek Rail untuk Cueball
    if (!cueBall.isPocketed && cueBall.isMoving()) {
        sf::Vector2f pos = cueBall.shape.getPosition();
        float r = cueBall.shape.getRadius();
        if (pos.x - r <= tableLeft || pos.x + r >= tableRight || pos.y - r <= tableTop || pos.y + r >= tableBottom) {
            if (firstBallHitThisShot) ballHitRailAfterContact = true;
        }
    }
    cueBall.update(dt, tableLeft, tableTop, tableRight, tableBottom);

    // Cek Rail untuk Object Balls
    for (auto &b : objectBalls) {
        if (!b.isPocketed && b.isMoving()) {
            sf::Vector2f pos = b.shape.getPosition();
            float r = b.shape.getRadius();
            if (pos.x - r <= tableLeft || pos.x + r >= tableRight || pos.y - r <= tableTop || pos.y + r >= tableBottom) {
                if (firstBallHitThisShot) ballHitRailAfterContact = true;
            }
        }
        b.update(dt, tableLeft, tableTop, tableRight, tableBottom);
    }

    // Stick Update
    bool showCue = canAim && currentGameState != BALL_IN_HAND && currentGameState != GAME_OVER;
    cueBall.updateCue(window, showCue, objectBalls); 

    // COLLISION CHECKING
    for (auto &b : objectBalls) {
        Ball::resolveCollision(cueBall, b, firstBallHitThisShot, firstObjectBallHit);
    }
    for (size_t i = 0; i < objectBalls.size(); ++i) {
        for (size_t j = i + 1; j < objectBalls.size(); ++j)
            Ball::resolveCollision(objectBalls[i], objectBalls[j], firstBallHitThisShot, firstObjectBallHit);
    }

    // POCKET CHECKING
    for (auto &h : holes) {
        if (h.checkBallInHole(cueBall)) cueBallPocketed = true;
        for (auto &b : objectBalls) {
            if (!b.isPocketed && h.checkBallInHole(b)) {
                pocketedObjBallsThisTurn.push_back(b.ballNumber);
                pocketedOrder.push_back(b.ballNumber);
            }
        }
    }

    // TURN RULES
    if (shotMade && canAim) {
        handleTurnEnd();
        shotMade = false;
    }

    // Reposisi bola putih jika masuk lubang (selain game over)
    if (cueBall.isPocketed && currentGameState != GAME_OVER) {
        float centerX = (tableLeft + tableRight) / 2.f;
        float centerY = (tableTop + tableBottom) / 2.f;
        cueBall.resetPosition(sf::Vector2f(centerX - 100.f, centerY));
        cueBall.physics.velocity = {0.f, 0.f};
    }
}

void GameManager::handleTurnEnd() {
    BallGroup currentGroup = playerGroup[currentPlayer];
    bool pocketedMyBall = false;
    bool pocketed8Ball = false;

    // Foul Detection
    if (cueBallPocketed) isFoul = true;
    if (!isFoul && !firstBallHitThisShot) isFoul = true;
    if (!isFoul && currentGroup != NONE) {
        BallGroup firstHitGroup = getBallGroup(firstObjectBallHit);
        if (firstHitGroup != currentGroup && firstHitGroup != EIGHT_BALL) isFoul = true;
    }
    if (!isFoul && pocketedObjBallsThisTurn.empty() && !ballHitRailAfterContact) isFoul = true;

    // Check Pocketed
    for (int number : pocketedObjBallsThisTurn) {
        if (number == 8) { pocketed8Ball = true; break; }
    }

    // 8-Ball Logic
    if (pocketed8Ball) {
        if (currentGameState == OPEN_TABLE || currentGameState == BREAK) {
             currentGameState = GAME_OVER;
             currentPlayer = (currentPlayer == 1) ? 2 : 1; 
             return;
        }
        int myRemaining = 0;
        BallGroup myGroup = playerGroup[currentPlayer];
        for(const auto& b : objectBalls) {
            BallGroup bg = getBallGroup(b.ballNumber);
            if(myGroup != NONE && bg == myGroup && !b.isPocketed) myRemaining++;
        }
        if (myRemaining == 0 && !isFoul) currentGameState = GAME_OVER;
        else {
            currentGameState = GAME_OVER;
            currentPlayer = (currentPlayer == 1) ? 2 : 1;
        }
        return; 
    }

    // Break & Open Table Logic
    if (currentGameState == BREAK) {
        currentGameState = OPEN_TABLE;
        if (!pocketedObjBallsThisTurn.empty() && !isFoul) pocketedMyBall = true; 
    }

    if (currentGameState == OPEN_TABLE) { 
        if (!pocketedObjBallsThisTurn.empty() && !isFoul) {
            bool sunkSolid = false, sunkStripe = false;
            for (int number : pocketedObjBallsThisTurn) {
                BallGroup bg = getBallGroup(number);
                if (bg == SOLID) sunkSolid = true;
                if (bg == STRIPES) sunkStripe = true;
            }
            if (sunkSolid && sunkStripe) { pocketedMyBall = true; } 
            else if (sunkSolid) {
                playerGroup[currentPlayer] = SOLID;
                playerGroup[(currentPlayer == 1) ? 2 : 1] = STRIPES;
                currentGameState = (currentPlayer == 1) ? SOLID_TURN : STRIPES_TURN;
                pocketedMyBall = true;
            }
            else if (sunkStripe) {
                playerGroup[currentPlayer] = STRIPES;
                playerGroup[(currentPlayer == 1) ? 2 : 1] = SOLID;
                currentGameState = (currentPlayer == 1) ? STRIPES_TURN : SOLID_TURN;
                pocketedMyBall = true;
            }
        }
    }

    // Closed Table Logic
    if (currentGameState == SOLID_TURN || currentGameState == STRIPES_TURN) {
        for (int number : pocketedObjBallsThisTurn) {
            if (getBallGroup(number) == currentGroup) { pocketedMyBall = true; break; }
        }
    }

    // Switch Player
    if (currentGameState != GAME_OVER) {
        if (isFoul) {
            currentPlayer = (currentPlayer == 1) ? 2 : 1;
            currentGameState = BALL_IN_HAND;
        } else if (!pocketedMyBall) {
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

std::string GameManager::getStatusMessage(sf::Color& outColor) {
    std::string msg;
    if (currentGameState == GAME_OVER) {
        msg = "GAME OVER! Player " + std::to_string(currentPlayer) + " MENANG!";
        outColor = sf::Color::Yellow;
    } else if (currentGameState == BALL_IN_HAND) {
        msg = "FOUL! BALL IN HAND ";
        int foulerId = (currentPlayer == 1) ? 2 : 1;
        BallGroup foulerGroup = playerGroup[foulerId];

        if (cueBallPocketed) msg += "(Sebab: Scratch / Bola Putih Masuk Lubang)";
        else if (!firstBallHitThisShot) msg += "(Sebab: Tidak Mengenai Bola Apapun)";
        else if (foulerGroup != NONE && firstObjectBallHit != -1) {
            BallGroup hitGroup = getBallGroup(firstObjectBallHit);
            if (hitGroup != foulerGroup && hitGroup != EIGHT_BALL) msg += "(Sebab: Mengenai Bola Lawan Terlebih Dahulu)";
        } 
        if (msg.find("Sebab") == std::string::npos && pocketedObjBallsThisTurn.empty() && !ballHitRailAfterContact) {
            msg += "(Sebab: Tidak Ada Bola Masuk & Tidak Ada Bola Sentuh Rail)";
        }
        outColor = sf::Color(255, 100, 100);
    } else {
        msg = "GILIRAN: PLAYER " + std::to_string(currentPlayer);
        outColor = sf::Color::White;
    }
    return msg;
}

void GameManager::render() {
    window.clear(sf::Color::Black);
    
    // 1. Prepare Status Message
    sf::Color statusColor;
    std::string statusMsg = getStatusMessage(statusColor);

    // 2. Render UI (Delegated to Score class)
    score.render(window, currentPlayer, playerGroup[1], playerGroup[2], 
                 objectBalls, pocketedOrder, ballTextures, statusMsg, statusColor);

    // 3. Render Game Board
    table.draw(window);
    for (auto &h : holes) h.draw(window);
    
    // Draw Balls
    for (auto &b : objectBalls) b.draw(window, font, &ballTextures[b.ballNumber]); 
    cueBall.draw(window, font, &ballTextures[0]);

    window.display();
}

void GameManager::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        processEvents();
        float dt = clock.restart().asSeconds();
        update(dt);
        render();
    }
}