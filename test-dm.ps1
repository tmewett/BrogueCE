# Brogue Dungeon Master AI Test Script for Windows
# This PowerShell script tests the DM agent by:
# 1. Starting the server
# 2. Running test events
# 3. Capturing and displaying the logs

# Configuration
$SERVER_DIR = Join-Path $PSScriptRoot "dm-agent"
$SERVER_SCRIPT = Join-Path $SERVER_DIR "server/server.js"
$TEST_SCRIPT = Join-Path $SERVER_DIR "test.js"
$LOGS_DIR = Join-Path $PSScriptRoot "playtest/logs"
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
    WHITE = "$ESC[37m"
}

# Create logs directory if it doesn't exist
if (-not (Test-Path $LOGS_DIR)) {
    Write-Host "$($COLORS.CYAN)Creating logs directory at $LOGS_DIR$($COLORS.RESET)"
    New-Item -Path $LOGS_DIR -ItemType Directory -Force | Out-Null
}

# Check if Node.js is installed
try {
    $nodeVersion = node --version
    Write-Host "$($COLORS.GREEN)Node.js found: $nodeVersion$($COLORS.RESET)"
} catch {
    Write-Host "$($COLORS.RED)$($COLORS.BRIGHT)ERROR: Node.js not found. Please install Node.js to run this test.$($COLORS.RESET)"
    exit 1
}

# Check if required npm packages are installed
if (-not (Test-Path (Join-Path $SERVER_DIR "node_modules"))) {
    Write-Host "$($COLORS.YELLOW)Installing npm packages...$($COLORS.RESET)"
    Set-Location $SERVER_DIR
    npm install
    Set-Location $PSScriptRoot
}

# Function to check if port is in use
function Test-PortInUse {
    param ($port)
    
    $connection = Test-NetConnection -ComputerName localhost -Port $port -ErrorAction SilentlyContinue -WarningAction SilentlyContinue
    return $connection.TcpTestSucceeded
}

# Function to find the most recent log file
function Get-LatestLogFile {
    $logFiles = Get-ChildItem -Path $LOGS_DIR -Filter "events_*.md" | Sort-Object LastWriteTime -Descending
    if ($logFiles.Count -gt 0) {
        return $logFiles[0].FullName
    }
    return $null
}

# Check if server is already running
if (Test-PortInUse $PORT) {
    Write-Host "$($COLORS.YELLOW)$($COLORS.BRIGHT)Server already running on port $PORT$($COLORS.RESET)"
} else {
    # Start the server in a new PowerShell window
    Write-Host "$($COLORS.CYAN)Starting DM Agent server...$($COLORS.RESET)"
    Start-Process powershell -ArgumentList "-NoExit", "-Command", "cd '$SERVER_DIR'; node server/server.js"
    
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
        exit 1
    }
}

# Run the tests
Write-Host "$($COLORS.BRIGHT)$($COLORS.MAGENTA)Running DM Agent tests...$($COLORS.RESET)"

# Ask if user wants to see verbose output
$verbose = Read-Host "Show verbose test output? (y/n)"
$verboseArg = if ($verbose -eq "y") { "--verbose" } else { "" }

# Ask if user wants faster test execution
$faster = Read-Host "Run tests faster? (y/n)"
$fasterArg = if ($faster -eq "y") { "--faster" } else { "" }

# Run tests with node
Set-Location $SERVER_DIR
node $TEST_SCRIPT $verboseArg $fasterArg
Set-Location $PSScriptRoot

# Display log file path
$latestLog = Get-LatestLogFile
if ($latestLog) {
    Write-Host "$($COLORS.GREEN)$($COLORS.BRIGHT)Test complete! Log file created at:$($COLORS.RESET)"
    Write-Host "$($COLORS.CYAN)$latestLog$($COLORS.RESET)"
    
    # Ask if user wants to view the log
    $viewLog = Read-Host "Open log file? (y/n)"
    if ($viewLog -eq "y") {
        # Open the log file
        Invoke-Item $latestLog
    }
} else {
    Write-Host "$($COLORS.YELLOW)No log file found.$($COLORS.RESET)"
}

Write-Host "$($COLORS.GREEN)$($COLORS.BRIGHT)Test script complete!$($COLORS.RESET)"
Write-Host "$($COLORS.CYAN)The DM Agent server is still running in the other window.$($COLORS.RESET)"
Write-Host "$($COLORS.CYAN)Close that window when you're done testing.$($COLORS.RESET)" 