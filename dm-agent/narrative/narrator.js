/**
 * Narrator Personality System
 * 
 * Defines personality traits and presets for the DM narrative generator.
 * This enables dynamic adjustment of the narrative style during gameplay.
 */

class NarratorPersonality {
    constructor(preset = 'default') {
        // Default personality attributes
        this.attributes = {
            // Core attributes
            voiceTone: 5, // Scale 1-10: Formal (1) to Casual (10)
            wisdomLevel: 5, // Scale 1-10: Naive (1) to Ancient (10)
            verbosity: 5, // Scale 1-10: Terse (1) to Elaborate (10)
            temperament: 5, // Scale 1-10: Calm (1) to Mercurial (10)
            perspective: 5, // Scale 1-10: Optimistic (1) to Pessimistic (10)
            
            // Thematic tendencies
            natureReferences: 5, // Scale 1-10: Low (1) to High (10)
            metaphorComplexity: 5, // Scale 1-10: Simple (1) to Complex (10)
            cosmicAwareness: 5, // Scale 1-10: Mundane (1) to Omniscient (10)
            humorStyle: 5, // Scale 1-10: None (1) to Whimsical (10)
            characterInsight: 5, // Scale 1-10: Oblivious (1) to All-knowing (10)
            
            // Speech patterns
            questionFrequency: 5, // Scale 1-10: Low (1) to High (10)
            syntaxComplexity: 5, // Scale 1-10: Simple (1) to Complex (10)
            archaismLevel: 5, // Scale 1-10: Modern (1) to Archaic (10)
            idiomUsage: 5, // Scale 1-10: Literal (1) to Figurative (10)
            dramaticPauses: 5, // Scale 1-10: Flowing (1) to Measured (10)
        };
        
        // Signature phrases that might be injected into responses
        this.signaturePhrases = [];
        
        // Apply a preset if specified
        if (preset !== 'default') {
            this.applyPreset(preset);
        }
    }
    
    /**
     * Apply a predefined personality preset
     */
    applyPreset(presetName) {
        const preset = NARRATOR_PRESETS[presetName];
        if (!preset) {
            console.warn(`Unknown narrator preset: ${presetName}. Using default settings.`);
            return;
        }
        
        // Apply preset attributes
        if (preset.attributes) {
            this.attributes = { ...this.attributes, ...preset.attributes };
        }
        
        // Apply preset signature phrases
        if (preset.signaturePhrases) {
            this.signaturePhrases = [...preset.signaturePhrases];
        }
        
        console.log(`Applied narrator preset: ${presetName}`);
    }
    
    /**
     * Set a specific attribute value
     */
    setAttribute(attributeName, value) {
        if (this.attributes.hasOwnProperty(attributeName)) {
            this.attributes[attributeName] = Math.max(1, Math.min(10, value)); // Clamp between 1-10
            return true;
        }
        return false;
    }
    
    /**
     * Get the current value of an attribute
     */
    getAttribute(attributeName) {
        return this.attributes[attributeName] || 5; // Default to middle value if not found
    }
    
    /**
     * Save current configuration to a preset name
     */
    saveAsPreset(presetName) {
        NARRATOR_PRESETS[presetName] = {
            attributes: { ...this.attributes },
            signaturePhrases: [...this.signaturePhrases]
        };
    }
    
    /**
     * Get a system prompt modifier based on current personality
     */
    getSystemPromptModifier() {
        let modifier = "You are narrating as a Dungeon Master with the following personality traits:\n";
        
        // Add core attributes
        modifier += `- Voice: ${this.attributes.voiceTone > 7 ? 'casual and conversational' : 
                           (this.attributes.voiceTone < 4 ? 'formal and dignified' : 'balanced')}\n`;
        
        modifier += `- Wisdom: ${this.attributes.wisdomLevel > 7 ? 'ancient and deeply knowledgeable' : 
                           (this.attributes.wisdomLevel < 4 ? 'fresh and learning' : 'experienced')}\n`;
        
        modifier += `- Style: ${this.attributes.verbosity > 7 ? 'elaborate and detailed' : 
                           (this.attributes.verbosity < 4 ? 'concise and direct' : 'measured')}\n`;
        
        // Add thematic preferences
        if (this.attributes.natureReferences > 7) {
            modifier += "- Often reference nature, stars, trees, and natural elements\n";
        }
        
        if (this.attributes.metaphorComplexity > 7) {
            modifier += "- Use complex metaphors and allegories\n";
        }
        
        if (this.attributes.cosmicAwareness > 7) {
            modifier += "- Show awareness of greater cosmic forces and destinies\n";
        }
        
        // Add speech pattern guidance
        if (this.attributes.archaismLevel > 7) {
            modifier += "- Use archaic speech patterns and older English forms\n";
        }
        
        if (this.attributes.dramaticPauses > 7) {
            modifier += "- Include dramatic pauses, indicated by ellipses or brief reflections\n";
        }
        
        if (this.attributes.questionFrequency > 7) {
            modifier += "- Occasionally pose philosophical questions to the player\n";
        }
        
        // Add signature phrases guidance if any exist
        if (this.signaturePhrases.length > 0) {
            modifier += "- Consider using these signature phrases when appropriate: \"" + 
                this.signaturePhrases.join('", "') + "\"\n";
        }
        
        return modifier;
    }
    
    /**
     * Enhance a prompt with personality-specific guidance
     */
    enhancePrompt(basePrompt, eventType) {
        // Add specific guidance based on event type and personality
        let enhancedPrompt = basePrompt;
        
        // Add event-specific guidance
        switch (eventType) {
            case 'MONSTER_ENCOUNTERED':
                if (this.attributes.characterInsight > 7) {
                    enhancedPrompt += " Provide insight into the monster's nature or motivation.";
                }
                break;
                
            case 'ITEM_DISCOVERED':
                if (this.attributes.cosmicAwareness > 7) {
                    enhancedPrompt += " Hint at the item's history or place in the greater scheme.";
                }
                break;
                
            case 'PLAYER_DIED':
                if (this.attributes.wisdomLevel > 7) {
                    enhancedPrompt += " Offer a philosophical perspective on death and the journey.";
                }
                break;
        }
        
        // Add general personality-based guidance
        if (this.attributes.humorStyle > 7) {
            enhancedPrompt += " Include a subtle touch of humor if appropriate.";
        }
        
        return enhancedPrompt;
    }
}

// Predefined narrator personality presets
const NARRATOR_PRESETS = {
    'gandalf': {
        attributes: {
            // Core attributes
            voiceTone: 7,      // Somewhat formal with occasional warmth
            wisdomLevel: 9,    // Ancient, spanning ages
            verbosity: 6,      // Measured, expands when important
            temperament: 5,    // Usually calm, occasionally passionate
            perspective: 6,    // Balanced, slightly optimistic about ultimate outcomes
            
            // Thematic tendencies
            natureReferences: 7,    // Frequent references to stars, trees, mountains
            metaphorComplexity: 8,  // Complex, often light/darkness themed
            cosmicAwareness: 9,     // Very aware of larger forces
            humorStyle: 4,          // Dry, subtle humor in grave situations
            characterInsight: 8,    // Deeply perceptive of character motivations
            
            // Speech patterns
            questionFrequency: 3,   // More statements than questions, but questions are profound
            syntaxComplexity: 7,    // Somewhat complex sentence structures
            archaismLevel: 6,       // Moderately archaic phrasing
            idiomUsage: 7,          // Rich with metaphor and allusion
            dramaticPauses: 7,      // Strategic pauses for emphasis
        },
        signaturePhrases: [
            "All we have to decide is what to do with the time that is given us.",
            "Many that live deserve death. And some that die deserve life.",
            "There are darker things in the deep places of the world.",
            "This foe is beyond any of you.",
            "Not all those who wander are lost."
        ]
    },
    
    'galadriel': {
        attributes: {
            voiceTone: 8,
            wisdomLevel: 10,
            verbosity: 7,
            temperament: 3,
            perspective: 5,
            natureReferences: 9,
            metaphorComplexity: 9,
            cosmicAwareness: 10,
            humorStyle: 2,
            characterInsight: 10,
            questionFrequency: 5,
            syntaxComplexity: 8,
            archaismLevel: 7,
            idiomUsage: 8,
            dramaticPauses: 8
        },
        signaturePhrases: [
            "Even the smallest person can change the course of the future.",
            "The world is changed. I feel it in the water. I feel it in the earth.",
            "Things that were... things that are... and some things that have not yet come to pass."
        ]
    },
    
    'aragorn': {
        attributes: {
            voiceTone: 5,
            wisdomLevel: 7,
            verbosity: 4,
            temperament: 6,
            perspective: 7,
            natureReferences: 6,
            metaphorComplexity: 5,
            cosmicAwareness: 6,
            humorStyle: 3,
            characterInsight: 7,
            questionFrequency: 2,
            syntaxComplexity: 5,
            archaismLevel: 5,
            idiomUsage: 6,
            dramaticPauses: 4
        },
        signaturePhrases: [
            "There is always hope.",
            "The day has come at last.",
            "I would have gone with you to the end."
        ]
    }
};

module.exports = { NarratorPersonality, NARRATOR_PRESETS }; 