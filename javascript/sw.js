const version = "20260219";
const assets = [
    "./",
    "./apple-touch-icon.png",
    "./brogue-icon-192.png",
    "./brogue-icon-256.png",
    "./brogue-icon-512.png",
    "./brogue.html",
    "./brogue.js",
    "./brogue.wasm",
    "./BrogueCE.ttf",
    "./bugs.svg",
    "./close.svg",
    "./delete.svg",
    "./download.svg",
    "./favicon.ico",
    "./files.svg",
    "./fullscreen.svg",
    "./index.html",
    "./info.svg",
    "./keyboard.svg",
    "./manifest.json",
    "./upload.svg",
    "./brogue-title-screen.jpg",
    "./brogue-screen-1.jpg",
    "./brogue-screen-3.jpg",
    "./brogue-screen-3.jpg",
];

self.addEventListener('install', e => {
    // Do precache assets
    e.waitUntil(
	    caches
	    .open(version)
	    .then(cache => {
		    cache.addAll(assets);
	    })
	    .then(() => self.skipWaiting())
    );
});

self.addEventListener('activate', (e) => {
    // Delete old versions of the cache
    e.waitUntil(
	    caches.keys().then((keys) => {
            return Promise.all( keys.filter(key => key != version).map(name => caches.delete(name)));
	    })
    );
});

const revalidate = async (req) => {
    console.log("Revalidating ", req.url);

    // Check if we are online, otherwise return the old value
    const isOnline = self.navigator.onLine;
    const oldVal = caches.match(req); // No await
    if (!isOnline) {
        console.log("Not online, using cache");
        return oldVal;
    }

    // Fetch the a new version
    const newVal = await fetch(req, { mode: 'cors', credentials: 'omit' });
    if (!newVal.ok) {
        console.log("Fetch failed, using cache");
        return oldVal;
    }

    console.log("Fetched - using new value");

    // Got a new value
    const cache  = await caches.open(version);  // --- TODO: This is wrong, it should pull the version from the resource, not from the service worker global, could cause resource version mismatches..?
    cache.put(req, newVal.clone()); // --- TODO? Could await and check result of put() here
    return newVal;
}

self.addEventListener('fetch', (e) => {
    if (e.request.method !== "GET") return;
    e.respondWith(revalidate(e.request));
});
