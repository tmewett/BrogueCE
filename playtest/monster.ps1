# Brogue DM AI - Monster Encounter Event
# Triggers a monster encounter event with the DM AI

param(
    [Parameter(Mandatory=$true)]
    [string]$name,
    
    [int]$level = 1,
    
    [string]$location = "dark chamber",
    
    [bool]$rare = $false,
    
    [bool]$firstEncounter = $true,
    
    [int]$playerLevel = 1
)

# Determine full server URL
$SERVER_URL = "http://localhost:3001/api/event"

# Create event data
$eventData = @{
    monsterName = $name
    monsterLevel = $level
    isRare = $rare
    isFirstEncounter = $firstEncounter
    locationDesc = $location
}

$context = @{
    playerLevel = $playerLevel
}

$body = @{
    eventType = "MONSTER_ENCOUNTERED"
    eventData = $eventData
    context = $context
} | ConvertTo-Json

# Display pretty info about what we're doing
Write-Host "`n===== MONSTER ENCOUNTER EVENT =====" -ForegroundColor Yellow
Write-Host "Player encounters a $name (level $level) in a $location" -ForegroundColor Cyan
if ($rare) {
    Write-Host "This is a RARE monster!" -ForegroundColor Magenta
}
if ($firstEncounter) {
    Write-Host "This is the first time encountering this creature." -ForegroundColor Cyan
}

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