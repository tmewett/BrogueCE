/**
 * Narrator Settings Controller
 * 
 * Manages the UI for narrator personality customization and handles
 * configuration persistence.
 */

const fs = require('fs');
const path = require('path');
const { NarratorPersonality, NARRATOR_PRESETS } = require('./narrator');

class NarratorSettings {
    constructor() {
        this.configPath = path.join(__dirname, '../config');
        this.settingsFile = path.join(this.configPath, 'narrator_settings.json');
        this.currentPersonality = new NarratorPersonality('gandalf'); // Default to Gandalf
        this.customPresets = {};
        
        // Ensure config directory exists
        if (!fs.existsSync(this.configPath)) {
            fs.mkdirSync(this.configPath, { recursive: true });
        }
        
        // Load existing settings
        this.loadSettings();
    }
    
    /**
     * Load narrator settings from disk
     */
    loadSettings() {
        try {
            if (fs.existsSync(this.settingsFile)) {
                const data = JSON.parse(fs.readFileSync(this.settingsFile, 'utf8'));
                
                // Load custom presets
                if (data.customPresets) {
                    this.customPresets = data.customPresets;
                }
                
                // Load current personality
                if (data.currentPreset) {
                    if (NARRATOR_PRESETS[data.currentPreset]) {
                        this.currentPersonality.applyPreset(data.currentPreset);
                    } else if (this.customPresets[data.currentPreset]) {
                        this.currentPersonality.attributes = { ...this.currentPersonality.attributes, ...this.customPresets[data.currentPreset].attributes };
                        this.currentPersonality.signaturePhrases = [...this.customPresets[data.currentPreset].signaturePhrases];
                    }
                }
                
                console.log('Loaded narrator settings');
            } else {
                this.saveSettings(); // Create default settings file
            }
        } catch (error) {
            console.error('Error loading narrator settings:', error.message);
        }
    }
    
    /**
     * Save current settings to disk
     */
    saveSettings() {
        try {
            const data = {
                currentPreset: this.getCurrentPresetName(),
                customPresets: this.customPresets
            };
            
            fs.writeFileSync(this.settingsFile, JSON.stringify(data, null, 2), 'utf8');
            console.log('Saved narrator settings');
        } catch (error) {
            console.error('Error saving narrator settings:', error.message);
        }
    }
    
    /**
     * Get the current active personality
     */
    getCurrentPersonality() {
        return this.currentPersonality;
    }
    
    /**
     * Determine which preset (if any) matches the current personality
     */
    getCurrentPresetName() {
        // Check built-in presets
        for (const [name, preset] of Object.entries(NARRATOR_PRESETS)) {
            if (this.isMatchingPreset(preset)) {
                return name;
            }
        }
        
        // Check custom presets
        for (const [name, preset] of Object.entries(this.customPresets)) {
            if (this.isMatchingPreset(preset)) {
                return name;
            }
        }
        
        return 'custom'; // Not matching any preset
    }
    
    /**
     * Check if current personality matches a preset
     */
    isMatchingPreset(preset) {
        // Check all attributes
        for (const [key, value] of Object.entries(preset.attributes)) {
            if (this.currentPersonality.attributes[key] !== value) {
                return false;
            }
        }
        
        // Check signature phrases (must have same phrases in same order)
        if (preset.signaturePhrases.length !== this.currentPersonality.signaturePhrases.length) {
            return false;
        }
        
        for (let i = 0; i < preset.signaturePhrases.length; i++) {
            if (preset.signaturePhrases[i] !== this.currentPersonality.signaturePhrases[i]) {
                return false;
            }
        }
        
        return true;
    }
    
    /**
     * Apply a preset to the current personality
     */
    applyPreset(presetName) {
        // Check built-in presets first
        if (NARRATOR_PRESETS[presetName]) {
            this.currentPersonality.applyPreset(presetName);
            this.saveSettings();
            return true;
        }
        
        // Then check custom presets
        if (this.customPresets[presetName]) {
            this.currentPersonality.attributes = { 
                ...this.currentPersonality.attributes, 
                ...this.customPresets[presetName].attributes 
            };
            this.currentPersonality.signaturePhrases = [...this.customPresets[presetName].signaturePhrases];
            this.saveSettings();
            return true;
        }
        
        return false;
    }
    
    /**
     * Save current personality as a custom preset
     */
    saveCustomPreset(presetName) {
        this.customPresets[presetName] = {
            attributes: { ...this.currentPersonality.attributes },
            signaturePhrases: [...this.currentPersonality.signaturePhrases]
        };
        this.saveSettings();
    }
    
    /**
     * Delete a custom preset
     */
    deleteCustomPreset(presetName) {
        if (this.customPresets[presetName]) {
            delete this.customPresets[presetName];
            this.saveSettings();
            return true;
        }
        return false;
    }
    
    /**
     * Set a specific personality attribute
     */
    setAttribute(attributeName, value) {
        const result = this.currentPersonality.setAttribute(attributeName, value);
        if (result) {
            this.saveSettings();
        }
        return result;
    }
    
    /**
     * Add a signature phrase
     */
    addSignaturePhrase(phrase) {
        if (phrase && !this.currentPersonality.signaturePhrases.includes(phrase)) {
            this.currentPersonality.signaturePhrases.push(phrase);
            this.saveSettings();
            return true;
        }
        return false;
    }
    
    /**
     * Remove a signature phrase
     */
    removeSignaturePhrase(index) {
        if (index >= 0 && index < this.currentPersonality.signaturePhrases.length) {
            this.currentPersonality.signaturePhrases.splice(index, 1);
            this.saveSettings();
            return true;
        }
        return false;
    }
    
    /**
     * Get all available presets (built-in and custom)
     */
    getAllPresets() {
        return {
            ...NARRATOR_PRESETS,
            ...this.customPresets
        };
    }
    
    /**
     * Generate HTML for the narrator settings UI
     */
    generateSettingsHTML() {
        const currentPreset = this.getCurrentPresetName();
        const allPresets = this.getAllPresets();
        
        let html = `
        <div class="narrator-settings-panel">
            <h2>The Mirror of Galadriel</h2>
            <div class="preset-selector">
                <label>Narrator Persona:</label>
                <select id="narrator-preset">`;
        
        // Add preset options
        for (const [name, _] of Object.entries(NARRATOR_PRESETS)) {
            html += `<option value="${name}" ${name === currentPreset ? 'selected' : ''}>${name.charAt(0).toUpperCase() + name.slice(1)}</option>`;
        }
        
        // Add custom presets
        if (Object.keys(this.customPresets).length > 0) {
            html += `<optgroup label="Custom Presets">`;
            for (const [name, _] of Object.entries(this.customPresets)) {
                html += `<option value="${name}" ${name === currentPreset ? 'selected' : ''}>${name}</option>`;
            }
            html += `</optgroup>`;
        }
        
        html += `
                </select>
                <button id="save-preset">Save</button>
                <button id="load-preset">Load</button>
                <button id="default-preset">Default</button>
            </div>
            
            <h3>Core Attributes</h3>
            <div class="attributes-section">`;
        
        // Core attributes sliders
        const coreAttributes = [
            { name: 'voiceTone', label: 'Voice Tone', min: 'Formal', max: 'Casual' },
            { name: 'wisdomLevel', label: 'Wisdom Level', min: 'Naive', max: 'Ancient' },
            { name: 'verbosity', label: 'Verbosity', min: 'Terse', max: 'Elaborate' },
            { name: 'temperament', label: 'Temperament', min: 'Calm', max: 'Mercurial' },
            { name: 'perspective', label: 'Perspective', min: 'Optimistic', max: 'Pessimistic' }
        ];
        
        for (const attr of coreAttributes) {
            const value = this.currentPersonality.getAttribute(attr.name);
            html += this.generateSliderHTML(attr.name, attr.label, attr.min, attr.max, value);
        }
        
        html += `
            </div>
            
            <h3>Thematic Tendencies</h3>
            <div class="attributes-section">`;
        
        // Thematic tendencies sliders
        const thematicAttributes = [
            { name: 'natureReferences', label: 'Nature References', min: 'Low', max: 'High' },
            { name: 'metaphorComplexity', label: 'Metaphor Complexity', min: 'Low', max: 'High' },
            { name: 'cosmicAwareness', label: 'Cosmic Awareness', min: 'Low', max: 'High' },
            { name: 'humorStyle', label: 'Humor Style', min: 'Dry', max: 'Whimsical' },
            { name: 'characterInsight', label: 'Character Insight', min: 'Low', max: 'High' }
        ];
        
        for (const attr of thematicAttributes) {
            const value = this.currentPersonality.getAttribute(attr.name);
            html += this.generateSliderHTML(attr.name, attr.label, attr.min, attr.max, value);
        }
        
        html += `
            </div>
            
            <h3>Speech Patterns</h3>
            <div class="attributes-section">`;
        
        // Speech patterns sliders
        const speechAttributes = [
            { name: 'questionFrequency', label: 'Question Frequency', min: 'Low', max: 'High' },
            { name: 'syntaxComplexity', label: 'Syntax Complexity', min: 'Low', max: 'High' },
            { name: 'archaismLevel', label: 'Archaism Level', min: 'Low', max: 'High' },
            { name: 'idiomUsage', label: 'Idiom Usage', min: 'Low', max: 'High' },
            { name: 'dramaticPauses', label: 'Dramatic Pauses', min: 'Low', max: 'High' }
        ];
        
        for (const attr of speechAttributes) {
            const value = this.currentPersonality.getAttribute(attr.name);
            html += this.generateSliderHTML(attr.name, attr.label, attr.min, attr.max, value);
        }
        
        // Signature phrases
        html += `
            </div>
            
            <h3>Signature Phrases</h3>
            <div class="signature-phrases">
                <ul id="phrases-list">`;
        
        for (const phrase of this.currentPersonality.signaturePhrases) {
            html += `<li>${phrase} <button class="remove-phrase">✕</button></li>`;
        }
        
        html += `
                </ul>
                <div class="add-phrase">
                    <input type="text" id="new-phrase" placeholder="Add a new signature phrase...">
                    <button id="add-phrase-btn">Add</button>
                </div>
            </div>
            
            <div class="preview-section">
                <h3>Preview</h3>
                <div id="preview-text">
                    <p class="preview-content">"${this.generatePreviewText()}"</p>
                </div>
            </div>
            
            <div class="button-row">
                <button id="apply-changes">Apply Changes</button>
                <button id="cancel-changes">Cancel</button>
            </div>
        </div>`;
        
        return html;
    }
    
    /**
     * Generate HTML for a single slider control
     */
    generateSliderHTML(name, label, min, max, value) {
        return `
            <div class="slider-container">
                <div class="slider-label">${label}:</div>
                <div class="slider-min">${min}</div>
                <input type="range" class="personality-slider" id="${name}-slider" 
                       min="1" max="10" value="${value}" 
                       data-attribute="${name}">
                <div class="slider-max">${max}</div>
            </div>
        `;
    }
    
    /**
     * Generate a preview text based on current personality
     */
    generatePreviewText() {
        // Create sample preview texts for different personality combinations
        const wisdomLevel = this.currentPersonality.getAttribute('wisdomLevel');
        const verbosity = this.currentPersonality.getAttribute('verbosity');
        const archaismLevel = this.currentPersonality.getAttribute('archaismLevel');
        const cosmicAwareness = this.currentPersonality.getAttribute('cosmicAwareness');
        
        // Highly verbose and wise
        if (wisdomLevel > 7 && verbosity > 7) {
            return "The passage ahead darkens, not merely in absence of light, but with something more... a shadow that does not belong to this world. You would be wise to tread carefully, for not all darkness yields to the simple flame of a torch.";
        }
        
        // Archaic and cosmic
        if (archaismLevel > 7 && cosmicAwareness > 7) {
            return "Behold, the ancient chamber unfolds before thee, echoing with whispers of powers long forgotten. The very stars once gazed upon these stones, and their memory lingers still.";
        }
        
        // Direct and simple
        if (wisdomLevel < 4 && verbosity < 4) {
            return "A dark corridor stretches ahead. Watch for traps and listen for monsters. The air feels dangerous here.";
        }
        
        // Default balanced preview
        return "The shadows deepen as you venture further into the cavern. A faint glimmer catches your eye - perhaps a treasure, or perhaps a trap. Choose your next steps with care.";
    }
}

module.exports = { NarratorSettings }; 