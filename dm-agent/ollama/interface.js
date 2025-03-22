/**
 * Ollama interface for Brogue Dungeon Master AI
 * 
 * Provides a client for interacting with Ollama API
 */

const axios = require('axios');

class OllamaInterface {
    constructor(config = {}) {
        this.model = config.model || 'llama3';
        this.baseUrl = config.baseUrl || 'http://localhost:11434';
        this.apiEndpoint = `${this.baseUrl}/api/generate`;
        this.timeout = config.timeout || 10000; // Increased timeout to 10 seconds
        this.temperature = config.temperature || 0.7;
        this.maxTokens = config.maxTokens || 512;
        this.fallbackResponses = {
            'MONSTER_ENCOUNTERED': 'You encounter a strange creature in the dungeon depths.',
            'ITEM_DISCOVERED': 'You find an interesting item amidst the dungeon debris.',
            'PLAYER_DIED': 'Your journey ends here, but the dungeon awaits your return...',
            'NEW_LEVEL': 'You enter a new level of the dungeon, the air feels different here.',
            'MONSTER_KILLED': 'The creature falls before you, its essence returning to the dungeon depths.'
        };
        
        console.log(`Ollama interface initialized with model: ${this.model}`);
        console.log(`API endpoint: ${this.apiEndpoint}`);
    }

    /**
     * Generate text using local Ollama model
     */
    async generate(prompt, systemPrompt = null, context = null, options = {}) {
        console.log(`Generating response for: ${prompt}`);
        
        const payload = {
            model: options.model || this.model,
            prompt: prompt,
            temperature: options.temperature || this.temperature,
            max_tokens: options.maxTokens || this.maxTokens
        };
        
        if (systemPrompt) {
            payload.system = systemPrompt;
            console.log(`Using system prompt: ${systemPrompt.substring(0, 50)}...`);
        }
        
        if (context) {
            payload.context = context;
            console.log(`Using context: ${JSON.stringify(context).substring(0, 50)}...`);
        }
        
        console.log('Full payload:', JSON.stringify(payload));
        
        try {
            console.log('Sending request to Ollama...');
            const response = await axios.post(this.apiEndpoint, payload, {
                timeout: options.timeout || this.timeout
            });
            console.log('Response received from Ollama');
            
            if (!response.data.response) {
                console.error('Ollama response missing "response" field:', response.data);
                return this.getFallbackResponse(prompt);
            }
            
            return response.data.response;
        } catch (error) {
            console.error('Ollama generation error:', error.message);
            
            // More detailed error logging
            if (error.response) {
                console.error('Error status:', error.response.status);
                console.error('Error data:', error.response.data);
            } else if (error.request) {
                console.error('No response received:', error.request);
            }
            
            // Return fallback response
            return this.getFallbackResponse(prompt);
        }
    }
    
    /**
     * Get a fallback response when Ollama fails
     */
    getFallbackResponse(prompt) {
        // Extract event type from prompt
        for (const [eventType, fallback] of Object.entries(this.fallbackResponses)) {
            if (prompt.includes(eventType)) {
                console.log(`Using fallback response for ${eventType}`);
                return fallback;
            }
        }
        
        // Default fallback
        return 'The dungeon seems to shift around you.';
    }
}

module.exports = { OllamaInterface }; 