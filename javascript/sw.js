const version = "260304";
const assets = [
    // Priority
    "./brogue.html",
    "./brogue.js",
    "./brogue.wasm",
    "./BrogueCE.ttf",
    "./direction-pad.svg",
    "./esc-button.png",
    "./manifest.json",

    // UI Elements
    "./bugs.svg",
    "./close.svg",
    "./delete.svg",
    "./dir-pad.svg",
    "./download.svg",
    "./files.svg",
    "./fullscreen.svg",
    "./info.svg",
    "./keyboard.svg",
    "./upload.svg",

    // Manifest data
    "./favicon.ico",
    "./brogue-icon-192.png",
    "./brogue-icon-256.png",
    "./brogue-icon-512.png",
    "./brogue-title-screen.jpg",
    "./brogue-screen-1.jpg",
    "./brogue-screen-2.jpg",
    "./brogue-screen-3.jpg",
];

const sendMessage = (data) => {
  // Send a message to all clients
  self.clients.matchAll().then(clients => {
    clients.forEach(client => {
      client.postMessage(data);
    });
  });
}

const refresh = `<!DOCTYPE html><body><h1>Service worker cache cleared!</h1></body>`;

const staticPage = (html) => {
  return new Response(html, { headers: { "content-type": "text/html; charset=UTF-8" } });
}

const preCache = async () => {
  const cache = await caches.open(version)
  return cache.addAll(assets);
}

const drainCache = async (keepVersion) => {
  const allVersions = await caches.keys();
  const oldVersions = allVersions.filter(verId => verId != keepVersion);
  return Promise.all(oldVersions.map(verId => caches.delete(verId)));
}

const putInCache = async (request, response) => {
  console.log("putInCache ", request.url, version);
  const cache = await caches.open(version);
  await cache.put(request, response);
};

const fromCache = async (req) => {
  const oldVal = await caches.match(req);
  if (oldVal) {
    console.log("fromCache", req.url, version);
    return oldVal;
  }

  try {
    // Fetch the new version
    const newVal = await fetch(req.clone());
    putInCache(req, newVal.clone());
    return newVal;
  } catch (err) {
    // TODO: Return static assets for various things (png, svg, etc.)
    console.log("..failed!", req.url, err);
    return new Response("Network error happened", {
      status: 408,
      headers: { "Content-Type": "text/plain" },
    });
  }
}

// Installation of a new service worker version
self.addEventListener('install', e => {
  e.waitUntil(preCache().then(() => self.skipWaiting()));
});

// New service worker version is now in control, clear old caches
self.addEventListener('activate', e => {
  e.waitUntil(drainCache(version).then( () => self.clients.claim() ));
});

// App wants to load a resource
self.addEventListener('fetch', e => {
  if (e.request.method !== "GET") return;
  if (e.request.url.includes('/REFRESH')) { drainCache("all").then( e.respondWith(staticPage(refresh)) ); }
  else e.respondWith(fromCache(e.request));
});

// Receive message from main page
self.addEventListener("message", e => {
  if (!e.data) return;
  switch (e.data.type) {
  case "SKIP_WAITING": self.skipWaiting(); break;
  case "GET_VERSION":  e.ports[0].postMessage({version}); break;
  }
});
