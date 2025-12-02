#include "CueBall.h"

CueBall::CueBall(float radius, sf::Vector2f pos, sf::Color color)
    : Ball(radius, pos, color, 0, false), 
      cueStick() 
{}

void CueBall::resetPosition(sf::Vector2f pos) {
    Ball::resetPosition(pos);
    cueStick.reset();
}

void CueBall::updateCue(sf::RenderWindow& window, bool canAim, const std::vector<Ball>& objectBalls) {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos);
    
    cueStick.update(shape.getPosition(), mousePos, canAim, objectBalls);

    if(cueStick.getIsDragging()) {
        cueStick.updateDrag(mousePos);
    }
}

void CueBall::draw(sf::RenderWindow& window, sf::Font& font, sf::Texture* texture) {
    Ball::draw(window, font, texture);
    cueStick.draw(window);
}