$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$sdlVersion = "2.0.10"
$sdlImageVersion = "2.0.5"

$sdlArhive = "SDL2-devel-$sdlVersion-mingw.tar.gz"
$sdlImageArhive = "SDL2_image-devel-$sdlImageVersion-mingw.tar.gz"

$sdlUrl = "https://www.libsdl.org/release/$sdlArhive"
$sdlImageUrl = "https://www.libsdl.org/projects/SDL_image/release/$sdlImageArhive"

"Downloading $sdlUrl" | Write-Host
curl.exe -sSLfO $sdlUrl
if ($LASTEXITCODE) { "Download failed" | Write-Host: exit 1 }

"Downloading $sdlImageUrl" | Write-Host
curl.exe -sSLfO $sdlImageUrl
if ($LASTEXITCODE) { "Download failed" | Write-Host: exit 1 }

$target = Join-Path $pwd.Drive.Root "opt/local/x86_64-w64-mingw32"
mkdir $target

tar xf $sdlArhive
gci ".\SDL2-$sdlVersion\x86_64-w64-mingw32" | cp -dest $target -recurse -force
mv ".\SDL2-$sdlVersion" ".\SDL2"

tar xf $sdlImageArhive
gci ".\SDL2_image-$sdlImageVersion\x86_64-w64-mingw32" | cp -dest $target -recurse -force
mv ".\SDL2_image-$sdlImageVersion" ".\SDL2_image"
