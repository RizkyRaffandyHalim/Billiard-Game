#pragma once
#include <SFML/Graphics.hpp>
#include "Ball.h"

class Hole {
public:
    sf::CircleShape shape;
    float radius;

    Hole(float x, float y, float r);

    bool checkBallInHole(Ball& ball);
    void draw(sf::RenderWindow& window);
};
