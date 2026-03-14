#include "../SceneMenu.h"
#include "GameEngine.h"
#include "Assets.h"
#include <cmath>

void SceneMenu::sysRender()
{
    sf::View view(sf::FloatRect(0.f, 0.f, (float)width(), (float)height()));
    view.setViewport(m_pGame->getViewport());
    m_pGame->window().setView(view);

    m_pGame->window().clear(sf::Color::Black);

    // Render Static Screens
    if (m_currentState == CREDITS_SCREEN)
    {
        m_pGame->window().draw(m_creditsSprite);
        m_pGame->renderFade();
        m_pGame->window().display();
        return;
    }
    if (m_currentState == CONTROLS_SCREEN)
    {
        m_pGame->window().draw(m_controlsSprite);
        m_pGame->renderFade();
        m_pGame->window().display();
        return;
    }

    // Render Background & Title
    m_pGame->window().draw(m_backgroundSprite);
    m_titleText.setPosition(sf::Vector2f(width() / 2.0f - m_titleText.getLocalBounds().width / 2.0f, 40.0f));
    m_pGame->window().draw(m_titleText);

    // Determine Subtitle
    std::string subTitle;
    if (m_currentState == START_GAME)
        subTitle = "Start Game";
    else if (m_currentState == OPTIONS_MAIN)
        subTitle = "Options";
    else if (m_currentState == OPTIONS_GAME)
        subTitle = "Game Settings";
    else if (m_currentState == OPTIONS_AUDIO)
        subTitle = "Audio Settings";
    else if (m_currentState == OPTIONS_CONTROLLER)
        subTitle = "Controller Settings";
    else if (m_currentState == OPTIONS_REMAP_KEYBOARD)
        subTitle = "Key Bindings";
    else if (m_currentState == OPTIONS_VIDEO)
        subTitle = "Video Settings";

    if (!subTitle.empty())
{
    m_subtitleText.setString(subTitle);
    m_subtitleText.setPosition(sf::Vector2f(width() / 2.0f - m_subtitleText.getLocalBounds().width / 2.0f, 250.0f));
    m_pGame->window().draw(m_subtitleText);
}

    // Render Menu Options
    const auto &currentMenu = m_mapMenuOptions.at(m_currentState);
    float startY = (m_currentState == MAIN_MENU) ? 275.0f : 315.0f;
    float verticalSpacing = (m_currentState == MAIN_MENU) ? 65.0f : 50.0f;
    if (m_currentState == OPTIONS_REMAP_KEYBOARD)
    {
        verticalSpacing = 35.0f;
        startY = 300.0f;
    }
    else if (m_currentState == OPTIONS_CONTROLLER)
        startY = 315.0f;

    // Pulse Effect
    float pulse = (std::sin(m_fPulseTimer) + 1.0f) / 2.0f;
    sf::Uint8 blueChannel = static_cast<sf::Uint8>(255.0f * (1.0f - pulse));
    sf::Color pulseColor(255, 255, blueChannel);

    for (size_t i = 0; i < currentMenu.size(); i++)
    {
        std::string optionText = currentMenu.at(i).label;

        // Dynamic Text Replacements
        if (m_currentState == START_GAME && optionText.find("Select Level:") != std::string::npos)
            optionText = "Select Level: < " + m_vecLevelNames[m_nSelectedLevelIndex] + " >";
        else if (m_currentState == START_GAME && optionText.find("Level Editor") != std::string::npos)
            optionText = "Level Editor: < " + m_vecLevelNames[m_nSelectedEditorLevelIndex] + " >";
        else if (m_currentState == OPTIONS_GAME && optionText.find("Game Difficulty:") != std::string::npos)
            optionText = "Game Difficulty: < " + m_vecDifficulties[m_nSelectedDifficulty] + " >";
        else if (m_currentState == OPTIONS_AUDIO && optionText.find("Music Volume:") != std::string::npos)
            optionText = "Music Volume: < " + std::to_string(m_nMusicVolume) + " >";
        else if (m_currentState == OPTIONS_AUDIO && optionText.find("SFX Volume:") != std::string::npos)
            optionText = "SFX Volume: < " + std::to_string(m_nSFXVolume) + " >";
        else if (m_currentState == OPTIONS_AUDIO && optionText.find("Voice Volume:") != std::string::npos)
            optionText = "Voice Volume: < " + std::to_string(m_nVoiceVolume) + " >";
        else if (m_currentState == OPTIONS_CONTROLLER && optionText.find("Swap A/B & X/Y:") != std::string::npos)
        {
            std::string status = m_pGame->getSwapABXY() ? "ON" : "OFF";
            optionText = "Swap A/B & X/Y: < " + status + " >";
        }
        else if (m_currentState == OPTIONS_VIDEO && optionText.find("Resolution:") != std::string::npos)
        {
            std::string resStr = std::to_string((int)m_vecResolutions[m_nSelectedResolution].x) + "x" + std::to_string((int)m_vecResolutions[m_nSelectedResolution].y);
            optionText = "Resolution: < " + resStr + " >";
        }
        else if (m_currentState == OPTIONS_VIDEO && optionText.find("Fullscreen:") != std::string::npos)
        {
            std::string fsStr = m_bSelectedFullscreen ? "ON" : "OFF";
            optionText = "Fullscreen: < " + fsStr + " >";
        }

        m_menuText.setString(optionText);

        if (m_currentState == MAIN_MENU)
        {
            m_menuText.setFont(m_pGame->getAssets().getFont("Harnold"));
            m_menuText.setCharacterSize(CHAR_SIZE_ITEM_MAIN);
        }
        else
        {
            m_menuText.setFont(m_pGame->getAssets().getFont("Pixeloid"));
            if (m_currentState == OPTIONS_REMAP_KEYBOARD)
                m_menuText.setCharacterSize(CHAR_SIZE_ITEM_SMALL);
            else
                m_menuText.setCharacterSize(CHAR_SIZE_ITEM_SUB);
        }

        if (i == m_nSelectedMenuIndex)
            m_menuText.setFillColor(pulseColor);
        else
            m_menuText.setFillColor(sf::Color::White);

        m_menuText.setPosition(sf::Vector2f((width() / 2.0f) - (m_menuText.getLocalBounds().width / 2.0f), startY + i * verticalSpacing));
        m_pGame->window().draw(m_menuText);
    }

    // Render Input Wait Overlay
    if (m_bWaitingForInput)
    {
        sf::Text waitText("Press new key...", m_pGame->getAssets().getFont("Harnold"), CHAR_SIZE_TITLE);
        waitText.setFillColor(sf::Color::Red);
        waitText.setOrigin(waitText.getLocalBounds().width / 2, waitText.getLocalBounds().height / 2);
        waitText.setPosition(width() / 2, height() / 2);
        m_pGame->window().draw(waitText);
    }

    // Render Developer Footer
    m_developerText.setPosition(sf::Vector2f((width() / 2.0f) - (m_developerText.getLocalBounds().width / 2.0f), height() - m_developerText.getCharacterSize() * 2));
    m_pGame->window().draw(m_developerText);

    // Render Splash Screen Overlay
    if (m_bShowSplash)
    {
        float fadeDuration = 1.0f;
        float holdDuration = m_fSplashDuration - fadeDuration;
        sf::Uint8 alpha = 255;

        if (m_fSplashTimer > holdDuration)
        {
            float fadeProgress = (m_fSplashTimer - holdDuration) / fadeDuration;
            alpha = static_cast<sf::Uint8>(255 * (1.0f - fadeProgress));
        }

        m_splashSprite.setColor(sf::Color(255, 255, 255, alpha));
        m_pGame->window().draw(m_splashSprite);
    }

    m_pGame->renderFade();
    m_pGame->window().display();
}