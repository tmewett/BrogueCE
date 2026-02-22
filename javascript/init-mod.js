
const urlParams = new URLSearchParams(window.location.search);
let seed = urlParams.get('seed') || urlParams.get('s');

 // r/brogue Tuesday contest seed generator
 if (seed !== null && seed.includes('tuesday')) seed = getMostRecentTuesday();

const variant   = urlParams.get('variant'); // rapid_brogue, bullet_brogue
const stealth   = urlParams.get('stealth')    || urlParams.get('S');
const noEffects = urlParams.get('no-effects') || urlParams.get('E');
const wizard    = urlParams.get('wizard')     || urlParams.get('W');
const graphics  = urlParams.get('graphics')   || urlParams.get('G');
const hybrid    = urlParams.get('hybrid')     || urlParams.get('H');;

Module.arguments = [];

if (seed != null && seed != "") { Module.arguments.push("-s"); Module.arguments.push(seed); }
if (variant != null && variant != "") { Module.arguments.push("--variant"); Module.arguments.push(variant); }

if (stealth   != null) Module.arguments.push("--stealth");
if (noEffects != null) Module.arguments.push("--no-effects");
if (wizard    != null) Module.arguments.push("--wizard");
if (graphics  != null) Module.arguments.push("--graphics");
if (hybrid    != null) Module.arguments.push("--hybrid");
