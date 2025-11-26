#pragma once
#include "Ball.h"
#include "CueStick.h"

class CueBall : public Ball {
public:
    CueStick cueStick; 

    CueBall(float radius, sf::Vector2f pos, sf::Color color);

    void resetPosition(sf::Vector2f pos) override;
    void updateCue(sf::RenderWindow& window, bool canAim);
    void draw(sf::RenderWindow& window, sf::Font& font, sf::Texture* texture = nullptr) override;
};