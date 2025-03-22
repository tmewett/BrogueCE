# Brogue DM AI - Item Discovery Event
# Triggers an item discovery event with the DM AI

param(
    [Parameter(Mandatory=$true)]
    [string]$name,
    
    [int]$category = 1,  # 1=weapon, 2=staff, 3=artifact, 4=potion, 5=scroll
    
    [bool]$rare = $false,
    
    [int]$playerLevel = 1
)

# Determine full server URL
$SERVER_URL = "http://localhost:3001/api/event"

# Create event data
$eventData = @{
    itemName = $name
    category = $category
    isRare = $rare
}

$context = @{
    playerLevel = $playerLevel
}

$body = @{
    eventType = "ITEM_DISCOVERED"
    eventData = $eventData
    context = $context
} | ConvertTo-Json

# Display pretty info about what we're doing
Write-Host "`n===== ITEM DISCOVERY EVENT =====" -ForegroundColor Yellow

# Determine category name
$categoryName = switch ($category) {
    1 { "Weapon" }
    2 { "Staff" }
    3 { "Artifact" }
    4 { "Potion" }
    5 { "Scroll" }
    default { "Item" }
}

Write-Host "Player discovers a $name ($categoryName)" -ForegroundColor Cyan
if ($rare) {
    Write-Host "This is a RARE item!" -ForegroundColor Magenta
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