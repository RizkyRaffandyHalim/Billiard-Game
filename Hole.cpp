#include "Hole.h"
#include <cmath>

Hole::Hole(float x,float y,float r): radius(r){
    shape.setRadius(radius);
    shape.setOrigin(radius,radius);
    shape.setPosition(x,y);
    shape.setFillColor(sf::Color::Black);
}

bool Hole::checkBallInHole(Ball& ball){
    if(ball.isPocketed) return false;

    sf::Vector2f diff = ball.shape.getPosition() - shape.getPosition();
    float dist = std::sqrt(diff.x*diff.x + diff.y*diff.y);

    if(dist < radius){ 
        ball.isPocketed = true;
        return true;
    }
    return false;
}

void Hole::draw(sf::RenderWindow& window){
    window.draw(shape);
}