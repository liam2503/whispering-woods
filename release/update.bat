@echo off
timeout /t 2 /nobreak > NUL
xcopy /Y /E temp_update\Assets\* Assets\
xcopy /Y temp_update\game.exe .\
rmdir /S /Q temp_update
del game.zip
start game.exe
exit