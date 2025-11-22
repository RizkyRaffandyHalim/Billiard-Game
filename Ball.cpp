#include "Ball.h"
#define _USE_MATH_DEFINES
#include <cmath>

Ball::Ball(float radius, sf::Vector2f pos, sf::Color color, int ballNumber, bool isStripe)
    : ballNumber(ballNumber), isPocketed(false)
{
    (void)isStripe; 

    shape.setRadius(radius);
    shape.setOrigin(radius, radius);
    shape.setPosition(pos);
    shape.setFillColor(color);

    // shading
    shading.setRadius(radius * 0.3f);
    shading.setOrigin(shading.getRadius(), shading.getRadius());
    shading.setFillColor(sf::Color(255, 255, 255, 100));
}

void Ball::resetPosition(sf::Vector2f pos) {
    shape.setPosition(pos);
    physics.velocity = {0.f, 0.f};
    isPocketed = false;
}

void Ball::update(float dt, float tableLeft, float tableTop, float tableRight, float tableBottom)
{
    if (isPocketed) return;

    shape.move(physics.velocity * dt);

    // Update shading
    shading.setPosition(
        shape.getPosition().x - shape.getRadius() * 0.4f,
        shape.getPosition().y - shape.getRadius() * 0.4f
    );

    // Gesekan dipanggil dari Fisika
    physics.applyFriction();

    float r = shape.getRadius();
    sf::Vector2f pos = shape.getPosition();

    // Pantulan batas meja (memodifikasi physics.velocity)
    if (pos.x - r < tableLeft)  { shape.setPosition(tableLeft + r, pos.y);      physics.velocity.x = -physics.velocity.x; }
    if (pos.x + r > tableRight) { shape.setPosition(tableRight - r, pos.y);     physics.velocity.x = -physics.velocity.x; }
    if (pos.y - r < tableTop)   { shape.setPosition(pos.x, tableTop + r);       physics.velocity.y = -physics.velocity.y; }
    if (pos.y + r > tableBottom){ shape.setPosition(pos.x, tableBottom - r);    physics.velocity.y = -physics.velocity.y; }
}

void Ball::draw(sf::RenderWindow& window, sf::Font& font) {
    if (isPocketed) return;

    window.draw(shape);
    window.draw(shading);

    if (ballNumber > 0) {
        sf::Text text;
        text.setFont(font);
        text.setString(std::to_string(ballNumber));
        text.setCharacterSize(int(shape.getRadius() * 1.2f));
        text.setStyle(sf::Text::Bold);

        if (ballNumber == 8) 
            text.setFillColor(sf::Color::White);
        else
            text.setFillColor(sf::Color::Black);

        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(
            textRect.left + textRect.width / 2.0f,
            textRect.top + textRect.height / 2.0f
        );
        text.setPosition(shape.getPosition());
        window.draw(text);
    }
}