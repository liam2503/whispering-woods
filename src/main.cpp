#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <fstream>
#include <string>
#include <cpr/cpr.h>
#include "GameEngine.h"

bool checkForUpdates(const std::string& currentVersion) 
{
    cpr::Response r = cpr::Get(cpr::Url{"https://liam2503.github.io/version.txt"});
    return (r.status_code == 200 && r.text != currentVersion);
}

void downloadAndExtractUpdates() 
{
    cpr::Response r = cpr::Get(cpr::Url{"https://liam2503.github.io/game.zip"});
    
    std::ofstream out("game.zip", std::ios::binary);
    out.write(r.text.data(), r.text.size());
    out.close();

#ifdef _WIN32
    std::system("powershell -command \"Expand-Archive -Force 'game.zip' 'temp_update'\"");
#else
    std::system("unzip -o game.zip -d temp_update");
#endif
}

void launchUpdater() 
{
#ifdef _WIN32
    std::system("start update.bat");
#else
    std::system("chmod +x update.sh && ./update.sh &");
#endif
}

int main()
{
    if (checkForUpdates("1.0.0")) 
    {
        downloadAndExtractUpdates();
        launchUpdater();
        return 0;
    }

    GameEngine g("Assets/assets.csv");
    g.run();
    
    return 0;
}