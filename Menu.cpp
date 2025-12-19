#include "Menu.h"
#include <iostream>

Menu::Menu(float width, float height) : windowWidth(width), windowHeight(height) {
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font." << std::endl;
    }

    // Load Background
    if (bgTexture.loadFromFile("assets/MenuBackground.png")) {
        bgTexture.setSmooth(true);
        bgSprite.setTexture(bgTexture);
        sf::Vector2u texSize = bgTexture.getSize();
        bgSprite.setScale(windowWidth / (float)texSize.x, windowHeight / (float)texSize.y);
    }

    // Load Logo
    hasLogoImage = false;
    if (logoTexture.loadFromFile("assets/MenuLogo.png")) {
        hasLogoImage = true;
        logoTexture.setSmooth(true);
        logoSprite.setTexture(logoTexture);
        logoSprite.setOrigin(logoSprite.getLocalBounds().width / 2.0f, logoSprite.getLocalBounds().height / 2.0f);
        logoSprite.setPosition(windowWidth / 2.0f, windowHeight / 3.0f);
        float maxLogoWidth = windowWidth * 0.5f;

        if (logoSprite.getGlobalBounds().width > maxLogoWidth) {
            float s = maxLogoWidth / logoSprite.getLocalBounds().width;
            logoSprite.setScale(s, s);
        }
    }

    // Setup Tombol
    float buttonStartY = windowHeight * 0.65f; 
    setupButton(btnStart, "PLAY GAME", buttonStartY);
    setupButton(btnExit, "EXIT", buttonStartY + 85.f);
}

void Menu::setupButton(Button& btn, std::string label, float yPos) {
    // Pengaturan Kotak Tombol
    float btnWidth = 350.f;
    float btnHeight = 70.f;
    
    btn.shape.setSize(sf::Vector2f(btnWidth, btnHeight));
    btn.shape.setOrigin(btnWidth / 2.0f, btnHeight / 2.0f);
    btn.shape.setPosition(windowWidth / 2.0f, yPos);
    
    // Warna
    btn.shape.setFillColor(sf::Color(180, 180, 180)); 
    btn.shape.setOutlineThickness(2.f);
    btn.shape.setOutlineColor(sf::Color::White);

    // Pengaturan Teks
    btn.text.setFont(font);
    btn.text.setString(label);
    btn.text.setCharacterSize(28);
    btn.text.setFillColor(sf::Color::Black);
    btn.text.setStyle(sf::Text::Bold);
    
    sf::FloatRect tBounds = btn.text.getLocalBounds();
    btn.text.setOrigin(tBounds.left + tBounds.width / 2.0f, tBounds.top + tBounds.height / 2.0f);
    btn.text.setPosition(windowWidth / 2.0f, yPos);
}

int Menu::run() {
    window.create(sf::VideoMode((int)windowWidth, (int)windowHeight), "Simulation Billiard Game");
    
    while (window.isOpen()) {
        sf::Event event;
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { window.close(); return 0; }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (btnStart.isHovered(mousePos, window)) { window.close(); return 1; }
                if (btnExit.isHovered(mousePos, window)) { window.close(); return 0; }
            }
        }

        // Efek Hover
        if (btnStart.isHovered(mousePos, window)) {
            btnStart.shape.setFillColor(sf::Color::White);
        } else {
            btnStart.shape.setFillColor(sf::Color(180, 180, 180));
        }

        if (btnExit.isHovered(mousePos, window)) {
            btnExit.shape.setFillColor(sf::Color::White);
        } else {
            btnExit.shape.setFillColor(sf::Color(180, 180, 180));
        }

        window.clear();
        if (bgTexture.getSize().x > 0) window.draw(bgSprite);
        else window.clear(sf::Color(0, 50, 0));

        if (hasLogoImage) window.draw(logoSprite);
        
        window.draw(btnStart.shape);
        window.draw(btnStart.text);
        window.draw(btnExit.shape);
        window.draw(btnExit.text);

        window.display();
    }
    return 0;
}