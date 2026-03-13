@echo off
timeout /t 3 /nobreak > NUL

:: Update Assets and EXE
xcopy /Y /E temp_update\Assets\* Assets\
xcopy /Y temp_update\game.exe .\

:: NEW: Update the local version file
xcopy /Y temp_update\version.txt .\

:: Clean up
rmdir /S /Q temp_update
del game.zip

start game.exe
exit