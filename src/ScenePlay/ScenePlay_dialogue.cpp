#include "../ScenePlay.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <SFML/Audio.hpp>
#include "GameEngine.h"

void ScenePlay::initializeSpeechSounds()
{
    m_rng.seed(static_cast<unsigned int>(std::time(0)));

    std::vector<std::string> DemonsoundFilePaths = {
        "Assets/SFX/Demon/ba.ogg", "Assets/SFX/Demon/be.ogg", "Assets/SFX/Demon/bi.ogg", "Assets/SFX/Demon/bo.ogg", "Assets/SFX/Demon/bu.ogg",
        "Assets/SFX/Demon/da.ogg", "Assets/SFX/Demon/de.ogg", "Assets/SFX/Demon/di.ogg", "Assets/SFX/Demon/do.ogg", "Assets/SFX/Demon/du.ogg",
        "Assets/SFX/Demon/ka.ogg", "Assets/SFX/Demon/ke.ogg", "Assets/SFX/Demon/ki.ogg", "Assets/SFX/Demon/ko.ogg", "Assets/SFX/Demon/ku.ogg",
        "Assets/SFX/Demon/la.ogg", "Assets/SFX/Demon/le.ogg", "Assets/SFX/Demon/li.ogg", "Assets/SFX/Demon/lo.ogg", "Assets/SFX/Demon/lu.ogg",
        "Assets/SFX/Demon/na.ogg", "Assets/SFX/Demon/ne.ogg", "Assets/SFX/Demon/ni.ogg", "Assets/SFX/Demon/no.ogg", "Assets/SFX/Demon/nu.ogg",
        "Assets/SFX/Demon/ma.ogg", "Assets/SFX/Demon/me.ogg", "Assets/SFX/Demon/mi.ogg", "Assets/SFX/Demon/mo.ogg", "Assets/SFX/Demon/mu.ogg",
        "Assets/SFX/Demon/pa.ogg", "Assets/SFX/Demon/pe.ogg", "Assets/SFX/Demon/pi.ogg", "Assets/SFX/Demon/po.ogg", "Assets/SFX/Demon/pu.ogg",
        "Assets/SFX/Demon/fa.ogg", "Assets/SFX/Demon/fe.ogg", "Assets/SFX/Demon/fi.ogg", "Assets/SFX/Demon/fo.ogg", "Assets/SFX/Demon/fu.ogg",
        "Assets/SFX/Demon/ra.ogg", "Assets/SFX/Demon/re.ogg", "Assets/SFX/Demon/ri.ogg", "Assets/SFX/Demon/ro.ogg", "Assets/SFX/Demon/ru.ogg",
        "Assets/SFX/Demon/sha.ogg", "Assets/SFX/Demon/she.ogg", "Assets/SFX/Demon/shi.ogg", "Assets/SFX/Demon/sho.ogg", "Assets/SFX/Demon/shu.ogg"};

    std::vector<std::string> FamiliarSoundFilePaths = {
        "Assets/SFX/Familiar/ba.ogg", "Assets/SFX/Familiar/be.ogg", "Assets/SFX/Familiar/bi.ogg", "Assets/SFX/Familiar/bo.ogg", "Assets/SFX/Familiar/bu.ogg",
        "Assets/SFX/Familiar/da.ogg", "Assets/SFX/Familiar/de.ogg", "Assets/SFX/Familiar/di.ogg", "Assets/SFX/Familiar/do.ogg", "Assets/SFX/Familiar/du.ogg",
        "Assets/SFX/Familiar/ka.ogg", "Assets/SFX/Familiar/ke.ogg", "Assets/SFX/Familiar/ki.ogg", "Assets/SFX/Familiar/ko.ogg", "Assets/SFX/Familiar/ku.ogg",
        "Assets/SFX/Familiar/la.ogg", "Assets/SFX/Familiar/le.ogg", "Assets/SFX/Familiar/li.ogg", "Assets/SFX/Familiar/lo.ogg", "Assets/SFX/Familiar/lu.ogg",
        "Assets/SFX/Familiar/na.ogg", "Assets/SFX/Familiar/ne.ogg", "Assets/SFX/Familiar/ni.ogg", "Assets/SFX/Familiar/no.ogg", "Assets/SFX/Familiar/nu.ogg",
        "Assets/SFX/Familiar/ma.ogg", "Assets/SFX/Familiar/me.ogg", "Assets/SFX/Familiar/mi.ogg", "Assets/SFX/Familiar/mo.ogg", "Assets/SFX/Familiar/mu.ogg",
        "Assets/SFX/Familiar/pa.ogg", "Assets/SFX/Familiar/pe.ogg", "Assets/SFX/Familiar/pi.ogg", "Assets/SFX/Familiar/po.ogg", "Assets/SFX/Familiar/pu.ogg",
        "Assets/SFX/Familiar/fa.ogg", "Assets/SFX/Familiar/fe.ogg", "Assets/SFX/Familiar/fi.ogg", "Assets/SFX/Familiar/fo.ogg", "Assets/SFX/Familiar/fu.ogg",
        "Assets/SFX/Familiar/ra.ogg", "Assets/SFX/Familiar/re.ogg", "Assets/SFX/Familiar/ri.ogg", "Assets/SFX/Familiar/ro.ogg", "Assets/SFX/Familiar/ru.ogg",
        "Assets/SFX/Familiar/sha.ogg", "Assets/SFX/Familiar/she.ogg", "Assets/SFX/Familiar/shi.ogg", "Assets/SFX/Familiar/sho.ogg", "Assets/SFX/Familiar/shu.ogg"};

    for (const auto &filePath : DemonsoundFilePaths)
    {
        sf::SoundBuffer buffer;
        if (buffer.loadFromFile(filePath))
        {
            m_DemonSpeechSoundBuffers.push_back(std::move(buffer));
        }
        else
        {
            std::cerr << "Error: Could not load speech sound file: " << filePath << std::endl;
        }
    }

    for (const auto &filePath : FamiliarSoundFilePaths)
    {
        sf::SoundBuffer buffer;
        if (buffer.loadFromFile(filePath))
        {
            m_FamiliarSpeechSoundBuffers.push_back(std::move(buffer));
        }
        else
        {
            std::cerr << "Error: Could not load speech sound file: " << filePath << std::endl;
        }
    }
}

void ScenePlay::loadCutscene(const std::string &a_strFilePath)
{
    m_cutsceneQueue.clear();
    std::ifstream infile(a_strFilePath);

    if (!infile.is_open())
    {
        std::cerr << "Failed to load cutscene: " << a_strFilePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(infile, line))
    {
        if (line.empty() || (line.size() >= 2 && line[0] == '/' && line[1] == '/'))
            continue;

        std::stringstream ss(line);
        std::string header, speaker, content;
        ss >> header;

        if (header == "Message")
        {

            if (ss >> speaker)
            {
                std::getline(ss, content);

                if (!content.empty() && content[0] == ' ')
                {
                    content.erase(0, 1);
                }
                m_cutsceneQueue.push_back({speaker, content});
            }
        }
        else if (header == "Command")
        {
            DialogueLine cmdLine;
            DialogueCommand cmd;

            if (ss >> cmd.targetTag >> cmd.action >> cmd.value1 >> cmd.value2 >> cmd.duration)
            {
                cmdLine.isCommand = true;
                cmdLine.command = cmd;
                cmdLine.speaker = "CMD";
                cmdLine.text = "";

                m_cutsceneQueue.push_back(cmdLine);
            }
            else
            {
                std::cerr << "Cutscene Error: Malformed Command line: " << line << std::endl;
            }
        }
    }

    if (!m_cutsceneQueue.empty())
    {
        m_bShowDialogue = true;

        nextDialogueLine();
    }
}

void ScenePlay::playRandomSpeechSound()
{
    std::vector<sf::SoundBuffer> *targetBuffers = nullptr;
    float pitch = 1.0f;

    if (m_currentSpeaker == "Fox")
    {
        targetBuffers = &m_FamiliarSpeechSoundBuffers;
        pitch = 1.5f;
    }
    else if (m_currentSpeaker == "Kao(Crow)")
    {
        targetBuffers = &m_DemonSpeechSoundBuffers;
        pitch = 1.0f;
    }
    else if (m_currentSpeaker == "Familiar(Fairy)")
    {
        targetBuffers = &m_FamiliarSpeechSoundBuffers;
        pitch = 1.0f;
    }
    else
    {

        targetBuffers = &m_DemonSpeechSoundBuffers;
        pitch = 1.0f;
    }

    if (targetBuffers->empty())
        return;

    std::uniform_int_distribution<> distrib(0, targetBuffers->size() - 1);
    size_t randomIndex = distrib(m_rng);

    m_chatterSound.setBuffer((*targetBuffers)[randomIndex]);
    m_chatterSound.setPitch(pitch);

    m_chatterSound.play();
}

void ScenePlay::nextDialogueLine()
{
    if (m_cutsceneQueue.empty())
    {
        m_bShowDialogue = false;
        m_currentSpeaker = "";
        m_pGame->playSound("apply");
        return;
    }

    DialogueLine currentLine = m_cutsceneQueue.front();
    m_cutsceneQueue.erase(m_cutsceneQueue.begin());

    if (currentLine.isCommand)
    {
        processCommand(currentLine.command);

        return nextDialogueLine();
    }

    m_pGame->playSound("ui");

    m_currentSpeaker = currentLine.speaker;
    m_speakerText.setString(currentLine.speaker);
    m_fullText = currentLine.text;
    m_currentText = "";
    m_charIndex = 0;
    m_textTimer = 0;

    playRandomSpeechSound();
}

void ScenePlay::updateDialogue()
{
    if (!m_bShowDialogue)
        return;

    if (m_charIndex < m_fullText.size())
    {
        m_chatterTimer++;

        if (m_chatterTimer >= CHATTER_RATE)
        {
            playRandomSpeechSound();
            m_chatterTimer = 0;
        }
    }

    if (m_charIndex < m_fullText.size())
    {
        m_textTimer++;

        if (m_textTimer >= 3)
        {
            m_currentText += m_fullText[m_charIndex];
            m_charIndex++;
            m_dialogueText.setString(m_currentText);
            m_textTimer = 0;
        }
    }
}

void ScenePlay::processCommand(const DialogueCommand &cmd)
{
    Entity *target = nullptr;
    if (cmd.targetTag == "player")
    {
        target = m_player;
    }
    else
    {
        const EntityVec &entities = m_entityManager.getEntities(cmd.targetTag);
        if (!entities.empty())
        {
            target = entities.front();
        }
    }

    if (!target)
    {
        std::cerr << "Command Error: Target entity '" << cmd.targetTag << "' not found.\n";
        return;
    }

    if (cmd.action == "MOVE")
    {
        if (target->hasComponent<CompTransform>())
        {
            try
            {
                float gridX = std::stof(cmd.value1);
                float gridY = std::stof(cmd.value2);

                float scale = 1.0f;
                if (target->hasComponent<CompTransform>())
                {
                    scale = target->getComponent<CompTransform>().vScale.x;
                }

                VectorPP newPos = gridToMidPixel(gridX, gridY, target, std::abs(scale));
                target->getComponent<CompTransform>().vPosition = newPos;
                target->getComponent<CompTransform>().vPrevPos = newPos;
            }
            catch (const std::invalid_argument &e)
            {
                std::cerr << "Command Error: Invalid numeric value for MOVE command.\n";
            }
        }
    }
    else if (cmd.action == "ANIM")
    {
        if (target->hasComponent<CompAnimation>())
        {
            bool repeat = (cmd.value2 == "true");

            target->getComponent<CompAnimation>().animation = m_pGame->getAssets().getAnimation(cmd.value1);
            target->getComponent<CompAnimation>().bRepeat = repeat;
        }
    }
    else if (cmd.action == "FACE")
    {
        if (target->hasComponent<CompTransform>())
        {
            float targetSign = (cmd.value1 == "right") ? 1.0f : -1.0f;
            target->getComponent<CompTransform>().vScale.x = std::abs(target->getComponent<CompTransform>().vScale.x) * targetSign;
        }
    }
}