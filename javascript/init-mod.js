const urlParams = new URLSearchParams(window.location.search);
let seed = urlParams.get('seed') || urlParams.get('s');
if (seed !== null && seed.includes('tuesday')) seed = getMostRecentTuesday(); // r/brogue Tuesday contest seed generator

const variant = urlParams.get('variant'); // rapid_brogue, bullet_brogue
const stealth = urlParams.get('stealth');
const noEffects = urlParams.get('no-effects');
const wizard = urlParams.get('wizard');

Module.arguments = [];

if (seed != null && seed != "") { Module.arguments.push("-s"); Module.arguments.push(seed); }
if (variant != null && variant != "") { Module.arguments.push("--variant"); Module.arguments.push(variant); }

if (stealth != null) Module.arguments.push("--stealth");
if (noEffects != null) Module.arguments.push("--no-effects");
if (wizard != null) Module.arguments.push("--wizard");
