const axios = require('axios');

console.log('Starting direct Ollama test...');

async function testOllama() {
    const payload = {
        model: 'llama3',
        prompt: 'Tell me a short joke about dungeons',
        stream: false
    };
    
    try {
        console.log('Sending request to Ollama...');
        const response = await axios.post('http://localhost:11434/api/generate', payload);
        console.log('Response received:');
        console.log(JSON.stringify(response.data, null, 2));
    } catch (error) {
        console.error('Error:', error.message);
        if (error.response) {
            console.error('Response status:', error.response.status);
            console.error('Response data:', JSON.stringify(error.response.data, null, 2));
        } else if (error.request) {
            console.error('No response received');
        }
    }
}

testOllama(); 