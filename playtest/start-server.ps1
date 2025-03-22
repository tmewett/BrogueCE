# Brogue DM AI - Server Startup Script
# This PowerShell script starts the DM Agent server

# Configuration
$DM_AGENT_DIR = Join-Path $PSScriptRoot "..\dm-agent"
$SERVER_SCRIPT = Join-Path $DM_AGENT_DIR "server\server.js"
$PORT = 3001

# ANSI colors for PowerShell output
$ESC = [char]27
$COLORS = @{
    RESET = "$ESC[0m"
    BRIGHT = "$ESC[1m"
    RED = "$ESC[31m"
    GREEN = "$ESC[32m"
    YELLOW = "$ESC[33m"
    BLUE = "$ESC[34m"
    MAGENTA = "$ESC[35m"
    CYAN = "$ESC[36m"
}

# Check if Node.js is installed
try {
    $nodeVersion = node --version
    Write-Host "$($COLORS.GREEN)Node.js found: $nodeVersion$($COLORS.RESET)"
} catch {
    Write-Host "$($COLORS.RED)$($COLORS.BRIGHT)ERROR: Node.js not found. Please install Node.js to run this server.$($COLORS.RESET)"
    Write-Host "$($COLORS.YELLOW)Download from: https://nodejs.org/$($COLORS.RESET)"
    exit 1
}

# Function to check if port is in use
function Test-PortInUse {
    param ($port)
    
    $connection = Test-NetConnection -ComputerName localhost -Port $port -ErrorAction SilentlyContinue -WarningAction SilentlyContinue
    return $connection.TcpTestSucceeded
}

# Check if required npm packages are installed
if (-not (Test-Path (Join-Path $DM_AGENT_DIR "node_modules"))) {
    Write-Host "$($COLORS.YELLOW)Installing npm packages...$($COLORS.RESET)"
    Set-Location $DM_AGENT_DIR
    npm install
    Set-Location $PSScriptRoot
}

# Main execution
Write-Host "$($COLORS.BRIGHT)$($COLORS.MAGENTA)BROGUE DM AI - SERVER STARTUP$($COLORS.RESET)"

# Check if server is already running
if (Test-PortInUse $PORT) {
    Write-Host "$($COLORS.YELLOW)$($COLORS.BRIGHT)Server already running on port $PORT$($COLORS.RESET)"
    
    # Ask if user wants to restart it
    $restart = Read-Host "Do you want to restart the server? (y/n)"
    if ($restart -ne "y") {
        Write-Host "$($COLORS.GREEN)Using the existing server. Run the simulation with:$($COLORS.RESET)"
        Write-Host "$($COLORS.CYAN).\simulate-gameplay.ps1$($COLORS.RESET)"
        exit 0
    }
    
    Write-Host "$($COLORS.YELLOW)Attempting to restart server...$($COLORS.RESET)"
}

# Start the server in a new PowerShell window
Write-Host "$($COLORS.CYAN)Starting DM Agent server...$($COLORS.RESET)"
Start-Process powershell -ArgumentList "-NoExit", "-Command", "cd '$DM_AGENT_DIR'; node server/server.js"

# Wait for server to start
$attempts = 0
$maxAttempts = 10
$serverRunning = $false

Write-Host "$($COLORS.CYAN)Waiting for server to start...$($COLORS.RESET)"

while (-not $serverRunning -and $attempts -lt $maxAttempts) {
    Start-Sleep -Seconds 1
    $attempts++
    
    if (Test-PortInUse $PORT) {
        $serverRunning = $true
        Write-Host "$($COLORS.GREEN)Server started successfully on port $PORT$($COLORS.RESET)"
    }
}

if (-not $serverRunning) {
    Write-Host "$($COLORS.RED)$($COLORS.BRIGHT)Failed to start server after $maxAttempts attempts$($COLORS.RESET)"
    Write-Host "$($COLORS.YELLOW)Please check for errors in the server window.$($COLORS.RESET)"
    exit 1
}

# Display next steps
Write-Host "`n$($COLORS.BRIGHT)$($COLORS.GREEN)Server is now running!$($COLORS.RESET)"
Write-Host "$($COLORS.CYAN)To simulate gameplay, run:$($COLORS.RESET)"
Write-Host "$($COLORS.YELLOW).\simulate-gameplay.ps1$($COLORS.RESET)"
Write-Host "`n$($COLORS.CYAN)The server is running in a separate window. Do not close it while testing.$($COLORS.RESET)" 