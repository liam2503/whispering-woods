#!/bin/bash
sleep 2
cp -R temp_update/Assets/* game.app/Contents/Resources/Assets/
cp temp_update/gameBin game.app/Contents/MacOS/
rm -rf temp_update
rm update.zip
open game.app
exit