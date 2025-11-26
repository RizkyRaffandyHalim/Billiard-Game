#include "Score.h"
#include <iostream>

Score::Score(int windowW, int windowH, float topH, float bottomH, float leftW, float rightW)
    : windowWidth(windowW), windowHeight(windowH), 
      topUIHeight(topH), bottomUIHeight(bottomH), 
      leftPanelWidth(leftW), rightPanelWidth(rightW)
{
    // SETUP SHAPES UI
    uiBackground.setSize(sf::Vector2f((float)windowWidth, (float)(windowHeight + topUIHeight + bottomUIHeight)));
    uiBackground.setFillColor(sf::Color(30, 30, 30)); 

    topPanel.setSize(sf::Vector2f((float)windowWidth, topUIHeight));
    topPanel.setFillColor(sf::Color(25, 25, 25)); 
    topPanel.setPosition(0, 0);
    topPanel.setOutlineThickness(1.f);
    topPanel.setOutlineColor(sf::Color(60, 60, 60));

    bottomPanel.setSize(sf::Vector2f((float)windowWidth, bottomUIHeight));
    bottomPanel.setFillColor(sf::Color(25, 25, 25));
    bottomPanel.setPosition(0, windowHeight + topUIHeight); 
    bottomPanel.setOutlineThickness(1.f);
    bottomPanel.setOutlineColor(sf::Color(60, 60, 60));

    leftPanelBg.setSize(sf::Vector2f(leftPanelWidth, (float)windowHeight));
    leftPanelBg.setPosition(0, topUIHeight);
    leftPanelBg.setFillColor(sf::Color(40, 30, 20)); 
    leftPanelBg.setOutlineThickness(1.f);
    leftPanelBg.setOutlineColor(sf::Color(20, 10, 5));

    rightPanelBg.setSize(sf::Vector2f(rightPanelWidth, (float)windowHeight));
    rightPanelBg.setPosition(windowWidth - rightPanelWidth, topUIHeight);
    rightPanelBg.setFillColor(sf::Color(40, 30, 20)); 
    rightPanelBg.setOutlineThickness(1.f);
    rightPanelBg.setOutlineColor(sf::Color(20, 10, 5));

    float grooveWidth = 40.f;
    railGroove.setSize(sf::Vector2f(grooveWidth, (float)windowHeight - 40.f));
    railGroove.setOrigin(grooveWidth/2, 0);
    railGroove.setPosition(windowWidth - rightPanelWidth/2, topUIHeight + 20.f);
    railGroove.setFillColor(sf::Color(10, 10, 10)); 
    railGroove.setOutlineThickness(2.f);
    railGroove.setOutlineColor(sf::Color(60, 60, 60));

    // Player Status Boxes
    p1Status.setSize(sf::Vector2f(120.f, 30.f));
    p2Status.setSize(sf::Vector2f(120.f, 30.f));
    float panelY = windowHeight + topUIHeight + 15.f;
    p2Status.setPosition(windowWidth - 140.f, panelY);
    p1Status.setPosition(windowWidth - 280.f, panelY);
}

void Score::loadAssets(sf::Font& fontRef) {
    font = &fontRef;

    if (!logoTexture.loadFromFile("assets/Logo.png")) {}
    logoTexture.setSmooth(true);
    logoSprite.setTexture(logoTexture);

    if (!p1AvatarTexture.loadFromFile("assets/Avatar1.png")) {}
    p1AvatarTexture.setSmooth(true);
    p1AvatarSprite.setTexture(p1AvatarTexture);

    if (!p2AvatarTexture.loadFromFile("assets/Avatar2.png")) {}
    p2AvatarTexture.setSmooth(true);
    p2AvatarSprite.setTexture(p2AvatarTexture);

    statusText.setFont(*font);
    statusText.setCharacterSize(20); 
    statusText.setFillColor(sf::Color::White);
    statusText.setPosition(20.f, windowHeight + topUIHeight + 18.f);
}

std::string Score::getGroupStr(BallGroup group) const {
    if (group == SOLID) return "SOLID";
    if (group == STRIPES) return "STRIPES";
    if (group == EIGHT_BALL) return "8-BALL";
    return "NONE";
}

bool Score::isBallPocketed(int number, const std::vector<Ball>& objectBalls) {
    for (const auto& b : objectBalls) {
        if (b.ballNumber == number) return b.isPocketed;
    }
    return true; 
}

void Score::drawMiniBall(sf::RenderWindow& window, float x, float y, int number, std::map<int, sf::Texture>& textures) {
    float radius = 15.f; 
    float diameter = radius * 2.f;

    sf::Sprite ballSprite;
    if (textures.find(number) != textures.end()) {
        ballSprite.setTexture(textures[number]);
    }
    
    // Scaling
    sf::Vector2u texSize = ballSprite.getTexture()->getSize();
    float scaleX = diameter / texSize.x;
    float scaleY = diameter / texSize.y;
    ballSprite.setScale(scaleX, scaleY);

    ballSprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
    ballSprite.setPosition(x, y);

    window.draw(ballSprite);
}

void Score::render(sf::RenderWindow& window, int currentPlayer, BallGroup groupP1, BallGroup groupP2, 
                   const std::vector<Ball>& objectBalls, const std::vector<int>& pocketedOrder, 
                   std::map<int, sf::Texture>& ballTextures, const std::string& statusMessage, sf::Color statusColor) 
{
    // Draw Basic Panels
    window.draw(uiBackground); 
    window.draw(bottomPanel); 
    window.draw(leftPanelBg);
    window.draw(rightPanelBg);
    window.draw(railGroove);
    window.draw(topPanel);

    // SIDE PANEL RAIL
    float railCenterX = windowWidth - (rightPanelWidth / 2.f);
    float startY = topUIHeight + (float)windowHeight - 50.f; 
    float spacing = 32.f; 
    int count = 0;
    for (int ballNum : pocketedOrder) {
        float yPos = startY - (count * spacing);
        drawMiniBall(window, railCenterX, yPos, ballNum, ballTextures);
        count++;
    }

    // SCOREBOARD UI
    float centerY = topUIHeight / 2.f; 
    float paddingSide = leftPanelWidth + 20.f; 
    float rightLimit = windowWidth - rightPanelWidth - 20.f;
    float centerX = (paddingSide + rightLimit) / 2.f;

    // Logo
    float desiredH = 70.f;
    float scale = desiredH / logoSprite.getLocalBounds().height;
    logoSprite.setScale(scale, scale);
    sf::FloatRect logoB = logoSprite.getLocalBounds();
    logoSprite.setOrigin(logoB.width / 2.f, logoB.height / 2.f);
    logoSprite.setPosition(centerX, centerY);
    window.draw(logoSprite);

    float startBallX_P1 = centerX - 70.f; 
    float startBallX_P2 = centerX + 70.f; 
    float ballSpacing = 34.f;    

    // PLAYER 1 UI
    float p1AvatarSize = 50.f; 
    float p1Scale = p1AvatarSize / p1AvatarSprite.getLocalBounds().height;
    p1AvatarSprite.setScale(p1Scale, p1Scale);
    p1AvatarSprite.setOrigin(0, p1AvatarSprite.getLocalBounds().height/2.f);
    p1AvatarSprite.setPosition(paddingSide, centerY); 
    window.draw(p1AvatarSprite);

    float p1TextX = paddingSide + p1AvatarSize + 15.f; 
    sf::Text nameP1; nameP1.setFont(*font); nameP1.setString("PLAYER 1"); nameP1.setCharacterSize(24);
    nameP1.setStyle(sf::Text::Bold); nameP1.setFillColor(currentPlayer == 1 ? sf::Color(50, 255, 50) : sf::Color(150, 150, 150));
    nameP1.setOrigin(0, nameP1.getLocalBounds().height / 2.f); nameP1.setPosition(p1TextX, centerY - 12.f); 
    window.draw(nameP1);

    sf::Text grpP1; grpP1.setFont(*font); grpP1.setString(getGroupStr(groupP1)); grpP1.setCharacterSize(14);
    grpP1.setFillColor(sf::Color(200, 200, 200)); grpP1.setPosition(p1TextX, centerY + 15.f); 
    window.draw(grpP1);

    // Bola Tersisa Player 1
    if (groupP1 != NONE) {
        int c = 0;
        int startB = (groupP1 == SOLID) ? 1 : 9;
        int endB = (groupP1 == SOLID) ? 7 : 15;
        for (int i = startB; i <= endB; i++) {
            if (!isBallPocketed(i, objectBalls)) {
                drawMiniBall(window, startBallX_P1 - c * ballSpacing, centerY, i, ballTextures);
                c++;
            }
        }
        if (c == 0 && !isBallPocketed(8, objectBalls)) drawMiniBall(window, startBallX_P1, centerY, 8, ballTextures);
    } else {
        sf::Text openTxt; openTxt.setFont(*font); openTxt.setString("OPEN TABLE");
        openTxt.setCharacterSize(14); openTxt.setStyle(sf::Text::Italic); openTxt.setFillColor(sf::Color(100, 100, 100));
        sf::FloatRect b = openTxt.getLocalBounds(); openTxt.setOrigin(b.width, 0);
        openTxt.setPosition(startBallX_P1, centerY - 8.f); window.draw(openTxt);
    }

    // PLAYER 2 UI
    float p2AvatarSize = 50.f; 
    float p2Scale = p2AvatarSize / p2AvatarSprite.getLocalBounds().height;
    p2AvatarSprite.setScale(p2Scale, p2Scale);
    p2AvatarSprite.setOrigin(p2AvatarSprite.getLocalBounds().width, p2AvatarSprite.getLocalBounds().height/2.f);
    p2AvatarSprite.setPosition(rightLimit, centerY); 
    window.draw(p2AvatarSprite);

    float p2TextX = rightLimit - p2AvatarSize - 15.f;
    sf::Text nameP2; nameP2.setFont(*font); nameP2.setString("PLAYER 2"); nameP2.setCharacterSize(24);
    nameP2.setStyle(sf::Text::Bold); nameP2.setFillColor(currentPlayer == 2 ? sf::Color(50, 255, 50) : sf::Color(150, 150, 150));
    sf::FloatRect bounds2 = nameP2.getLocalBounds(); nameP2.setOrigin(bounds2.width, bounds2.height / 2.f);
    nameP2.setPosition(p2TextX, centerY - 12.f); window.draw(nameP2);

    sf::Text grpP2; grpP2.setFont(*font); grpP2.setString(getGroupStr(groupP2)); grpP2.setCharacterSize(14);
    grpP2.setFillColor(sf::Color(200, 200, 200)); sf::FloatRect bGrp = grpP2.getLocalBounds();
    grpP2.setOrigin(bGrp.width, 0); grpP2.setPosition(p2TextX, centerY + 15.f); window.draw(grpP2);

    // Bola Tersisa Player 2
    if (groupP2 != NONE) {
        int c = 0;
        int startB = (groupP2 == SOLID) ? 1 : 9;
        int endB = (groupP2 == SOLID) ? 7 : 15;
        for (int i = startB; i <= endB; i++) {
            if (!isBallPocketed(i, objectBalls)) {
                drawMiniBall(window, startBallX_P2 + c * ballSpacing, centerY, i, ballTextures);
                c++;
            }
        }
        if (c == 0 && !isBallPocketed(8, objectBalls)) drawMiniBall(window, startBallX_P2, centerY, 8, ballTextures);
    } else {
        sf::Text openTxt; openTxt.setFont(*font); openTxt.setString("OPEN TABLE");
        openTxt.setCharacterSize(14); openTxt.setStyle(sf::Text::Italic); openTxt.setFillColor(sf::Color(100, 100, 100));
        openTxt.setPosition(startBallX_P2, centerY - 8.f); window.draw(openTxt);
    }

    // BOTTOM STATUS PANEL
    statusText.setString(statusMessage);
    statusText.setFillColor(statusColor);
    window.draw(statusText);
    
    sf::Color p1Color = (groupP1 == SOLID) ? sf::Color(255, 155, 00) : (groupP1 == STRIPES ? sf::Color::Blue : sf::Color(100,100,100));
    sf::Color p2Color = (groupP2 == SOLID) ? sf::Color(255, 155, 00) : (groupP2 == STRIPES ? sf::Color::Blue : sf::Color(100,100,100));
    
    p1Status.setFillColor(p1Color);
    p2Status.setFillColor(p2Color);
    window.draw(p1Status);
    window.draw(p2Status);
    
    p1Text.setFont(*font); p1Text.setCharacterSize(14); p1Text.setFillColor(sf::Color::White);
    p1Text.setString("P1: " + getGroupStr(groupP1));
    sf::FloatRect b1 = p1Text.getLocalBounds();
    p1Text.setOrigin(b1.width/2, b1.height/2);
    p1Text.setPosition(p1Status.getPosition().x + p1Status.getSize().x/2, p1Status.getPosition().y + p1Status.getSize().y/2 - 2);
    
    p2Text.setFont(*font); p2Text.setCharacterSize(14); p2Text.setFillColor(sf::Color::White);
    p2Text.setString("P2: " + getGroupStr(groupP2));
    sf::FloatRect b2 = p2Text.getLocalBounds();
    p2Text.setOrigin(b2.width/2, b2.height/2);
    p2Text.setPosition(p2Status.getPosition().x + p2Status.getSize().x/2, p2Status.getPosition().y + p2Status.getSize().y/2 - 2);

    window.draw(p1Text);
    window.draw(p2Text);
}