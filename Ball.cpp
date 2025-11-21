#include "Ball.h"
#define _USE_MATH_DEFINES
#include <cmath>

//   Konstruktor Ball
Ball::Ball(float radius, sf::Vector2f pos, sf::Color color, int ballNumber, bool isStripe)
    : ballNumber(ballNumber), isPocketed(false), mass(1.f)
{
    (void)isStripe; 

    shape.setRadius(radius);
    shape.setOrigin(radius, radius);
    shape.setPosition(pos);
    shape.setFillColor(color);
    velocity = {0.f, 0.f};

    // shading
    shading.setRadius(radius * 0.3f);
    shading.setOrigin(shading.getRadius(), shading.getRadius());
    shading.setFillColor(sf::Color(255, 255, 255, 100));
}

//   Reset Posisi
void Ball::resetPosition(sf::Vector2f pos) {
    shape.setPosition(pos);
    velocity = {0.f, 0.f};
    isPocketed = false;
}

//       Update Logic
void Ball::update(float dt, const sf::RenderWindow& window,
                  float tableLeft, float tableTop,
                  float tableRight, float tableBottom)
{
    if (isPocketed) return;

    // Gerak bola
    shape.move(velocity * dt);

    // Update shading
    shading.setPosition(
        shape.getPosition().x - shape.getRadius() * 0.4f,
        shape.getPosition().y - shape.getRadius() * 0.4f
    );

    // Gesekan
    velocity *= 0.99f;
    if (std::abs(velocity.x) < 0.05f) velocity.x = 0;
    if (std::abs(velocity.y) < 0.05f) velocity.y = 0;

    float r = shape.getRadius();
    sf::Vector2f pos = shape.getPosition();

    // Pantulan batas meja
    if (pos.x - r < tableLeft)  { shape.setPosition(tableLeft + r, pos.y);      velocity.x = -velocity.x; }
    if (pos.x + r > tableRight) { shape.setPosition(tableRight - r, pos.y);     velocity.x = -velocity.x; }
    if (pos.y - r < tableTop)   { shape.setPosition(pos.x, tableTop + r);       velocity.y = -velocity.y; }
    if (pos.y + r > tableBottom){ shape.setPosition(pos.x, tableBottom - r);    velocity.y = -velocity.y; }
}

//       Draw Ball
void Ball::draw(sf::RenderWindow& window, sf::Font& font) {
    if (isPocketed) return;

    window.draw(shape);
    window.draw(shading);

    // Nomor bola
    if (ballNumber > 0) {
        sf::Text text;
        text.setFont(font);
        text.setString(std::to_string(ballNumber));

        text.setCharacterSize(int(shape.getRadius() * 1.2f));
        text.setStyle(sf::Text::Bold);

        // Warna angka
        if (ballNumber == 8) 
            text.setFillColor(sf::Color::White);
        else
            text.setFillColor(sf::Color::Black);

        // Centering text
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(
            textRect.left + textRect.width / 2.0f,
            textRect.top + textRect.height / 2.0f
        );
        text.setPosition(shape.getPosition());

        window.draw(text);
    }
}
