#include <SFML/Graphics.hpp>
#include "GameEngine.h"
int main()
{
  GameEngine g("Assets/assets.csv");
  g.run();
}