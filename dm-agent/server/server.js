/**
 * Server for Brogue Dungeon Master AI
 * 
 * Handles HTTP requests from the game and returns narrative responses
 */

const express = require('express');
const { OllamaInterface } = require('../ollama/interface');
const { MemoryManager } = require('../memory/manager');
const { NarrativeGenerator } = require('../narrative/generator');
const path = require('path');
const fs = require('fs');
require('dotenv').config();

// Initialize Express app
const app = express();
app.use(express.json());

// Serve static files from the public directory
app.use(express.static(path.join(__dirname, '../public')));

// Fancy console logging with colors
const COLORS = {
    RESET: '\x1b[0m',
    BRIGHT: '\x1b[1m',
    DIM: '\x1b[2m',
    RED: '\x1b[31m',
    GREEN: '\x1b[32m',
    YELLOW: '\x1b[33m',
    BLUE: '\x1b[34m',
    MAGENTA: '\x1b[35m',
    CYAN: '\x1b[36m',
    WHITE: '\x1b[37m',
    BG_BLACK: '\x1b[40m',
    BG_RED: '\x1b[41m',
    BG_GREEN: '\x1b[42m',
    BG_YELLOW: '\x1b[43m',
    BG_BLUE: '\x1b[44m',
    BG_MAGENTA: '\x1b[45m',
    BG_CYAN: '\x1b[46m',
    BG_WHITE: '\x1b[47m'
};

// Setup logging directory
const LOGS_DIR = path.join(__dirname, '../../playtest/logs');
if (!fs.existsSync(LOGS_DIR)) {
    fs.mkdirSync(LOGS_DIR, { recursive: true });
}

// Session logging
const sessionId = Date.now();
const logFile = path.join(LOGS_DIR, `session_${sessionId}.log`);
const eventLogFile = path.join(LOGS_DIR, `events_${sessionId}.md`);

// Initialize event log with a header
fs.writeFileSync(eventLogFile, `# Brogue DM Session ${new Date().toLocaleString()}\n\n`);

// Fancy logger function
function fancyLog(type, message, data = null) {
    let color, icon, bgColor = '';
    
    switch (type) {
        case 'EVENT':
            color = COLORS.YELLOW;
            icon = '🎲';
            break;
        case 'NARRATIVE':
            color = COLORS.CYAN;
            icon = '📜';
            break;
        case 'MONSTER':
            color = COLORS.RED;
            icon = '👹';
            break;
        case 'ITEM':
            color = COLORS.GREEN;
            icon = '💎';
            break;
        case 'PLAYER':
            color = COLORS.MAGENTA;
            icon = '🧙';
            break;
        case 'LEVEL':
            color = COLORS.BLUE;
            icon = '🗺️';
            break;
        case 'ERROR':
            color = COLORS.RED;
            bgColor = COLORS.BG_BLACK;
            icon = '❌';
            break;
        default:
            color = COLORS.WHITE;
            icon = '📢';
    }
    
    // Console output
    console.log(`${bgColor}${color}${COLORS.BRIGHT}${icon} [${type}] ${message}${COLORS.RESET}`);
    
    // Log to file
    const timestamp = new Date().toISOString();
    let logEntry = `[${timestamp}] [${type}] ${message}\n`;
    if (data) {
        logEntry += `DATA: ${JSON.stringify(data, null, 2)}\n`;
    }
    fs.appendFileSync(logFile, logEntry);
    
    // Create a more readable, themed log entry for the event log
    if (['EVENT', 'NARRATIVE', 'MONSTER', 'ITEM', 'PLAYER', 'LEVEL'].includes(type)) {
        const prettyTimestamp = new Date().toLocaleTimeString();
        let mdEntry = '';
        
        if (type === 'EVENT') {
            mdEntry = `## ${prettyTimestamp} - ${message}\n\n`;
        } else if (type === 'NARRATIVE') {
            mdEntry = `> *${message}*\n\n`;
        } else if (type === 'MONSTER') {
            mdEntry = `### ${icon} ${message}\n\n`;
        } else if (type === 'ITEM') {
            mdEntry = `### ${icon} ${message}\n\n`;
        } else if (type === 'PLAYER') {
            mdEntry = `### ${icon} ${message}\n\n`;
        } else if (type === 'LEVEL') {
            mdEntry = `## ${icon} ${message}\n\n`;
        }
        
        if (data) {
            mdEntry += "```json\n" + JSON.stringify(data, null, 2) + "\n```\n\n";
        }
        
        fs.appendFileSync(eventLogFile, mdEntry);
    }
}

// Initialize components
const ollama = new OllamaInterface({
    model: process.env.OLLAMA_MODEL || 'llama3',
    baseUrl: process.env.OLLAMA_URL || 'http://localhost:11434',
    temperature: parseFloat(process.env.OLLAMA_TEMPERATURE || '0.7'),
    maxTokens: parseInt(process.env.OLLAMA_MAX_TOKENS || '512')
});

const memoryManager = new MemoryManager(
    process.env.MEMORY_BANK_PATH || path.join(__dirname, '../../memory-bank')
);

const narrativeGenerator = new NarrativeGenerator(ollama, memoryManager);

fancyLog('EVENT', 'Dungeon Master AI Initialized', {
    model: ollama.model,
    memoryPath: memoryManager.memoryBankPath
});

// Root endpoint for health check
app.get('/', (req, res) => {
    fancyLog('EVENT', 'Health check requested');
    res.json({
        status: 'ok',
        message: 'Brogue Dungeon Master AI server is running',
        sessionId: sessionId,
        logFile: logFile
    });
});

// Direct route for narrator settings UI
app.get('/narrator', (req, res) => {
    res.sendFile(path.join(__dirname, '../public/narrator.html'));
    fancyLog('EVENT', 'Narrator settings UI requested');
});

// Process game events
app.post('/api/event', async (req, res) => {
    try {
        const { eventType, eventData, context } = req.body;
        
        // Validate event data
        if (!eventType) {
            fancyLog('ERROR', 'Missing eventType in request');
            return res.status(400).json({
                status: 'error',
                message: 'Missing eventType in request'
            });
        }
        
        // Log the event appropriately
        let logType = 'EVENT';
        let logMessage = `Received ${eventType}`;
        
        switch (eventType) {
            case 'MONSTER_ENCOUNTERED':
                logType = 'MONSTER';
                logMessage = `Encountered ${eventData.monsterName} in ${eventData.locationDesc || 'the dungeon'}`;
                break;
            case 'MONSTER_KILLED':
                logType = 'MONSTER';
                logMessage = `Defeated ${eventData.monsterName}!`;
                break;
            case 'ITEM_DISCOVERED':
                logType = 'ITEM';
                logMessage = `Found ${eventData.itemName}`;
                break;
            case 'PLAYER_DIED':
                logType = 'PLAYER';
                logMessage = `Player died, killed by ${eventData.killedBy}`;
                break;
            case 'NEW_LEVEL':
                logType = 'LEVEL';
                logMessage = `Descended to depth ${eventData.depth}, a ${eventData.environmentType || 'dungeon'} area`;
                break;
        }
        
        fancyLog(logType, logMessage, eventData);
        
        // Record event in memory
        memoryManager.recordEvent(eventType, eventData, context);
        
        // Determine if this event needs narrative enhancement
        if (shouldEnhanceEvent(eventType, eventData)) {
            fancyLog('EVENT', `Enhancing event: ${eventType}`);
            
            // Generate narrative response
            const narrativeResponse = await narrativeGenerator.generateResponse(
                eventType, 
                eventData, 
                context
            );
            
            fancyLog('NARRATIVE', narrativeResponse);
            
            return res.json({
                status: 'success',
                narrative: narrativeResponse
            });
        }
        
        // Event doesn't need narrative response
        return res.json({
            status: 'success',
            message: 'Event recorded'
        });
    } catch (error) {
        fancyLog('ERROR', `Error processing event: ${error.message}`, error);
        return res.status(500).json({
            status: 'error',
            message: 'Internal server error'
        });
    }
});

// Determine if an event should be enhanced with narrative
function shouldEnhanceEvent(eventType, eventData) {
    // Major events always get enhanced
    const majorEvents = [
        'PLAYER_DIED', 
        'NEW_LEVEL'
    ];
    
    if (majorEvents.includes(eventType)) {
        return true;
    }
    
    // Check for special criteria
    switch (eventType) {
        case 'MONSTER_ENCOUNTERED':
            return eventData.isFirstEncounter === true || eventData.isRare === true;
            
        case 'MONSTER_KILLED':
            return eventData.isRare === true;
            
        case 'ITEM_DISCOVERED':
            return eventData.isRare === true;
            
        default:
            return false;
    }
}

// Add endpoint to get logs
app.get('/api/logs/current', (req, res) => {
    try {
        if (fs.existsSync(eventLogFile)) {
            const logContent = fs.readFileSync(eventLogFile, 'utf8');
            res.type('text/markdown').send(logContent);
        } else {
            res.status(404).json({
                status: 'error',
                message: 'Log file not found'
            });
        }
    } catch (error) {
        fancyLog('ERROR', `Error serving log file: ${error.message}`);
        res.status(500).json({
            status: 'error',
            message: 'Failed to read log file'
        });
    }
});

// Add endpoints for narrator settings
app.get('/api/narrator/settings', (req, res) => {
    try {
        const settings = narrativeGenerator.getNarratorSettings();
        const currentPersonality = settings.getCurrentPersonality();
        const presetName = settings.getCurrentPresetName();
        
        res.json({
            status: 'success',
            presetName: presetName,
            attributes: currentPersonality.attributes,
            signaturePhrases: currentPersonality.signaturePhrases,
            availablePresets: Object.keys(settings.getAllPresets())
        });
    } catch (error) {
        fancyLog('ERROR', `Error getting narrator settings: ${error.message}`);
        res.status(500).json({
            status: 'error',
            message: 'Failed to get narrator settings'
        });
    }
});

app.post('/api/narrator/settings', (req, res) => {
    try {
        const { action, presetName, attributeName, attributeValue, phrase, phraseIndex } = req.body;
        const settings = narrativeGenerator.getNarratorSettings();
        
        let result = false;
        
        switch (action) {
            case 'applyPreset':
                if (presetName) {
                    result = settings.applyPreset(presetName);
                    fancyLog('EVENT', `Applied narrator preset: ${presetName}`);
                }
                break;
                
            case 'savePreset':
                if (presetName) {
                    settings.saveCustomPreset(presetName);
                    result = true;
                    fancyLog('EVENT', `Saved custom narrator preset: ${presetName}`);
                }
                break;
                
            case 'deletePreset':
                if (presetName) {
                    result = settings.deleteCustomPreset(presetName);
                    fancyLog('EVENT', `Deleted custom narrator preset: ${presetName}`);
                }
                break;
                
            case 'setAttribute':
                if (attributeName && attributeValue !== undefined) {
                    result = settings.setAttribute(attributeName, attributeValue);
                    fancyLog('EVENT', `Set narrator attribute ${attributeName} to ${attributeValue}`);
                }
                break;
                
            case 'addPhrase':
                if (phrase) {
                    result = settings.addSignaturePhrase(phrase);
                    fancyLog('EVENT', `Added signature phrase: "${phrase}"`);
                }
                break;
                
            case 'removePhrase':
                if (phraseIndex !== undefined) {
                    result = settings.removeSignaturePhrase(phraseIndex);
                    fancyLog('EVENT', `Removed signature phrase at index ${phraseIndex}`);
                }
                break;
                
            default:
                return res.status(400).json({
                    status: 'error',
                    message: 'Unknown action'
                });
        }
        
        if (result) {
            res.json({
                status: 'success',
                message: 'Narrator settings updated',
                currentPreset: settings.getCurrentPresetName(),
                attributes: settings.getCurrentPersonality().attributes,
                signaturePhrases: settings.getCurrentPersonality().signaturePhrases
            });
        } else {
            res.status(400).json({
                status: 'error',
                message: 'Failed to update settings'
            });
        }
    } catch (error) {
        fancyLog('ERROR', `Error updating narrator settings: ${error.message}`);
        res.status(500).json({
            status: 'error',
            message: 'Failed to update narrator settings'
        });
    }
});

// Get HTML for the narrator settings UI
app.get('/api/narrator/ui', (req, res) => {
    try {
        const settings = narrativeGenerator.getNarratorSettings();
        const html = settings.generateSettingsHTML();
        
        res.type('text/html').send(html);
    } catch (error) {
        fancyLog('ERROR', `Error generating narrator UI: ${error.message}`);
        res.status(500).json({
            status: 'error',
            message: 'Failed to generate narrator UI'
        });
    }
});

// Start server
const PORT = parseInt(process.env.PORT || '3001');
app.listen(PORT, () => {
    fancyLog('EVENT', `Dungeon Master AI server running on port ${PORT}`, {
        ollamaModel: ollama.model,
        memoryBankPath: memoryManager.memoryBankPath
    });
}); 