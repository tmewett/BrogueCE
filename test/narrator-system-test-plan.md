# Narrator Personality System Test Plan

This document outlines the testing strategy for the BrogueMCP narrator personality system.

## Unit Tests

### Core Personality System (narrator.js)

| Test ID | Description | Expected Result |
|---------|-------------|-----------------|
| UT-NP-001 | Create personality with default settings | All attributes set to middle values (5) |
| UT-NP-002 | Apply Gandalf preset | Attributes match Gandalf preset values |
| UT-NP-003 | Set individual attributes | Attribute changes and is clamped between 1-10 |
| UT-NP-004 | Get system prompt modifier | Returns appropriate text based on attributes |
| UT-NP-005 | Enhance prompt | Adds contextual enhancements based on personality |
| UT-NP-006 | Save custom preset | New preset is created with correct values |

### Settings Management (settings.js)

| Test ID | Description | Expected Result |
|---------|-------------|-----------------|
| UT-NS-001 | Load settings from disk | Correctly loads saved settings |
| UT-NS-002 | Save settings to disk | Settings file created with correct data |
| UT-NS-003 | Apply built-in preset | Personality updated with preset values |
| UT-NS-004 | Save and load custom preset | Custom preset persists between sessions |
| UT-NS-005 | Delete custom preset | Preset is removed from storage |
| UT-NS-006 | Generate settings HTML | Valid HTML is generated |
| UT-NS-007 | Generate preview text | Preview changes based on personality |

### Integration Tests (generator.js)

| Test ID | Description | Expected Result |
|---------|-------------|-----------------|
| IT-NG-001 | Generate response with different verbosity | Response length varies with verbosity setting |
| IT-NG-002 | Post-process with signature phrases | Occasionally adds signature phrases |
| IT-NG-003 | Calculate temperature from personality | Temperature changes based on temperament |
| IT-NG-004 | Max tokens from verbosity | Token limit scales with verbosity |
| IT-NG-005 | Enhance prompts with cosmic awareness | Adds cosmic references to prompts |
| IT-NG-006 | Enhance prompts with nature references | Adds nature references to prompts |

## API Tests

| Test ID | Description | Expected Result |
|---------|-------------|-----------------|
| API-001 | GET /api/narrator/settings | Returns current settings |
| API-002 | POST /api/narrator/settings (applyPreset) | Preset is applied |
| API-003 | POST /api/narrator/settings (setAttribute) | Attribute is updated |
| API-004 | POST /api/narrator/settings (addPhrase) | Phrase is added |
| API-005 | POST /api/narrator/settings (removePhrase) | Phrase is removed |
| API-006 | GET /api/narrator/ui | Returns HTML |
| API-007 | GET /narrator | Returns HTML page |

## UI Tests

| Test ID | Description | Expected Result |
|---------|-------------|-----------------|
| UI-001 | Load settings page | UI displays with current settings |
| UI-002 | Change preset dropdown | UI updates with selected preset |
| UI-003 | Move sliders | Values update in real-time |
| UI-004 | Add signature phrase | Phrase appears in list |
| UI-005 | Remove signature phrase | Phrase is removed from list |
| UI-006 | Apply changes | Settings are saved and reflected in game |
| UI-007 | Cancel changes | Changes are discarded |
| UI-008 | Responsive design | UI adapts to different screen sizes |

## Game Integration Tests

| Test ID | Description | Expected Result |
|---------|-------------|-----------------|
| GI-001 | Press 'N' key in game | Browser opens with settings UI |
| GI-002 | Change narrator settings | Game narration style changes |
| GI-003 | Different narration styles | Content varies based on personality |
| GI-004 | Multiple preset changes | Game adapts to each preset style |

## Performance Tests

| Test ID | Description | Expected Result |
|---------|-------------|-----------------|
| PT-001 | Load time for settings UI | Under 2 seconds |
| PT-002 | Response generation time | Under 1 second |
| PT-003 | Memory usage | Below 50MB |
| PT-004 | CPU usage during generation | Below 20% |

## Security Tests

| Test ID | Description | Expected Result |
|---------|-------------|-----------------|
| ST-001 | XSS vulnerability in UI | No injection possible |
| ST-002 | File system access | Limited to configuration directory |
| ST-003 | API input validation | Rejects malformed inputs |

## Test Automation Strategy

1. **Unit Tests**: Implement using Jest for JavaScript components
2. **API Tests**: Use Postman or Supertest for API endpoint testing
3. **UI Tests**: Implement using Playwright or Cypress
4. **Game Integration**: Manual testing with automated test harness
5. **Performance**: Use Artillery for load testing

## Test Data

- Multiple preset configurations
- Various personality attribute combinations
- Edge cases (all attributes at 1, all at 10)
- Valid and invalid signature phrases
- Special characters and Unicode in settings

## Test Environment

- Windows, macOS, and Linux testing
- Multiple browsers for UI testing
- Both development and production builds

## Reporting

Test results will be stored in the `BrogueMCP/test/reports` directory and will include:
- Pass/fail status for each test
- Performance metrics
- Screenshots of UI tests
- Generated narrative samples 