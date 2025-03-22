# Narrator Personality System

The Narrator Personality System introduces dynamic storytelling capabilities to the BrogueMCP Dungeon Master AI. This system allows players to customize how the game narrates events, tailoring the storytelling style to their preferences.

## Features

- **Adjustable Narrator Personality**: Fine-tune numerous aspects of the narrator's voice, from formal to casual, terse to verbose, and more.
- **Thematic Tendencies**: Control the narrator's propensity to reference nature, use complex metaphors, or demonstrate cosmic awareness.
- **Speech Patterns**: Adjust question frequency, syntax complexity, archaism level, and more.
- **Preset Personalities**: Choose from built-in presets inspired by characters from Lord of the Rings, or create your own.
- **Signature Phrases**: Add memorable quotes that may occasionally appear in narration.
- **Live Preview**: See examples of how different settings affect the narration style.

## How to Use

### In-Game Access

1. While playing BrogueMCP, press the `N` key to open the Narrator Settings UI in your default web browser.
2. Adjust settings as desired and click "Apply Changes" to update the narrator's personality.
3. Continue playing to experience the new narration style.

### Personality Attributes

The narrator personality is divided into three main categories:

#### Core Attributes
- **Voice Tone**: Ranges from formal and dignified to casual and conversational.
- **Wisdom Level**: Ranges from naive and fresh to ancient and deeply knowledgeable.
- **Verbosity**: Controls how concise or elaborate the narration is.
- **Temperament**: Adjusts the emotional range from calm to mercurial.
- **Perspective**: Sets the outlook from optimistic to pessimistic.

#### Thematic Tendencies
- **Nature References**: Controls frequency of references to natural elements.
- **Metaphor Complexity**: Adjusts the complexity of metaphors and allegories.
- **Cosmic Awareness**: Sets the level of awareness of greater forces and destinies.
- **Humor Style**: Controls the type and frequency of humor.
- **Character Insight**: Adjusts how perceptive the narrator is about characters and monsters.

#### Speech Patterns
- **Question Frequency**: Controls how often the narrator poses questions.
- **Syntax Complexity**: Adjusts the complexity of sentence structures.
- **Archaism Level**: Sets the level of archaic language and older speech forms.
- **Idiom Usage**: Controls the use of idioms and figurative language.
- **Dramatic Pauses**: Adjusts the use of pauses and measured speech.

### Built-in Presets

- **Gandalf**: Formal with occasional warmth, ancient wisdom, and complex light/darkness metaphors.
- **Galadriel**: Highly cosmic awareness, nature-focused, and with ethereal, prophetic speech patterns.
- **Aragorn**: Balanced tone, practical wisdom, direct communication with occasional poetic flourishes.

## Technical Details

### Architecture

The Narrator Personality System consists of:

1. **NarratorPersonality Class**: Core entity that manages personality attributes and enhances prompts.
2. **NarratorSettings Class**: Handles persistence, UI generation, and preset management.
3. **API Endpoints**: Server routes for accessing and modifying settings.
4. **Web Interface**: User-friendly UI for adjusting settings.
5. **Integration with NarrativeGenerator**: Enhances AI prompts based on personality settings.

### Server Routes

- **GET /narrator**: Opens the narrator settings UI
- **GET /api/narrator/settings**: Retrieves current narrator settings
- **POST /api/narrator/settings**: Updates narrator settings
- **GET /api/narrator/ui**: Gets the HTML for the settings UI

### Files

- **narrator.js**: Core personality system
- **settings.js**: Settings management
- **narrator.html**: Web UI
- **narrator.css**: Styling for the UI
- **generator.js** (modified): Integrates personality into narrative generation

## Extending the System

### Adding New Presets

To add new built-in presets, edit the `NARRATOR_PRESETS` object in `narrator.js`.

### Customizing the UI

The UI is styled with CSS in `narrator.css`. You can modify this to change the appearance.

### Adding New Personality Dimensions

To add new personality dimensions:

1. Add the new attribute to the `attributes` object in the `NarratorPersonality` constructor
2. Update the `getSystemPromptModifier` method to include the new attribute
3. Modify the `enhancePrompt` method to use the new attribute
4. Update the UI generation in `generateSettingsHTML`

## Troubleshooting

- **UI not opening**: Ensure the DM agent server is running at http://localhost:3001
- **Settings not saving**: Check console for errors; ensure the config directory is writable
- **Changes not reflected in narration**: Remember that personality effects are probabilistic and contextual

## Example Usage

```javascript
// Getting the current personality
const personality = narratorGenerator.getNarratorSettings().getCurrentPersonality();

// Applying a preset
narratorGenerator.getNarratorSettings().applyPreset('gandalf');

// Setting a single attribute
narratorGenerator.getNarratorSettings().setAttribute('verbosity', 8);

// Adding a signature phrase
narratorGenerator.getNarratorSettings().addSignaturePhrase('All we have to decide is what to do with the time that is given us.');
``` 