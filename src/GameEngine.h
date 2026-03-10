#pragma once

#include "Assets.h"
#include "Common.h"
#include "Scene.h"
#include <memory>
#include <SFML/Audio.hpp>
#include <map>
#include <list>

class Scene;
class Assets;

typedef std::map<std::string, Scene *> SceneMap;

class GameEngine
{
protected:
    // --- Core Systems ---
    sf::RenderWindow m_window;
    bool m_bIsRunning = true;
    Assets m_assets;
    size_t m_nCurrentFrame = 0;

    // --- Scene Management ---
    std::string m_strCurrentScene;
    SceneMap m_mapScenes;
    std::string m_sNextSceneName = "";
    Scene *m_pNextScene = nullptr;
    bool m_bNextEndCurrent = false;

    // --- Transition Effects ---
    float m_fFadeAlpha = 0.0f;
    bool m_bIsFadingOut = false;
    bool m_bIsFadingIn = true;
    std::string m_pendingLevelPath = "";
    bool m_isLoading = false;
    float m_loadingProgress = 0.0f;
    sf::Text m_loadingText;
    bool m_hasRenderedLoadingScreen = false;

    // --- Input System ---
    std::map<std::string, sf::Keyboard::Key> m_keyMap;
    std::map<std::string, unsigned int> m_controllerMap;
    bool m_bUseController = false;
    int m_nJoystickID = 0;
    VectorPP m_vVirtualCursorPos = {640, 360};
    float m_scrollCooldown = 0.0f;
    const float SCROLL_DELAY = 0.15f;
    bool m_bSwapABXY = false; 

    // --- Audio System ---
    sf::Music m_music;
    std::list<sf::Sound> m_sounds;
    int m_nMusicVolume = 100;
    int m_nSFXVolume = 100;
    int m_nVoiceVolume = 50;
    std::string m_sMusicPathCurrent = "";
    std::string m_sMusicPathRequest = "";
    float m_fVolumeMax;
    float m_fVolumeTarget = 0.0f;

    // --- Internal Helpers ---
    void init(const std::string &a_strPath);
    void initKeyBindings();
    void update();
    void sysUserInput();
    Scene *currentScene();

public:
    // --- Lifecycle ---
    GameEngine(const std::string &a_strPath);
    ~GameEngine();
    void run();
    void quit();
    bool isRunning();

    // --- Scene Control ---
    void changeScene(const std::string &a_strSceneName, Scene *a_pScene, bool a_bEndCurrentScene = false, bool useFade = true);
    Scene *getScene(const std::string &a_strScene);
    void prepareLevelLoad(const std::string &path) { m_pendingLevelPath = path; }
    void renderFade();
    void setLoadingScreen(bool active, const std::string &message = "Loading...");
    void setLoadingProgress(float progress) { m_loadingProgress = progress; }

    // --- Input Accessors ---
    sf::Keyboard::Key getKey(const std::string &action) const;
    void setKey(const std::string &action, sf::Keyboard::Key key);
    unsigned int getControllerButton(const std::string &action) const;
    void setControllerButton(const std::string &action, unsigned int button);
    int getJoystickID() const;
    void setJoystickID(int id);
    std::string getJoystickName() const;
    bool isUsingController() const { return m_bUseController; }
    void setSwapABXY(bool enabled);
    bool getSwapABXY() const { return m_bSwapABXY; }

    // --- Audio Accessors ---
    void requestMusic(const std::string &path, float targetVolume);
    void updateMusicVolume(int volume);
    void playSound(const std::string &name);
    void setVoiceVolume(int volume);
    float getVoiceVolume() const;
    int getSFXVolume() const { return m_nSFXVolume; }
    void setSFXVolume(int vol) { m_nSFXVolume = vol; }

    // --- Resources ---
    sf::RenderWindow &window();
    const Assets &getAssets() const;
    size_t getCurrentFrame();
};