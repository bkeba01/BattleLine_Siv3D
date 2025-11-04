@echo off
echo Starting 2 instances of BattleLine...
cd /d "%~dp0App"
start BattleLine(debug).exe
timeout /t 2
start BattleLine(debug).exe
echo Both instances started!
pause
