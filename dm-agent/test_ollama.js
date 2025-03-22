/**
 * Simple test script for Ollama connection
 */

const axios = require('axios');

async function testOllama() {
    console.log('Testing Ollama connection...');
    
    const payload = {
        model: 'llama3',
        prompt: 'Tell me a short story about a dungeon explorer.',
        temperature: 0.7,
        max_tokens: 100
    };
    
    try {
        console.log('Sending request to Ollama...');
        const response = await axios.post('http://localhost:11434/api/generate', payload, {
            timeout: 10000 // 10 seconds timeout
        });
        
        console.log('Response received from Ollama:');
        console.log(response.data.response);
        return true;
    } catch (error) {
        console.error('Ollama request failed:');
        console.error(error.message);
        if (error.response) {
            console.error('Response status:', error.response.status);
            console.error('Response data:', error.response.data);
        }
        return false;
    }
}

testOllama().then(success => {
    if (success) {
        console.log('Ollama test successful!');
    } else {
        console.log('Ollama test failed!');
    }
}); 