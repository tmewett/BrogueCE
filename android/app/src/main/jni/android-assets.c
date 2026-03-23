/*
 * android-assets.c — Extract APK assets to internal storage.
 *
 * Android bundles assets inside the APK, accessible only through
 * SDL_RWFromFile or the Android AssetManager. BrogueCE's game code
 * uses fopen/IMG_Load with regular file paths, so we copy the assets
 * to the app's writable directory at startup.
 */

#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* Source names (as seen by Android AssetManager — no "assets/" prefix) */
static const char *assetSrc[] = {
    "icon.png",
    "tiles.png",
    "tiles.bin",
    NULL
};

/* Destination names (relative to destDir, matching what game code expects) */
static const char *assetDst[] = {
    "assets/icon.png",
    "assets/tiles.png",
    "assets/tiles.bin",
    NULL
};

static int fileExists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static int copyAsset(const char *srcName, const char *dstName, const char *destDir) {
    char destPath[1024];
    snprintf(destPath, sizeof(destPath), "%s/%s", destDir, dstName);

    if (fileExists(destPath))
        return 1;

    SDL_RWops *src = SDL_RWFromFile(srcName, "rb");
    if (!src) {
        SDL_Log("Failed to open asset %s: %s", srcName, SDL_GetError());
        return 0;
    }

    Sint64 size = SDL_RWsize(src);
    if (size <= 0) {
        SDL_RWclose(src);
        return 0;
    }

    void *buf = malloc(size);
    if (!buf) {
        SDL_RWclose(src);
        return 0;
    }

    SDL_RWread(src, buf, 1, size);
    SDL_RWclose(src);

    FILE *dst = fopen(destPath, "wb");
    if (!dst) {
        free(buf);
        return 0;
    }

    fwrite(buf, 1, size, dst);
    fclose(dst);
    free(buf);
    return 1;
}

void androidExtractAssets(const char *destDir) {
    /* Create assets/ subdirectory */
    char assetsDir[1024];
    snprintf(assetsDir, sizeof(assetsDir), "%s/assets", destDir);
    mkdir(assetsDir, 0755);

    for (int i = 0; assetSrc[i]; i++) {
        if (!copyAsset(assetSrc[i], assetDst[i], destDir))
            SDL_Log("Warning: could not extract %s", assetSrc[i]);
    }
}
