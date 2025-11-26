#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>
#include "Ball.h"

class Score {
public:
    Score(int windowW, int windowH, float topH, float bottomH, float leftW, float rightW);

    void loadAssets(sf::Font& fontRef);
    
    void render(sf::RenderWindow& window, 
                int currentPlayer, 
                BallGroup groupP1, 
                BallGroup groupP2, 
                const std::vector<Ball>& objectBalls,
                const std::vector<int>& pocketedOrder,
                std::map<int, sf::Texture>& ballTextures,
                const std::string& statusMessage,
                sf::Color statusColor);

private:
    void drawMiniBall(sf::RenderWindow& window, float x, float y, int number, std::map<int, sf::Texture>& textures);
    std::string getGroupStr(BallGroup group) const;
    bool isBallPocketed(int number, const std::vector<Ball>& objectBalls);

    int windowWidth, windowHeight;
    float topUIHeight, bottomUIHeight;
    float leftPanelWidth, rightPanelWidth;

    sf::RectangleShape uiBackground; 
    sf::RectangleShape topPanel;     
    sf::RectangleShape bottomPanel;  
    sf::RectangleShape leftPanelBg;
    sf::RectangleShape rightPanelBg;
    sf::RectangleShape railGroove; 

    sf::RectangleShape p1Status, p2Status;
    sf::Text p1Text, p2Text;
    sf::Text statusText;
    sf::Font* font; 

    sf::Texture logoTexture;
    sf::Sprite logoSprite;
    
    sf::Texture p1AvatarTexture;
    sf::Sprite p1AvatarSprite;

    sf::Texture p2AvatarTexture;
    sf::Sprite p2AvatarSprite;
};