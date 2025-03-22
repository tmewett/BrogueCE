# Brogue DM AI - Gameplay Simulation Script
# This PowerShell script simulates a gameplay session with the DM AI

param(
    [string]$adventure = $null,  # Optional adventure to run
    [switch]$verbose = $false,   # Show detailed information
    [switch]$faster = $false     # Run the simulation faster
)

# Configuration
$DM_AGENT_DIR = Join-Path $PSScriptRoot "..\dm-agent"
$SERVER_URL = "http://localhost:3001"
$WAIT_TIME = if ($faster) { 2 } else { 5 }  # Seconds between events

# ANSI colors for terminal output
$ESC = [char]27
$COLORS = @{
    RESET = "$ESC[0m"
    BRIGHT = "$ESC[1m"
    DIM = "$ESC[2m"
    RED = "$ESC[31m"
    GREEN = "$ESC[32m"
    YELLOW = "$ESC[33m"
    BLUE = "$ESC[34m"
    MAGENTA = "$ESC[35m"
    CYAN = "$ESC[36m"
    WHITE = "$ESC[37m"
}

# Function to check if server is running
function Test-ServerRunning {
    try {
        $response = Invoke-RestMethod -Uri $SERVER_URL -Method Get -ErrorAction Stop
        return $true
    } catch {
        return $false
    }
}

# Function to send an event to the DM agent server
function Send-DmEvent {
    param (
        [string]$eventType,
        [hashtable]$eventData,
        [hashtable]$context
    )
    
    $body = @{
        eventType = $eventType
        eventData = $eventData
        context = $context
    } | ConvertTo-Json
    
    Write-Host "`n$($COLORS.BRIGHT)$($COLORS.YELLOW)=== $eventType Event ===$($COLORS.RESET)" 
    
    if ($verbose) {
        Write-Host "$($COLORS.DIM)Sending: $($body | ConvertTo-Json -Depth 3)$($COLORS.RESET)"
    } else {
        # Show a simplified version of the event
        $desc = ""
        switch ($eventType) {
            "MONSTER_ENCOUNTERED" { 
                $desc = "Encountered a $($eventData.monsterName) in $($eventData.locationDesc)" 
            }
            "MONSTER_KILLED" { 
                $desc = "Defeated a $($eventData.monsterName)" 
            }
            "ITEM_DISCOVERED" { 
                $desc = "Found a $($eventData.itemName)" 
            }
            "NEW_LEVEL" { 
                $desc = "Descended to depth $($eventData.depth): $($eventData.environmentType)" 
            }
            "PLAYER_DIED" { 
                $desc = "Player died to a $($eventData.killedBy) at depth $($eventData.maxDepth)" 
            }
        }
        Write-Host "$($COLORS.CYAN)$desc$($COLORS.RESET)"
    }
    
    try {
        Start-Sleep -Seconds 1  # Small pause before sending
        $response = Invoke-RestMethod -Uri "$SERVER_URL/api/event" -Method Post -Body $body -ContentType "application/json"
        
        if ($response.narrative) {
            Write-Host "`n$($COLORS.BRIGHT)$($COLORS.MAGENTA)Dungeon Master:$($COLORS.RESET)" 
            Write-Host "$($COLORS.MAGENTA)$($response.narrative)$($COLORS.RESET)`n"
        }
        return $true
    } catch {
        Write-Host "$($COLORS.RED)Error sending event: $_$($COLORS.RESET)"
        return $false
    }
}

# Adventure Scenarios
$ADVENTURES = @{
    "The Goblin Caves" = @(
        @{
            eventType = "NEW_LEVEL"
            eventData = @{
                depth = 1
                direction = 1  # Descending
                environmentType = "cave"
            }
            context = @{
                playerLevel = 1
                playerClass = "warrior"
                playerRace = "human"
            }
        },
        @{
            eventType = "MONSTER_ENCOUNTERED"
            eventData = @{
                monsterName = "goblin scout"
                monsterLevel = 1
                isRare = $false
                isFirstEncounter = $true
                locationDesc = "narrow passage"
            }
            context = @{
                playerLevel = 1
            }
        },
        @{
            eventType = "MONSTER_KILLED"
            eventData = @{
                monsterName = "goblin scout"
                monsterLevel = 1
                killedBy = "player"
                isRare = $false
            }
            context = @{
                playerLevel = 1
            }
        },
        @{
            eventType = "ITEM_DISCOVERED"
            eventData = @{
                itemName = "Dagger of Slaying"
                category = 1  # Weapon
                isRare = $true
            }
            context = @{
                playerLevel = 1
            }
        },
        @{
            eventType = "MONSTER_ENCOUNTERED"
            eventData = @{
                monsterName = "goblin chieftain"
                monsterLevel = 3
                isRare = $true
                isFirstEncounter = $true
                locationDesc = "throne room"
            }
            context = @{
                playerLevel = 1
            }
        },
        @{
            eventType = "PLAYER_DIED"
            eventData = @{
                killedBy = "goblin chieftain"
                totalTurns = 235
                maxDepth = 1
            }
            context = @{
                playerLevel = 1
                gold = 25
            }
        }
    )
    "The Crystal Caverns" = @(
        @{
            eventType = "NEW_LEVEL"
            eventData = @{
                depth = 3
                direction = 1  # Descending
                environmentType = "crystal cavern"
            }
            context = @{
                playerLevel = 3
                playerClass = "mage"
                playerRace = "elf"
            }
        },
        @{
            eventType = "MONSTER_ENCOUNTERED"
            eventData = @{
                monsterName = "crystal elemental"
                monsterLevel = 4
                isRare = $true
                isFirstEncounter = $true
                locationDesc = "glowing chamber"
            }
            context = @{
                playerLevel = 3
            }
        },
        @{
            eventType = "ITEM_DISCOVERED"
            eventData = @{
                itemName = "Staff of Lightning"
                category = 2  # Staff
                isRare = $true
            }
            context = @{
                playerLevel = 3
            }
        },
        @{
            eventType = "MONSTER_KILLED"
            eventData = @{
                monsterName = "crystal elemental"
                monsterLevel = 4
                killedBy = "player"
                isRare = $true
            }
            context = @{
                playerLevel = 3
            }
        },
        @{
            eventType = "NEW_LEVEL"
            eventData = @{
                depth = 4
                direction = 1  # Descending
                environmentType = "cavern"
            }
            context = @{
                playerLevel = 3
            }
        },
        @{
            eventType = "MONSTER_ENCOUNTERED"
            eventData = @{
                monsterName = "vampire bat"
                monsterLevel = 4
                isRare = $false
                isFirstEncounter = $true
                locationDesc = "dark corridor"
            }
            context = @{
                playerLevel = 3
            }
        },
        @{
            eventType = "MONSTER_KILLED"
            eventData = @{
                monsterName = "vampire bat"
                monsterLevel = 4
                killedBy = "player"
                isRare = $false
            }
            context = @{
                playerLevel = 4
            }
        },
        @{
            eventType = "MONSTER_ENCOUNTERED"
            eventData = @{
                monsterName = "dragon"
                monsterLevel = 10
                isRare = $true
                isFirstEncounter = $true
                locationDesc = "treasure chamber"
            }
            context = @{
                playerLevel = 8
            }
        },
        @{
            eventType = "PLAYER_DIED"
            eventData = @{
                killedBy = "dragon"
                totalTurns = 3456
                maxDepth = 12
            }
            context = @{
                playerLevel = 8
                gold = 780
            }
        }
    )
    "The Undead Catacombs" = @(
        @{
            eventType = "NEW_LEVEL"
            eventData = @{
                depth = 5
                direction = 1
                environmentType = "catacombs"
            }
            context = @{
                playerLevel = 6
                playerClass = "paladin"
                playerRace = "human"
            }
        },
        @{
            eventType = "MONSTER_ENCOUNTERED"
            eventData = @{
                monsterName = "skeletal warrior"
                monsterLevel = 5
                isRare = $false
                isFirstEncounter = $true
                locationDesc = "bone-littered hall"
            }
            context = @{
                playerLevel = 6
            }
        },
        @{
            eventType = "MONSTER_KILLED"
            eventData = @{
                monsterName = "skeletal warrior"
                monsterLevel = 5
                killedBy = "player"
                isRare = $false
            }
            context = @{
                playerLevel = 6
            }
        },
        @{
            eventType = "ITEM_DISCOVERED"
            eventData = @{
                itemName = "Holy Avenger"
                category = 1  # Weapon
                isRare = $true
            }
            context = @{
                playerLevel = 6
            }
        },
        @{
            eventType = "MONSTER_ENCOUNTERED"
            eventData = @{
                monsterName = "lich king"
                monsterLevel = 12
                isRare = $true
                isFirstEncounter = $true
                locationDesc = "ancient throne room"
            }
            context = @{
                playerLevel = 6
            }
        },
        @{
            eventType = "MONSTER_KILLED"
            eventData = @{
                monsterName = "lich king"
                monsterLevel = 12
                killedBy = "player"
                isRare = $true
            }
            context = @{
                playerLevel = 7
            }
        },
        @{
            eventType = "ITEM_DISCOVERED"
            eventData = @{
                itemName = "Crown of Souls"
                category = 3  # Artifact
                isRare = $true
            }
            context = @{
                playerLevel = 7
            }
        }
    )
}

# Main execution
Write-Host "$($COLORS.BRIGHT)$($COLORS.MAGENTA)BROGUE DM AI - GAMEPLAY SIMULATION$($COLORS.RESET)"
Write-Host "$($COLORS.DIM)This script simulates game events to test the DM agent.$($COLORS.RESET)`n"

# Check if server is running
if (-not (Test-ServerRunning)) {
    Write-Host "$($COLORS.RED)$($COLORS.BRIGHT)ERROR: DM agent server not running!$($COLORS.RESET)"
    Write-Host "Please start the server first with:`n$($COLORS.CYAN)cd BrogueMCP\playtest; .\start-server.ps1$($COLORS.RESET)"
    exit 1
}

Write-Host "$($COLORS.GREEN)DM agent server is running.$($COLORS.RESET)"

# If no adventure was specified, show a menu
if (-not $adventure) {
    Write-Host "`n$($COLORS.YELLOW)Please select an adventure:$($COLORS.RESET)"
    $i = 1
    $ADVENTURES.Keys | ForEach-Object {
        Write-Host "$($COLORS.CYAN)$i.$($COLORS.RESET) $_"
        $i++
    }
    
    $selection = Read-Host "`nEnter a number"
    if ($selection -match '^\d+$' -and [int]$selection -ge 1 -and [int]$selection -le $ADVENTURES.Count) {
        $adventure = @($ADVENTURES.Keys)[[int]$selection - 1]
    } else {
        # Default to first adventure if invalid selection
        $adventure = @($ADVENTURES.Keys)[0]
    }
}

# Make sure the adventure exists
if (-not $ADVENTURES.ContainsKey($adventure)) {
    Write-Host "$($COLORS.RED)Adventure '$adventure' not found. Using the first available adventure.$($COLORS.RESET)"
    $adventure = @($ADVENTURES.Keys)[0]
}

# Begin adventure
Write-Host "`n$($COLORS.BRIGHT)$($COLORS.GREEN)Starting adventure: $adventure$($COLORS.RESET)"
Write-Host "$($COLORS.DIM)Events will appear with $WAIT_TIME second delays to simulate gameplay.$($COLORS.RESET)`n"

$events = $ADVENTURES[$adventure]
for ($i = 0; $i -lt $events.Count; $i++) {
    $event = $events[$i]
    
    # Wait between events (except first one)
    if ($i -gt 0) {
        Write-Host "$($COLORS.DIM)Gameplay continues...$($COLORS.RESET)"
        Start-Sleep -Seconds $WAIT_TIME
    }
    
    # Send event to DM agent
    Send-DmEvent -eventType $event.eventType -eventData $event.eventData -context $event.context
}

Write-Host "`n$($COLORS.BRIGHT)$($COLORS.GREEN)Adventure complete!$($COLORS.RESET)"
Write-Host "$($COLORS.CYAN)View the full log by running: .\view-logs.ps1$($COLORS.RESET)" 