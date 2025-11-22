#include "CueBall.h"

CueBall::CueBall(float radius, sf::Vector2f pos, sf::Color color)
    : Ball(radius, pos, color, 0, false), 
      cueStick() 
{
}

void CueBall::resetPosition(sf::Vector2f pos) {
    Ball::resetPosition(pos);
    cueStick.reset();
}

void CueBall::updateCue(sf::RenderWindow& window, bool canAim) {
    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);
    cueStick.update(shape.getPosition(), mousePos, canAim);
    
    if(cueStick.getIsDragging()) {
        cueStick.updateDrag(mousePos);
    }
}

void CueBall::draw(sf::RenderWindow& window, sf::Font& font) {
    Ball::draw(window, font);
    cueStick.draw(window);
}