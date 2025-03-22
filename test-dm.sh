#!/bin/bash
echo "Running Brogue DM agent test"

echo "1. Starting DM agent server..."
cd dm-agent
# Start the server in the background
npm start &
SERVER_PID=$!

echo "2. Waiting for server to start..."
sleep 3

echo "3. Running test events..."
node test.js

echo "4. Shutting down server..."
kill $SERVER_PID

echo "Done! Test complete." 