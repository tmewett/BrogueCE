/**
 * Narrative Generator for Brogue Dungeon Master AI
 * 
 * Generates narrative responses based on game events using Ollama
 */

const axios = require('axios');
const fs = require('fs');
const path = require('path');
const { NarratorSettings } = require('./settings');

class NarrativeGenerator {
    constructor(ollamaInterface, memoryManager) {
        this.ollama = ollamaInterface;
        this.memory = memoryManager;
        this.narratorSettings = new NarratorSettings();
        
        // Load fallback messages
        try {
            const fallbackPath = path.join(__dirname, 'fallbacks.json');
            if (fs.existsSync(fallbackPath)) {
                this.fallbackMessages = JSON.parse(fs.readFileSync(fallbackPath, 'utf8'));
                console.log('Loaded fallback narrative messages');
            } else {
                console.log('No fallback messages file found at', fallbackPath);
                this.fallbackMessages = {};
            }
        } catch (error) {
            console.error('Error loading fallback messages:', error.message);
            this.fallbackMessages = {};
        }
        
        // System prompts for different event types
        this.systemPrompts = {
            MONSTER_ENCOUNTERED: `You are the Dungeon Master AI for a roguelike game called Brogue. 
                Generate a brief (1-2 sentences), atmospheric description of the monster encounter. 
                Focus on creating tension and immersion. Don't be verbose.`,
                
            ITEM_DISCOVERED: `You are the Dungeon Master AI for a roguelike game called Brogue.
                Generate a brief, intriguing description of the item discovery.
                Focus on the item's appearance, possible history, or magical aura.
                Keep it to 1-2 sentences maximum.`,
                
            PLAYER_DIED: `You are the Dungeon Master AI for a roguelike game called Brogue.
                The player has died. Generate a short, atmospheric epitaph that mentions
                how they died and acknowledges their achievements. Maximum 3 sentences.`,
                
            NEW_LEVEL: `You are the Dungeon Master AI for a roguelike game called Brogue.
                Generate a brief, atmospheric description of the new dungeon level the player
                has entered. Focus on sights, sounds, smells, and the general feel.
                Keep it to 2 sentences maximum.`,
                
            MONSTER_KILLED: `You are the Dungeon Master AI for a roguelike game called Brogue.
                Generate a brief, atmospheric description of the monster's death.
                Focus on the manner of its defeat and the aftermath.
                Keep it to 1-2 sentences maximum.`
        };
        
        console.log('Narrative generator initialized');
    }
    
    /**
     * Generate narrative response for a game event
     */
    async generateResponse(eventType, eventData, context = {}) {
        console.log(`Generating narrative for event: ${eventType}`);
        
        try {
            // First, try the direct Ollama approach
            // Build a complete prompt that includes context information
            const prompt = this.buildPromptForEvent(eventType, eventData, context);
            
            console.log(`Generating response for: ${prompt}`);
            
            // Include recent memories in context
            const recentMemories = this.memory.getRecentMemories(3);
            const memoryContext = recentMemories.length > 0 
                ? `Recent events: ${recentMemories.map(m => 
                    `${m.eventType}: ${JSON.stringify(m.eventData)}`).join('. ')}`
                : null;
            
            // Get base system prompt
            let systemPrompt = this.systemPrompts[eventType] || 
                'You are the Dungeon Master AI for a roguelike game called Brogue. Generate a brief, atmospheric response.';
            
            // Enhance system prompt with personality
            const personality = this.narratorSettings.getCurrentPersonality();
            systemPrompt = systemPrompt + "\n\n" + personality.getSystemPromptModifier();
            
            // Adjust verbosity based on personality
            const verbosity = personality.getAttribute('verbosity');
            if (verbosity > 7) {
                systemPrompt = systemPrompt.replace(/1-2 sentences/g, '2-3 sentences')
                                          .replace(/maximum 3 sentences/g, 'maximum 4 sentences')
                                          .replace(/Keep it to 2 sentences maximum/g, 'Use up to 3 sentences');
            } else if (verbosity < 4) {
                systemPrompt = systemPrompt.replace(/1-2 sentences/g, '1 sentence')
                                          .replace(/maximum 3 sentences/g, 'maximum 2 sentences')
                                          .replace(/Keep it to 2 sentences maximum/g, 'Use just 1 concise sentence');
            }
            
            console.log(`Using system prompt: ${systemPrompt.substring(0, 50)}...`);
            if (memoryContext) {
                console.log(`Using context: "${memoryContext.substring(0, 50)}..."`);
            }
            
            // Create payload for Ollama with personality-appropriate temperature
            const temperature = this.calculateTemperatureFromPersonality(personality);
            
            const payload = {
                model: this.ollama.model,
                prompt: personality.enhancePrompt(prompt, eventType),
                temperature: temperature,
                max_tokens: this.calculateMaxTokensFromPersonality(personality),
                system: systemPrompt,
                context: memoryContext
            };
            
            console.log(`Full payload: ${JSON.stringify(payload)}`);
            
            // Send request to Ollama
            console.log(`Sending request to Ollama...`);
            try {
                const response = await axios.post('http://localhost:11434/api/generate', payload);
                
                if (response.data && response.data.response) {
                    const narrativeResponse = this.postProcessResponse(response.data.response, personality);
                    console.log(`Generated narrative: ${narrativeResponse}`);
                    return narrativeResponse;
                } else {
                    throw new Error('No response field in Ollama response');
                }
            } catch (ollamaError) {
                console.error(`Ollama generation error: ${ollamaError.message}`);
                
                if (ollamaError.response) {
                    console.error(`Error status: ${ollamaError.response.status}`);
                    console.error(`Error data: ${JSON.stringify(ollamaError.response.data, null, 2)}`);
                }
                
                // Try fallback to templated messages
                const fallbackMessage = this.getFallbackResponseFromTemplate(eventType, eventData);
                console.log(`Using fallback narrative: ${fallbackMessage}`);
                return fallbackMessage;
            }
        } catch (error) {
            console.error(`Error generating narrative: ${error.message}`);
            return this.getFallbackResponseFromTemplate(eventType, eventData);
        }
    }
    
    /**
     * Post-process the generated response to ensure it aligns with personality
     */
    postProcessResponse(response, personality) {
        // If there's a chance to include signature phrases (1 in 10)
        if (personality.signaturePhrases.length > 0 && Math.random() < 0.1) {
            const randomPhrase = personality.signaturePhrases[
                Math.floor(Math.random() * personality.signaturePhrases.length)
            ];
            
            // Only append if the phrase isn't already in the response
            if (!response.includes(randomPhrase)) {
                return response + " " + randomPhrase;
            }
        }
        
        return response;
    }
    
    /**
     * Calculate appropriate temperature based on personality traits
     */
    calculateTemperatureFromPersonality(personality) {
        // Base temperature
        let temp = 0.7;
        
        // Increase temperature for more mercurial temperaments
        const temperament = personality.getAttribute('temperament');
        temp += (temperament - 5) * 0.04; // +/- 0.2 max adjustment
        
        // Higher metaphor complexity and cosmic awareness increase temperature
        const metaphorComplexity = personality.getAttribute('metaphorComplexity');
        const cosmicAwareness = personality.getAttribute('cosmicAwareness');
        temp += ((metaphorComplexity + cosmicAwareness) / 2 - 5) * 0.02; // +/- 0.1 max adjustment
        
        // Clamp between 0.5 and 1.0
        return Math.max(0.5, Math.min(1.0, temp));
    }
    
    /**
     * Calculate max tokens based on verbosity
     */
    calculateMaxTokensFromPersonality(personality) {
        const verbosity = personality.getAttribute('verbosity');
        // Base tokens: 100, adjust up or down based on verbosity
        return 80 + (verbosity * 10); // 90-180 token range
    }
    
    /**
     * Build appropriate prompt for the event type
     */
    buildPromptForEvent(eventType, eventData, context) {
        // Base prompt without personality adjustments
        let basePrompt = '';
        
        switch (eventType) {
            case 'MONSTER_ENCOUNTERED':
                basePrompt = `Player (level ${context.playerLevel || '?'}) encounters a ${eventData.monsterName} in a ${eventData.locationDesc || 'dark chamber'}. ${eventData.isFirstEncounter ? "This is their first time seeing this creature." : "They've encountered this type of creature before."}`;
                break;
                
            case 'ITEM_DISCOVERED':
                basePrompt = `Player discovers a ${eventData.itemName}. ${eventData.isRare ? "This is a rare item." : ""}`;
                break;
                
            case 'PLAYER_DIED':
                basePrompt = `Player (level ${context.playerLevel || '?'}) died after ${eventData.totalTurns || 'many'} turns, reaching dungeon depth ${eventData.maxDepth || '?'}. They were killed by ${eventData.killedBy}.`;
                break;
                
            case 'NEW_LEVEL':
                basePrompt = `Player has descended to dungeon depth ${eventData.depth}. This is a ${eventData.environmentType || 'dungeon'} environment.`;
                break;
                
            case 'MONSTER_KILLED':
                basePrompt = `Player has killed a ${eventData.monsterName}. ${eventData.isRare ? "This was a rare creature." : ""}`;
                break;
                
            default:
                basePrompt = `Player has experienced a ${eventType} event.`;
                break;
        }
        
        // Enhance the prompt based on personality
        const personality = this.narratorSettings.getCurrentPersonality();
        
        // Add cosmic awareness if high
        if (personality.getAttribute('cosmicAwareness') > 7) {
            if (eventType === 'MONSTER_ENCOUNTERED' && eventData.isRare) {
                basePrompt += " The creature has an aura of ancient power about it.";
            } else if (eventType === 'NEW_LEVEL') {
                basePrompt += " There is a sense of destiny or purpose to this place.";
            }
        }
        
        // Add nature references if high
        if (personality.getAttribute('natureReferences') > 7) {
            if (eventType === 'NEW_LEVEL') {
                const environments = ['cavern', 'cave', 'grotto', 'ravine', 'chasm', 'hollow'];
                const natureElements = ['moss', 'fungi', 'crystals', 'roots', 'stalagmites', 'underground streams'];
                
                basePrompt += ` The ${eventData.environmentType || 'dungeon'} contains natural ${
                    environments[Math.floor(Math.random() * environments.length)]
                } features and ${
                    natureElements[Math.floor(Math.random() * natureElements.length)]
                }.`;
            }
        }
        
        return basePrompt;
    }
    
    /**
     * Summarize event data for inclusion in prompts
     */
    summarizeEventData(eventData) {
        if (!eventData) return '';
        
        if (eventData.monsterName) {
            return `encountered a ${eventData.monsterName}${eventData.isRare ? " (rare)" : ""}`;
        } else if (eventData.itemName) {
            return `found a ${eventData.itemName}${eventData.isRare ? " (rare)" : ""}`;
        } else if (eventData.depth) {
            return `reached depth ${eventData.depth} (${eventData.environmentType || 'dungeon'})`;
        } else if (eventData.killedBy) {
            return `died to a ${eventData.killedBy} at depth ${eventData.maxDepth || '?'}`;
        }
        
        return JSON.stringify(eventData);
    }
    
    /**
     * Get fallback response from templates in fallbacks.json
     */
    getFallbackResponseFromTemplate(eventType, eventData) {
        // Check if we have templates for this event type
        const templates = this.fallbackMessages[eventType] || this.fallbackMessages['DEFAULT'];
        
        if (!templates || templates.length === 0) {
            return this.getFallbackResponse(eventType);
        }
        
        // Select a random template
        const template = templates[Math.floor(Math.random() * templates.length)];
        
        // Replace placeholders with actual data
        let message = template;
        
        if (eventData) {
            Object.keys(eventData).forEach(key => {
                message = message.replace(new RegExp(`\\{${key}\\}`, 'g'), eventData[key]);
            });
        }
        
        return message;
    }
    
    /**
     * Get hardcoded fallback response for an event type if no templates are available
     */
    getFallbackResponse(eventType) {
        switch (eventType) {
            case 'MONSTER_ENCOUNTERED':
                return 'You encounter a strange creature in the dungeon depths.';
                
            case 'ITEM_DISCOVERED':
                return 'You find an interesting item amidst the dungeon debris.';
                
            case 'PLAYER_DIED':
                return 'Your journey ends here, but the dungeon awaits your return...';
                
            case 'NEW_LEVEL':
                return 'You enter a new level of the dungeon, the air feels different here.';
                
            case 'MONSTER_KILLED':
                return 'The creature falls before you, its essence returning to the dungeon depths.';
                
            default:
                return 'The dungeon seems to shift around you.';
        }
    }
    
    /**
     * Get the narrator settings controller
     */
    getNarratorSettings() {
        return this.narratorSettings;
    }
}

module.exports = { NarrativeGenerator }; 