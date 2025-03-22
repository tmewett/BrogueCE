/**
 * Test script for Brogue Dungeon Master AI
 * 
 * Simulates game events and tests the server response
 */

const axios = require('axios');
const path = require('path');
const fs = require('fs');
require('dotenv').config();

// Server URL
const SERVER_URL = 'http://localhost:3001';

// Get any command line arguments
const args = process.argv.slice(2);
const VERBOSE = args.includes('--verbose') || args.includes('-v');
const WAIT_TIME = args.includes('--faster') ? 1000 : 3000; // Time between events

// ANSI colors for terminal output
const COLORS = {
    RESET: '\x1b[0m',
    BRIGHT: '\x1b[1m',
    RED: '\x1b[31m',
    GREEN: '\x1b[32m',
    YELLOW: '\x1b[33m',
    BLUE: '\x1b[34m',
    MAGENTA: '\x1b[35m',
    CYAN: '\x1b[36m'
};

// Test data for adventures
const TEST_ADVENTURES = [
    {
        name: "The Goblin Caves",
        events: [
            {
                eventType: 'NEW_LEVEL',
                eventData: {
                    depth: 1,
                    direction: 1, // Descending
                    environmentType: 'cave'
                },
                context: {
                    playerLevel: 1,
                    playerClass: 'warrior',
                    playerRace: 'human'
                }
            },
            {
                eventType: 'MONSTER_ENCOUNTERED',
                eventData: {
                    monsterName: 'goblin scout',
                    monsterLevel: 1,
                    isRare: false,
                    isFirstEncounter: true,
                    locationDesc: 'narrow passage'
                },
                context: {
                    playerLevel: 1
                }
            },
            {
                eventType: 'MONSTER_KILLED',
                eventData: {
                    monsterName: 'goblin scout',
                    monsterLevel: 1,
                    killedBy: 'player',
                    isRare: false
                },
                context: {
                    playerLevel: 1
                }
            },
            {
                eventType: 'ITEM_DISCOVERED',
                eventData: {
                    itemName: 'Dagger of Slaying',
                    category: 1, // Weapon
                    isRare: true
                },
                context: {
                    playerLevel: 1
                }
            },
            {
                eventType: 'MONSTER_ENCOUNTERED',
                eventData: {
                    monsterName: 'goblin chieftain',
                    monsterLevel: 3,
                    isRare: true,
                    isFirstEncounter: true,
                    locationDesc: 'throne room'
                },
                context: {
                    playerLevel: 1
                }
            },
            {
                eventType: 'PLAYER_DIED',
                eventData: {
                    killedBy: 'goblin chieftain',
                    totalTurns: 235,
                    maxDepth: 1
                },
                context: {
                    playerLevel: 1,
                    gold: 25
                }
            }
        ]
    },
    {
        name: "The Crystal Caverns",
        events: [
            {
                eventType: 'NEW_LEVEL',
                eventData: {
                    depth: 3,
                    direction: 1, // Descending
                    environmentType: 'crystal cavern'
                },
                context: {
                    playerLevel: 3,
                    playerClass: 'mage',
                    playerRace: 'elf'
                }
            },
            {
                eventType: 'MONSTER_ENCOUNTERED',
                eventData: {
                    monsterName: 'crystal elemental',
                    monsterLevel: 4,
                    isRare: true,
                    isFirstEncounter: true,
                    locationDesc: 'glowing chamber'
                },
                context: {
                    playerLevel: 3
                }
            },
            {
                eventType: 'ITEM_DISCOVERED',
                eventData: {
                    itemName: 'Staff of Lightning',
                    category: 2, // Staff
                    isRare: true
                },
                context: {
                    playerLevel: 3
                }
            },
            {
                eventType: 'MONSTER_KILLED',
                eventData: {
                    monsterName: 'crystal elemental',
                    monsterLevel: 4,
                    killedBy: 'player',
                    isRare: true
                },
                context: {
                    playerLevel: 3
                }
            },
            {
                eventType: 'NEW_LEVEL',
                eventData: {
                    depth: 4,
                    direction: 1, // Descending
                    environmentType: 'cavern'
                },
                context: {
                    playerLevel: 3
                }
            },
            {
                eventType: 'MONSTER_ENCOUNTERED',
                eventData: {
                    monsterName: 'vampire bat',
                    monsterLevel: 4,
                    isRare: false,
                    isFirstEncounter: true,
                    locationDesc: 'dark corridor'
                },
                context: {
                    playerLevel: 3
                }
            },
            {
                eventType: 'MONSTER_KILLED',
                eventData: {
                    monsterName: 'vampire bat',
                    monsterLevel: 4,
                    killedBy: 'player',
                    isRare: false
                },
                context: {
                    playerLevel: 4
                }
            },
            {
                eventType: 'MONSTER_ENCOUNTERED',
                eventData: {
                    monsterName: 'dragon',
                    monsterLevel: 10,
                    isRare: true,
                    isFirstEncounter: true,
                    locationDesc: 'treasure chamber'
                },
                context: {
                    playerLevel: 8
                }
            },
            {
                eventType: 'PLAYER_DIED',
                eventData: {
                    killedBy: 'dragon',
                    totalTurns: 3456,
                    maxDepth: 12
                },
                context: {
                    playerLevel: 8,
                    gold: 780
                }
            }
        ]
    },
    {
        name: "The Undead Catacombs",
        events: [
            {
                eventType: 'NEW_LEVEL',
                eventData: {
                    depth: 5,
                    direction: 1,
                    environmentType: 'catacombs'
                },
                context: {
                    playerLevel: 6,
                    playerClass: 'paladin',
                    playerRace: 'human'
                }
            },
            {
                eventType: 'MONSTER_ENCOUNTERED',
                eventData: {
                    monsterName: 'skeletal warrior',
                    monsterLevel: 5,
                    isRare: false,
                    isFirstEncounter: true,
                    locationDesc: 'bone-littered hall'
                },
                context: {
                    playerLevel: 6
                }
            },
            {
                eventType: 'MONSTER_KILLED',
                eventData: {
                    monsterName: 'skeletal warrior',
                    monsterLevel: 5,
                    killedBy: 'player',
                    isRare: false
                },
                context: {
                    playerLevel: 6
                }
            },
            {
                eventType: 'ITEM_DISCOVERED',
                eventData: {
                    itemName: 'Holy Avenger',
                    category: 1, // Weapon
                    isRare: true
                },
                context: {
                    playerLevel: 6
                }
            },
            {
                eventType: 'MONSTER_ENCOUNTERED',
                eventData: {
                    monsterName: 'lich king',
                    monsterLevel: 12,
                    isRare: true,
                    isFirstEncounter: true,
                    locationDesc: 'ancient throne room'
                },
                context: {
                    playerLevel: 6
                }
            },
            {
                eventType: 'MONSTER_KILLED',
                eventData: {
                    monsterName: 'lich king',
                    monsterLevel: 12,
                    killedBy: 'player',
                    isRare: true
                },
                context: {
                    playerLevel: 7
                }
            },
            {
                eventType: 'ITEM_DISCOVERED',
                eventData: {
                    itemName: 'Crown of Souls',
                    category: 3, // Artifact
                    isRare: true
                },
                context: {
                    playerLevel: 7
                }
            }
        ]
    }
];

// Utility function to wait X milliseconds
function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

// Main function to run tests
async function runTests() {
    console.log(`\n${COLORS.BRIGHT}${COLORS.YELLOW}=== Brogue Dungeon Master AI Test ====${COLORS.RESET}\n`);
    
    try {
        // Check if server is running
        console.log(`${COLORS.CYAN}Checking server status...${COLORS.RESET}`);
        const response = await axios.get(SERVER_URL);
        console.log(`${COLORS.GREEN}Server is running! Status: ${response.data.status}${COLORS.RESET}`);
        console.log(`Session ID: ${response.data.sessionId}`);
        console.log(`Log file: ${response.data.logFile}`);
        
        // Select an adventure to test
        const adventureIndex = Math.floor(Math.random() * TEST_ADVENTURES.length);
        const adventure = TEST_ADVENTURES[adventureIndex];
        
        console.log(`\n${COLORS.BRIGHT}${COLORS.MAGENTA}Starting adventure: ${adventure.name}${COLORS.RESET}\n`);
        await sleep(WAIT_TIME / 2);
        
        // Process each event in the adventure
        for (const [index, event] of adventure.events.entries()) {
            console.log(`\n${COLORS.YELLOW}Event ${index + 1}/${adventure.events.length}: ${event.eventType}${COLORS.RESET}`);
            
            if (VERBOSE) {
                console.log(`Data: ${JSON.stringify(event.eventData, null, 2)}`);
            }
            
            try {
                await sleep(WAIT_TIME); // Wait between events for dramatic effect
                
                const eventResponse = await axios.post(`${SERVER_URL}/api/event`, event);
                
                if (eventResponse.data.narrative) {
                    console.log(`\n${COLORS.BRIGHT}${COLORS.CYAN}### Dungeon Master Narrates:${COLORS.RESET}`);
                    console.log(`${COLORS.CYAN}${eventResponse.data.narrative}${COLORS.RESET}\n`);
                }
            } catch (error) {
                console.error(`${COLORS.RED}Error sending event: ${error.message}${COLORS.RESET}`);
            }
        }
        
        console.log(`\n${COLORS.BRIGHT}${COLORS.GREEN}==== Adventure Complete! =====${COLORS.RESET}\n`);
        
        // Fetch and display the log
        try {
            console.log(`${COLORS.CYAN}Fetching event log...${COLORS.RESET}`);
            const logResponse = await axios.get(`${SERVER_URL}/api/logs/current`);
            console.log(`\n${COLORS.YELLOW}Log is available at: ${SERVER_URL}/api/logs/current${COLORS.RESET}\n`);
        } catch (error) {
            console.error(`${COLORS.RED}Error fetching log: ${error.message}${COLORS.RESET}`);
        }
        
    } catch (error) {
        console.error(`${COLORS.RED}${COLORS.BRIGHT}ERROR: ${error.message}${COLORS.RESET}`);
        console.error(`${COLORS.RED}Is the server running? Please start it with 'npm start' from the dm-agent directory.${COLORS.RESET}`);
    }
}

// Run the tests
runTests(); 