@echo off
echo Running Brogue DM agent test

echo 1. Starting DM agent server...
cd dm-agent
start cmd /k "npm start"

echo 2. Waiting for server to start...
timeout /t 3 /nobreak

echo 3. Running test events...
cd dm-agent
node test.js

echo Done! Check the server window for output.
echo Press any key to exit...
pause > nul 