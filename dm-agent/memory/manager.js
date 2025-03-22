/**
 * Memory Manager for Brogue Dungeon Master AI
 * 
 * Manages storage and retrieval of game events and knowledge
 */

const fs = require('fs');
const path = require('path');

class MemoryManager {
    constructor(memoryBankPath = './memory-bank') {
        this.memoryBankPath = memoryBankPath;
        this.shortTermMemory = [];
        this.maxShortTermMemories = 20;
        
        // Create memory bank directory if it doesn't exist
        this.ensureMemoryDirectory();
        
        console.log(`Memory manager initialized with path: ${this.memoryBankPath}`);
    }
    
    /**
     * Ensure memory bank directory exists
     */
    ensureMemoryDirectory() {
        if (!fs.existsSync(this.memoryBankPath)) {
            try {
                fs.mkdirSync(this.memoryBankPath, { recursive: true });
                
                // Create subdirectories
                const subdirs = ['creatures', 'items', 'events', 'player'];
                for (const dir of subdirs) {
                    fs.mkdirSync(path.join(this.memoryBankPath, dir), { recursive: true });
                }
                
                console.log(`Created memory bank directory at ${this.memoryBankPath}`);
            } catch (error) {
                console.error(`Failed to create memory bank directory: ${error.message}`);
            }
        }
    }
    
    /**
     * Record a game event
     */
    recordEvent(eventType, eventData, context = {}) {
        console.log(`Recording event: ${eventType}`);
        
        // Create memory entry
        const memoryEntry = {
            timestamp: Date.now(),
            eventType,
            eventData,
            context
        };
        
        // Add to short-term memory
        this.shortTermMemory.unshift(memoryEntry);
        
        // Trim short-term memory if needed
        if (this.shortTermMemory.length > this.maxShortTermMemories) {
            this.shortTermMemory.pop();
        }
        
        // Store significant events in long-term memory
        if (this.isSignificantEvent(eventType, eventData)) {
            this.storeInLongTermMemory(memoryEntry);
        }
        
        // Update knowledge bases
        this.updateKnowledgeBases(eventType, eventData);
    }
    
    /**
     * Determine if an event is significant enough for long-term storage
     */
    isSignificantEvent(eventType, eventData) {
        // Define criteria for significant events
        const significantEvents = [
            'PLAYER_DIED',
            'NEW_LEVEL',
            'MONSTER_ENCOUNTERED'
        ];
        
        // Check if it's in our list of significant events
        if (significantEvents.includes(eventType)) {
            return true;
        }
        
        // Check for special criteria
        if (eventType === 'MONSTER_KILLED' && eventData.isRare) {
            return true;
        }
        
        if (eventType === 'ITEM_DISCOVERED' && eventData.isRare) {
            return true;
        }
        
        return false;
    }
    
    /**
     * Store event in long-term memory
     */
    storeInLongTermMemory(memoryEntry) {
        try {
            // Create filename based on timestamp and event type
            const filename = `${memoryEntry.timestamp}_${memoryEntry.eventType}.json`;
            const filePath = path.join(this.memoryBankPath, 'events', filename);
            
            // Write to file
            fs.writeFileSync(filePath, JSON.stringify(memoryEntry, null, 2));
            console.log(`Stored event in long-term memory: ${filename}`);
        } catch (error) {
            console.error(`Failed to store event in long-term memory: ${error.message}`);
        }
    }
    
    /**
     * Update knowledge bases based on events
     */
    updateKnowledgeBases(eventType, eventData) {
        if (eventType === 'MONSTER_ENCOUNTERED' || eventType === 'MONSTER_KILLED') {
            this.updateCreatureKnowledge(eventData);
        } else if (eventType === 'ITEM_DISCOVERED') {
            this.updateItemKnowledge(eventData);
        }
    }
    
    /**
     * Update creature knowledge
     */
    updateCreatureKnowledge(eventData) {
        if (!eventData.monsterName) return;
        
        try {
            const creaturePath = path.join(this.memoryBankPath, 'creatures', `${eventData.monsterName}.json`);
            
            // Load existing data if it exists
            let creatureData = {};
            if (fs.existsSync(creaturePath)) {
                const fileContent = fs.readFileSync(creaturePath, 'utf-8');
                creatureData = JSON.parse(fileContent);
                
                // Update existing data
                creatureData.lastSeen = Date.now();
                creatureData.encounterCount = (creatureData.encounterCount || 0) + 1;
                
                // Merge new information
                Object.assign(creatureData, eventData);
            } else {
                // Create new entry
                creatureData = {
                    ...eventData,
                    firstSeen: Date.now(),
                    lastSeen: Date.now(),
                    encounterCount: 1
                };
            }
            
            // Write back to file
            fs.writeFileSync(creaturePath, JSON.stringify(creatureData, null, 2));
            console.log(`Updated creature knowledge: ${eventData.monsterName}`);
        } catch (error) {
            console.error(`Failed to update creature knowledge: ${error.message}`);
        }
    }
    
    /**
     * Update item knowledge
     */
    updateItemKnowledge(eventData) {
        if (!eventData.itemName) return;
        
        try {
            const itemPath = path.join(this.memoryBankPath, 'items', `${eventData.itemName}.json`);
            
            // Load existing data if it exists
            let itemData = {};
            if (fs.existsSync(itemPath)) {
                const fileContent = fs.readFileSync(itemPath, 'utf-8');
                itemData = JSON.parse(fileContent);
                
                // Update existing data
                itemData.lastSeen = Date.now();
                itemData.encounterCount = (itemData.encounterCount || 0) + 1;
                
                // Merge new information
                Object.assign(itemData, eventData);
            } else {
                // Create new entry
                itemData = {
                    ...eventData,
                    firstSeen: Date.now(),
                    lastSeen: Date.now(),
                    encounterCount: 1
                };
            }
            
            // Write back to file
            fs.writeFileSync(itemPath, JSON.stringify(itemData, null, 2));
            console.log(`Updated item knowledge: ${eventData.itemName}`);
        } catch (error) {
            console.error(`Failed to update item knowledge: ${error.message}`);
        }
    }
    
    /**
     * Get recent memories for context building
     */
    getRecentMemories(count = 5) {
        return this.shortTermMemory.slice(0, count);
    }
    
    /**
     * Get creature knowledge
     */
    getCreatureKnowledge(creatureName) {
        try {
            const creaturePath = path.join(this.memoryBankPath, 'creatures', `${creatureName}.json`);
            
            if (fs.existsSync(creaturePath)) {
                const fileContent = fs.readFileSync(creaturePath, 'utf-8');
                return JSON.parse(fileContent);
            }
        } catch (error) {
            console.error(`Failed to get creature knowledge: ${error.message}`);
        }
        
        return null;
    }
    
    /**
     * Get item knowledge
     */
    getItemKnowledge(itemName) {
        try {
            const itemPath = path.join(this.memoryBankPath, 'items', `${itemName}.json`);
            
            if (fs.existsSync(itemPath)) {
                const fileContent = fs.readFileSync(itemPath, 'utf-8');
                return JSON.parse(fileContent);
            }
        } catch (error) {
            console.error(`Failed to get item knowledge: ${error.message}`);
        }
        
        return null;
    }
}

module.exports = { MemoryManager }; 