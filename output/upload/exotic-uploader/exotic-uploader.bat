@echo off

echo Installing nrf52 board libraries...
arduino-cli core update-index
arduino-cli core install adafruit:nrf52

echo Search for Arduino COM port...
arduino-cli board list > board-list.txt

SETLOCAL
SET line = ""
FOR /f "tokens=* delims=" %%a in ('type ".\board-list.txt"^| Find /i "Adafruit Feather nRf52840 Express"') DO SET line=%%a
FOR /F "tokens=1" %%a in ("%line%") DO SET port=%%a

echo Arduino COM Port found = %port%

echo Transfering firmware...
arduino-cli upload --port %port% --fqbn adafruit:nrf52:feather52840 -i position-switch-connected-v1.2.1.zip
ENDLOCAL

del board-list.txt
PAUSE