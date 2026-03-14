#include "GameEngine.h"
#include "Assets.h"
#include "ScenePlay.h"
#include "SceneMenu.h"
#include "SceneMenu.h"
#include "SceneEditor.h"
#include "Action.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>

GameEngine::GameEngine(const std::string &a_strPath)
{
    init(a_strPath);
}

GameEngine::~GameEngine()
{
    auto iter = m_mapScenes.begin();
    while (iter != m_mapScenes.end())
    {
        delete iter->second;
        iter->second = nullptr;
        iter++;
    }
}

void GameEngine::initKeyBindings()
{
    m_keyMap["MOVE_LEFT"] = sf::Keyboard::A;
    m_keyMap["MOVE_RIGHT"] = sf::Keyboard::D;
    m_keyMap["MOVE_UP"] = sf::Keyboard::W;
    m_keyMap["MOVE_DOWN"] = sf::Keyboard::S;

    m_keyMap["JUMP"] = sf::Keyboard::W;
    m_keyMap["BURROW"] = sf::Keyboard::S;

    m_keyMap["MELEE"] = sf::Keyboard::Space;
    m_keyMap["BOOMERANG"] = sf::Keyboard::E;
    m_keyMap["SHOOT"] = sf::Keyboard::Unknown;

    m_keyMap["ATTACK"] = sf::Keyboard::Space;
    m_keyMap["ELEMENT_SWITCH"] = sf::Keyboard::E;
    m_keyMap["MENU"] = sf::Keyboard::Escape;
    m_keyMap["SELECT"] = sf::Keyboard::Enter;
    m_keyMap["WEAPON_UP"] = sf::Keyboard::Num1;
    m_keyMap["WEAPON_DOWN"] = sf::Keyboard::Num2;

    m_controllerMap["ATTACK"] = 0;
    m_controllerMap["JUMP"] = 1;
    m_controllerMap["BURROW"] = 2;
    m_controllerMap["ELEMENT_SWITCH"] = 3;

    m_controllerMap["WEAPON_SWITCH_L"] = 4;
    m_controllerMap["WEAPON_SWITCH_R"] = 5;
    m_controllerMap["MENU"] = 7;
    m_controllerMap["SELECT"] = 0;
    m_controllerMap["CANCEL"] = 1;
    m_controllerMap["ERASER"] = 2;
}

void GameEngine::init(const std::string &a_strPath)
{
    initKeyBindings();
    m_assets.loadFromFile(a_strPath);
    m_window.create(sf::VideoMode(1280, 720), "The Whispering Wood", sf::Style::Default);
    m_window.setFramerateLimit(60);
    m_fFadeAlpha = 255.0f;
    m_bIsFadingIn = true;
    m_vVirtualCursorPos = VectorPP(m_window.getSize().x / 2.0f, m_window.getSize().y / 2.0f);

    m_nMusicVolume = 100;
    m_fVolumeMax = (float)m_nMusicVolume;

    m_strCurrentScene = "MENU";
    m_mapScenes["MENU"] = new SceneMenu(this);

    m_loadingText.setFont(m_assets.getFont("Pixeloid"));
    m_loadingText.setCharacterSize(48);
    m_loadingText.setFillColor(sf::Color::White);

    m_loadingText.setString("Loading...");
    m_loadingText.getLocalBounds();
    m_loadingText.setString("");
}

void GameEngine::updateViewport(unsigned int width, unsigned int height)
{
    float targetRatio = m_vInternalResolution.x / m_vInternalResolution.y;
    float windowRatio = (float)width / (float)height;
    float vWidth = 1.0f, vHeight = 1.0f, vPosX = 0.0f, vPosY = 0.0f;

    if (windowRatio > targetRatio) {
        vWidth = targetRatio / windowRatio;
        vPosX = (1.0f - vWidth) / 2.0f;
    } else {
        vHeight = windowRatio / targetRatio;
        vPosY = (1.0f - vHeight) / 2.0f;
    }
    m_letterboxViewport = sf::FloatRect(vPosX, vPosY, vWidth, vHeight);
}

void GameEngine::requestMusic(const std::string &path, float targetVolume)
{
    if (m_sMusicPathCurrent != path)
    {
        m_sMusicPathRequest = path;
    }

    m_fVolumeTarget = targetVolume;
}

void GameEngine::updateMusicVolume(int volume)
{
    m_nMusicVolume = volume;
    m_fVolumeMax = (float)m_nMusicVolume;
    m_music.setVolume((float)m_nMusicVolume);
}

int GameEngine::getJoystickID() const { return m_nJoystickID; }
void GameEngine::setJoystickID(int id) { m_nJoystickID = id; }
std::string GameEngine::getJoystickName() const
{
    if (sf::Joystick::isConnected(m_nJoystickID))
    {
        sf::Joystick::Identification id = sf::Joystick::getIdentification(m_nJoystickID);
        return std::string(id.name);
    }
    return "Disconnected";
}

void GameEngine::setVoiceVolume(int volume)
{
    if (volume < 0)
        m_nVoiceVolume = 0;
    else if (volume > 100)
        m_nVoiceVolume = 100;
    else
        m_nVoiceVolume = volume;
}

float GameEngine::getVoiceVolume() const
{
    return (float)m_nVoiceVolume;
}

void GameEngine::run()
{
    while (isRunning())
        update();
}

void GameEngine::update()
{
    float currentVolume = m_music.getVolume();
    float targetVolumeSFML = m_fVolumeTarget * (m_fVolumeMax / 100.0f);

    if (m_bIsFadingOut)
    {
        targetVolumeSFML = 0.0f;
    }

    if (m_sMusicPathCurrent != m_sMusicPathRequest && !m_sMusicPathRequest.empty() && !m_bIsFadingOut)
    {
        if (m_music.getStatus() == sf::Music::Playing)
            m_music.stop();
        if (m_music.openFromFile(m_sMusicPathRequest))
        {
            m_music.setLoop(true);
            m_music.setVolume(targetVolumeSFML);
            m_music.play();
            m_sMusicPathCurrent = m_sMusicPathRequest;
            m_sMusicPathRequest = "";
        }
    }

    if (std::abs(currentVolume - targetVolumeSFML) > 0.5f)
    {
        float speed = 0.98f;
        float newVolume = currentVolume * speed + targetVolumeSFML * (1.0f - speed);
        
        if (std::abs(currentVolume - newVolume) >= 1.0f)
        {
            m_music.setVolume(newVolume);
        }
        else
        {
            m_music.setVolume(currentVolume > targetVolumeSFML ? currentVolume - 1.0f : currentVolume + 1.0f);
        }
    }
    else if (m_music.getVolume() != targetVolumeSFML)
    {
        m_music.setVolume(targetVolumeSFML);
        if (targetVolumeSFML == 0.0f && m_music.getStatus() == sf::Music::Playing)
        {
            m_music.stop();
            m_sMusicPathCurrent = "";
        }
    }

    if (m_fFadeAlpha < 255.0f)
        sysUserInput();

    if (m_mapScenes.find(m_strCurrentScene) != m_mapScenes.end())
    {
        if (m_mapScenes.at(m_strCurrentScene) != nullptr)
        {
            m_mapScenes.at(m_strCurrentScene)->update();
        }
    }

    float fadeSpeed = 5.0f;

    if (m_bIsFadingOut)
    {
        m_fFadeAlpha += fadeSpeed;
        if (m_fFadeAlpha >= 255.0f)
        {
            m_fFadeAlpha = 255.0f;

            if (!m_pendingLevelPath.empty() && !m_hasRenderedLoadingScreen)
            {
                renderFade();      
                m_window.display();
                m_hasRenderedLoadingScreen = true;
                return;
            }

            m_bIsFadingOut = false;
            m_hasRenderedLoadingScreen = false;

            if (m_sNextSceneName == "MENU")
            {
                m_pNextScene = new SceneMenu(this);
            }
            else if (!m_pendingLevelPath.empty())
            {
                if (m_sNextSceneName == "PLAY")
                {
                    m_pNextScene = new ScenePlay(this, m_pendingLevelPath);
                }
                else if (m_sNextSceneName == "EDITOR")
                {
                    m_pNextScene = new SceneEditor(this, m_pendingLevelPath);
                }
                m_pendingLevelPath = "";
            }

            std::string oldSceneName = m_strCurrentScene;

            if (m_bNextEndCurrent)
            {
                auto it = m_mapScenes.find(oldSceneName);
                if (it != m_mapScenes.end())
                {
                    if (it->second != m_pNextScene)
                    {
                        delete it->second;
                    }
                    m_mapScenes.erase(it);
                }
            }

            m_strCurrentScene = m_sNextSceneName;
            if (m_pNextScene)
            {
                m_mapScenes[m_strCurrentScene] = m_pNextScene;
            }

            m_bIsFadingIn = true;
            m_pNextScene = nullptr;
            setLoadingScreen(false);
        }
    }
    else if (m_bIsFadingIn)
    {
        m_fFadeAlpha -= fadeSpeed;

        if (m_fFadeAlpha <= 0.0f)
        {
            m_fFadeAlpha = 0.0f;
            m_bIsFadingIn = false;
        }
    }
    m_nCurrentFrame++;
    m_scrollCooldown -= 0.01;
}

void GameEngine::renderFade()
{
    if (m_fFadeAlpha > 0.0f)
    {
        sf::RectangleShape overlay(sf::Vector2f((float)m_window.getSize().x, (float)m_window.getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)m_fFadeAlpha));

        sf::View currentView = m_window.getView();
        m_window.setView(m_window.getDefaultView());
        m_window.draw(overlay);
        m_window.setView(currentView);
    }

    if (m_isLoading)
    {
        sf::View currentView = m_window.getView();
        m_window.setView(m_window.getDefaultView());

        // 1. Solid Black Background to prevent "transparency flash"
        sf::RectangleShape bg(sf::Vector2f((float)m_window.getSize().x, (float)m_window.getSize().y));
        bg.setFillColor(sf::Color::Black);
        m_window.draw(bg);

        // 2. Centered Text
        m_loadingText.setPosition(m_window.getSize().x / 2.0f, m_window.getSize().y / 2.0f - 20.0f);
        m_window.draw(m_loadingText);

        // 3. Progress Bar
        float barWidth = 400.0f;
        float barHeight = 20.0f;
        float x = (m_window.getSize().x - barWidth) / 2.0f;
        float y = (m_window.getSize().y / 2.0f) + 40.0f;

        sf::RectangleShape barBG(sf::Vector2f(barWidth, barHeight));
        barBG.setPosition(x, y);
        barBG.setFillColor(sf::Color(50, 50, 50));
        barBG.setOutlineColor(sf::Color::White);
        barBG.setOutlineThickness(2.0f);
        m_window.draw(barBG);

        sf::RectangleShape barFill(sf::Vector2f(barWidth * m_loadingProgress, barHeight));
        barFill.setPosition(x, y);
        barFill.setFillColor(sf::Color::Green);
        m_window.draw(barFill);

        m_window.setView(currentView);
        return;
    }
}

void GameEngine::quit()
{
    m_music.stop();
    m_bIsRunning = false;
}
bool GameEngine::isRunning() { return m_bIsRunning && window().isOpen(); }
Scene *GameEngine::currentScene()
{
    // Check if the scene exists before trying to access it
    auto it = m_mapScenes.find(m_strCurrentScene);
    if (it != m_mapScenes.end())
    {
        return it->second;
    }
    return nullptr;
}
Scene *GameEngine::getScene(const std::string &scene) { return m_mapScenes[scene]; }
size_t GameEngine::getCurrentFrame() { return m_nCurrentFrame; }
sf::RenderWindow &GameEngine::window() { return m_window; }
const Assets &GameEngine::getAssets() const { return m_assets; }
void GameEngine::changeScene(const std::string &a_strSceneName, Scene *a_pScene, bool a_bEndCurrentScene, bool useFade)
{
    m_sNextSceneName = a_strSceneName;
    m_pNextScene = a_pScene;
    m_bNextEndCurrent = a_bEndCurrentScene;
    if (useFade)
    {
        m_bIsFadingOut = true;
    }
    else
    {

        m_fFadeAlpha = 255.0f;
        m_bIsFadingOut = true;
    }
}

sf::Keyboard::Key GameEngine::getKey(const std::string &action) const
{
    if (m_keyMap.find(action) != m_keyMap.end())
        return m_keyMap.at(action);
    return sf::Keyboard::Unknown;
}

void GameEngine::setKey(const std::string &action, sf::Keyboard::Key key)
{
    m_keyMap[action] = key;
}

unsigned int GameEngine::getControllerButton(const std::string &action) const
{
    if (m_controllerMap.find(action) != m_controllerMap.end())
        return m_controllerMap.at(action);
    return 99;
}

void GameEngine::setControllerButton(const std::string &action, unsigned int button)
{
    m_controllerMap[action] = button;
}

void GameEngine::sysUserInput()
{
    sf::Event event{};
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            quit();
        
        if (event.type == sf::Event::Resized)
            updateViewport(event.size.width, event.size.height);

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::X)
        {
            sf::Texture texture;
            texture.create(m_window.getSize().x, m_window.getSize().y);
            texture.update(m_window);
            texture.copyToImage().saveToFile("include/screenshots/test.png");
        }

        if (m_bIsFadingOut)
            continue;

        if (currentScene() == nullptr)
            continue;

        if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
        {
            m_bUseController = false;

            std::string keyName = std::to_string(event.key.code);
            if (currentScene()->getActionMap().find(keyName) != currentScene()->getActionMap().end())
            {
                const std::string actionType = (event.type == sf::Event::KeyPressed ? "START" : "END");
                currentScene()->sysDoAction(Action(currentScene()->getActionMap().at(keyName), actionType));
            }
        }

        sf::Vector2i tempPos = sf::Mouse::getPosition(m_window);
        VectorPP vMousePos(tempPos.x, tempPos.y);

        if (event.type == sf::Event::MouseButtonPressed)
        {
            m_bUseController = false;
            if (event.mouseButton.button == sf::Mouse::Left)
                currentScene()->sysDoAction(Action("LEFT_CLICK", "START", vMousePos));
            else if (event.mouseButton.button == sf::Mouse::Right)
                currentScene()->sysDoAction(Action("RIGHT_CLICK", "START", vMousePos));
        }
        if (event.type == sf::Event::MouseButtonReleased)
        {
            m_bUseController = false;
            if (event.mouseButton.button == sf::Mouse::Left)
                currentScene()->sysDoAction(Action("LEFT_CLICK", "END", vMousePos));
            else if (event.mouseButton.button == sf::Mouse::Right)
                currentScene()->sysDoAction(Action("RIGHT_CLICK", "END", vMousePos));
        }
        if (event.type == sf::Event::MouseMoved)
        {
            if (std::abs(event.mouseMove.x - m_vVirtualCursorPos.x) > 2 || std::abs(event.mouseMove.y - m_vVirtualCursorPos.y) > 2)
            {
                m_bUseController = false;
            }
            currentScene()->sysDoAction(Action("MOUSE_MOVE", "START", VectorPP(event.mouseMove.x, event.mouseMove.y)));
        }
        if (event.type == sf::Event::MouseWheelScrolled)
        {
            m_bUseController = false;
            if (event.mouseWheelScroll.delta > 0)
                currentScene()->sysDoAction(Action("SCROLL_UP", "START"));
            else if (event.mouseWheelScroll.delta < 0)
                currentScene()->sysDoAction(Action("SCROLL_DOWN", "START"));
        }

        if (event.type == sf::Event::JoystickButtonPressed || event.type == sf::Event::JoystickButtonReleased)
        {
            if (event.joystickButton.joystickId == m_nJoystickID)
            {
                m_bUseController = true;

                std::string btnKey = "BTN_" + std::to_string(event.joystickButton.button);

                if (currentScene()->getActionMap().find(btnKey) != currentScene()->getActionMap().end())
                {
                    const std::string actionType = (event.type == sf::Event::JoystickButtonPressed ? "START" : "END");
                    currentScene()->sysDoAction(Action(currentScene()->getActionMap().at(btnKey), actionType));
                }
            }
        }

        if (event.type == sf::Event::MouseWheelScrolled)
        {
            if (m_scrollCooldown <= 0.0f)
            {
                if (event.mouseWheelScroll.delta > 0)
                {
                    currentScene()->sysDoAction(Action("WEAPON_UP", "START"));
                }
                else if (event.mouseWheelScroll.delta < 0)
                {
                    currentScene()->sysDoAction(Action("WEAPON_DOWN", "START"));
                }
                m_scrollCooldown = SCROLL_DELAY;
            }
        }
    }

    if (sf::Joystick::isConnected(m_nJoystickID))
    {
        float threshold = 50.0f;

        float ls_x = sf::Joystick::getAxisPosition(m_nJoystickID, sf::Joystick::X);
        float ls_y = sf::Joystick::getAxisPosition(m_nJoystickID, sf::Joystick::Y);
        float rs_x = sf::Joystick::getAxisPosition(m_nJoystickID, sf::Joystick::U);
        float rs_y = sf::Joystick::getAxisPosition(m_nJoystickID, sf::Joystick::V);
        float dp_x = sf::Joystick::getAxisPosition(m_nJoystickID, sf::Joystick::PovX);
        float dp_y = sf::Joystick::getAxisPosition(m_nJoystickID, sf::Joystick::PovY);
        float trig = sf::Joystick::getAxisPosition(m_nJoystickID, sf::Joystick::Z);

        if (std::abs(ls_x) > 20 || std::abs(ls_y) > 20 || std::abs(rs_x) > 20 || std::abs(rs_y) > 20 || std::abs(dp_x) > 20 || std::abs(dp_y) > 20)
        {
            m_bUseController = true;
        }

        static bool ls_l = false, ls_r = false, ls_u = false, ls_d = false;
        static bool dp_l = false, dp_r = false, dp_u = false, dp_d = false;
        static bool rs_l = false, rs_r = false, rs_u = false, rs_d = false;
        static bool trig_press = false;

        auto dispatch = [&](const std::string &name, bool active, bool &state)
        {
            if (active)
            {
                if (!state)
                {
                    if (currentScene()->getActionMap().find(name) != currentScene()->getActionMap().end())
                        currentScene()->sysDoAction(Action(currentScene()->getActionMap().at(name), "START"));
                    state = true;
                }
            }
            else
            {
                if (state)
                {
                    if (currentScene()->getActionMap().find(name) != currentScene()->getActionMap().end())
                        currentScene()->sysDoAction(Action(currentScene()->getActionMap().at(name), "END"));
                    state = false;
                }
            }
        };

        dispatch("LS_LEFT", ls_x < -threshold, ls_l);
        dispatch("LS_RIGHT", ls_x > threshold, ls_r);
        dispatch("LS_UP", ls_y < -threshold, ls_u);
        dispatch("LS_DOWN", ls_y > threshold, ls_d);

        dispatch("DPAD_LEFT", dp_x < -threshold, dp_l);
        dispatch("DPAD_RIGHT", dp_x > threshold, dp_r);
        dispatch("DPAD_UP", dp_y > threshold, dp_u);
        dispatch("DPAD_DOWN", dp_y < -threshold, dp_d);

        dispatch("RS_LEFT", rs_x < -threshold, rs_l);
        dispatch("RS_RIGHT", rs_x > threshold, rs_r);
        dispatch("RS_UP", rs_y < -threshold, rs_u);
        dispatch("RS_DOWN", rs_y > threshold, rs_d);

        dispatch("TRIGGER", std::abs(trig) > threshold, trig_press);

        if (m_bUseController)
        {
        }
    }
}

void GameEngine::setSwapABXY(bool enabled)
{
    m_bSwapABXY = enabled;

    // 0 = Bottom (Xbox A / Nintendo B)
    // 1 = Right  (Xbox B / Nintendo A)
    // 2 = Left   (Xbox X / Nintendo Y)
    // 3 = Top    (Xbox Y / Nintendo X)

    if (m_bSwapABXY)
    {
        // Nintendo Style
        m_controllerMap["ATTACK"] = 1;
        m_controllerMap["JUMP"] = 0;
        m_controllerMap["BURROW"] = 3;
        m_controllerMap["ELEMENT_SWITCH"] = 2;

        m_controllerMap["SELECT"] = 1;
        m_controllerMap["CANCEL"] = 0;

        m_controllerMap["MELEE"] = 1;
        m_controllerMap["BOOMERANG"] = 2;
        m_controllerMap["ERASER"] = 3;
    }
    else
    {
        // Xbox Style
        m_controllerMap["ATTACK"] = 0;
        m_controllerMap["JUMP"] = 1;
        m_controllerMap["BURROW"] = 2;
        m_controllerMap["ELEMENT_SWITCH"] = 3;

        m_controllerMap["SELECT"] = 0;
        m_controllerMap["CANCEL"] = 1;

        m_controllerMap["MELEE"] = 0;
        m_controllerMap["BOOMERANG"] = 3;
        m_controllerMap["ERASER"] = 2;
    }
}

void GameEngine::playSound(const std::string &name)
{
    m_sounds.remove_if([](const sf::Sound &s)
                       { return s.getStatus() == sf::Sound::Stopped; });

    const auto &soundData = m_assets.getSound(name);

    m_sounds.emplace_back();
    sf::Sound &sound = m_sounds.back();

    sound.setBuffer(soundData.buffer);

    float finalVolume = (m_nSFXVolume / 100.0f) * soundData.volume;
    sound.setVolume(finalVolume);

    sound.play();
}

void GameEngine::setLoadingScreen(bool active, const std::string &message)
{
    m_isLoading = active;
    m_loadingProgress = 0.0f;
    m_loadingText.setString(message);
    sf::FloatRect textRect = m_loadingText.getLocalBounds();
    m_loadingText.setOrigin(textRect.left + textRect.width / 2.0f,
                            textRect.top + textRect.height / 2.0f);
}

const sf::Vector2f& GameEngine::getInternalResolution() const
{
    return m_vInternalResolution;
}

void GameEngine::setInternalResolution(float w, float h)
{
    m_vInternalResolution = sf::Vector2f(w, h);
    updateViewport(m_window.getSize().x, m_window.getSize().y);
}

bool GameEngine::isFullscreen() const
{
    return m_bFullscreen;
}

void GameEngine::setFullscreen(bool fullscreen)
{
    m_bFullscreen = fullscreen;
}

void GameEngine::applyVideoSettings()
{
    if (m_bFullscreen)
    {
        m_window.create(sf::VideoMode::getDesktopMode(), "The Whispering Wood", sf::Style::Fullscreen);
    }
    else
    {
        m_window.create(sf::VideoMode(1280, 720), "The Whispering Wood", sf::Style::Default);
    }
    m_window.setFramerateLimit(60);
    updateViewport(m_window.getSize().x, m_window.getSize().y);
}