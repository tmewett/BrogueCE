# Brogue DM AI - Level Transition Event
# Triggers a level transition event with the DM AI

param(
    [Parameter(Mandatory=$true)]
    [int]$depth,
    
    [string]$environment = "dungeon",
    
    [int]$direction = 1,  # 1=descending, 0=same level, -1=ascending
    
    [int]$playerLevel = 1
)

# Determine full server URL
$SERVER_URL = "http://localhost:3001/api/event"

# Create event data
$eventData = @{
    depth = $depth
    direction = $direction
    environmentType = $environment
}

$context = @{
    playerLevel = $playerLevel
}

$body = @{
    eventType = "NEW_LEVEL"
    eventData = $eventData
    context = $context
} | ConvertTo-Json

# Display pretty info about what we're doing
Write-Host "`n===== LEVEL TRANSITION EVENT =====" -ForegroundColor Yellow

# Determine direction text
$directionText = switch ($direction) {
    1 { "descending to" }
    -1 { "ascending to" }
    default { "moving to" }
}

Write-Host "Player is $directionText depth $depth: $environment" -ForegroundColor Cyan

# Send the event
try {
    Write-Host "`nSending event to DM Agent..." -ForegroundColor Gray
    $response = Invoke-RestMethod -Uri $SERVER_URL -Method Post -Body $body -ContentType "application/json"
    
    if ($response.narrative) {
        Write-Host "`n[Dungeon Master]" -ForegroundColor Magenta
        Write-Host $response.narrative -ForegroundColor White
    } else {
        Write-Host "`nNo narrative response received." -ForegroundColor Yellow
    }
} catch {
    Write-Host "Error: $_" -ForegroundColor Red
    Write-Host "Make sure the DM Agent server is running (run start-server.ps1 first)" -ForegroundColor Red
}

Write-Host "`n" 