#include "Ball.h"
#define _USE_MATH_DEFINES
#include <cmath>

Ball::Ball(float radius, sf::Vector2f pos, sf::Color color, int ballNumber, bool isStripe)
    : ballNumber(ballNumber), isPocketed(false)
{
    physics.mass = 1.f;
    physics.velocity = {0.f, 0.f};

    shape.setRadius(radius);
    shape.setOrigin(radius, radius);
    shape.setPosition(pos);
    shape.setFillColor(color);

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

    shading.setPosition(
        shape.getPosition().x - shape.getRadius() * 0.4f,
        shape.getPosition().y - shape.getRadius() * 0.4f
    );

    physics.applyFriction();

    float r = shape.getRadius();
    sf::Vector2f pos = shape.getPosition();

    // Pantulan Dinding
    if (pos.x - r < tableLeft)  { shape.setPosition(tableLeft + r, pos.y);      physics.velocity.x = -physics.velocity.x; }
    if (pos.x + r > tableRight) { shape.setPosition(tableRight - r, pos.y);     physics.velocity.x = -physics.velocity.x; }
    if (pos.y - r < tableTop)   { shape.setPosition(pos.x, tableTop + r);       physics.velocity.y = -physics.velocity.y; }
    if (pos.y + r > tableBottom){ shape.setPosition(pos.x, tableBottom - r);    physics.velocity.y = -physics.velocity.y; }
}

void Ball::draw(sf::RenderWindow& window, sf::Font& font, sf::Texture* texture) {
    if (isPocketed) return;

    if (texture) {
        sf::Sprite sprite;
        sprite.setTexture(*texture);
        
        float diameter = shape.getRadius() * 2.f;
        sf::Vector2u texSize = texture->getSize();
        float scaleX = diameter / texSize.x;
        float scaleY = diameter / texSize.y;
        sprite.setScale(scaleX, scaleY);

        sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        sprite.setPosition(shape.getPosition());
        
        window.draw(sprite);
    } else {
        window.draw(shape);
    }
}

// IMPLEMENTASI LOGIKA TUMBUKAN
bool Ball::resolveCollision(Ball& a, Ball& b, bool& firstBallHitFlag, int& firstObjectHitVal) {
    if (a.isPocketed || b.isPocketed) return false;

    sf::Vector2f diff = b.shape.getPosition() - a.shape.getPosition();
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    float radiusSum = a.shape.getRadius() + b.shape.getRadius();

    if (dist < radiusSum && dist > 0.f) {
        if (a.ballNumber == 0 || b.ballNumber == 0) { 
            firstBallHitFlag = true;
            if (firstObjectHitVal == -1) {
                if (a.ballNumber == 0 && b.ballNumber > 0) firstObjectHitVal = b.ballNumber;
                else if (b.ballNumber == 0 && a.ballNumber > 0) firstObjectHitVal = a.ballNumber;
            }
        }

        // Logika Fisika Elastis
        sf::Vector2f norm = diff / dist;
        sf::Vector2f tangent(-norm.y, norm.x);

        float v1n = a.physics.velocity.x * norm.x + a.physics.velocity.y * norm.y;
        float v1t = a.physics.velocity.x * tangent.x + a.physics.velocity.y * tangent.y;
        float v2n = b.physics.velocity.x * norm.x + b.physics.velocity.y * norm.y;
        float v2t = b.physics.velocity.x * tangent.x + b.physics.velocity.y * tangent.y;

        float m1 = a.physics.mass, m2 = b.physics.mass;
        float v1nAfter = (v1n * (m1 - m2) + 2 * m2 * v2n) / (m1 + m2);
        float v2nAfter = (v2n * (m2 - m1) + 2 * m1 * v1n) / (m1 + m2);

        sf::Vector2f v1nVec = v1nAfter * norm;
        sf::Vector2f v1tVec = v1t * tangent;
        sf::Vector2f v2nVec = v2nAfter * norm;
        sf::Vector2f v2tVec = v2t * tangent;

        a.physics.velocity = v1nVec + v1tVec;
        b.physics.velocity = v2nVec + v2tVec;

        // Koreksi Overlap
        float overlap = radiusSum - dist + 0.1f;
        a.shape.move(-norm * overlap / 2.f);
        b.shape.move(norm * overlap / 2.f);
        
        return true;
    }
    return false;
}