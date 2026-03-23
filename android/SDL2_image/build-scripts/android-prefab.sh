#!/bin/bash

set -e

if ! [ "x$ANDROID_NDK_HOME" != "x" -a -d "$ANDROID_NDK_HOME" ]; then
    echo "ANDROID_NDK_HOME environment variable is not set"
    exit 1
fi

if ! [ "x$ANDROID_HOME" != "x" -a -d "$ANDROID_HOME" ]; then
    echo "ANDROID_HOME environment variable is not set"
    exit 1
fi

if [ "x$ANDROID_API" = "x" ]; then
    ANDROID_API="$(ls "$ANDROID_HOME/platforms" | grep -E "^android-[0-9]+$" | sed 's/android-//' | sort -n -r | head -1)"
    if [ "x$ANDROID_API" = "x" ]; then
        echo "No Android platform found in $ANDROID_HOME/platforms"
        exit 1
    fi
else
    if ! [ -d "$ANDROID_HOME/platforms/android-$ANDROID_API" ]; then
        echo "Android api version $ANDROID_API is not available ($ANDROID_HOME/platforms/android-$ANDROID_API does not exist)" >2
        exit 1
    fi
fi

android_platformdir="$ANDROID_HOME/platforms/android-$ANDROID_API"

echo "Building for android api version $ANDROID_API"
echo "android_platformdir=$android_platformdir"

scriptdir=$(cd -P -- "$(dirname -- "$0")" && printf '%s\n' "$(pwd -P)")
sdlimage_root=$(cd -P -- "$(dirname -- "$0")/.." && printf '%s\n' "$(pwd -P)")

build_root="${sdlimage_root}/build-android-prefab"

android_abis="armeabi-v7a arm64-v8a x86 x86_64"
android_api=19
android_ndk=21
android_stl="c++_shared"

sdlimage_major=$(sed -ne 's/^#define SDL_IMAGE_MAJOR_VERSION  *//p' "${sdlimage_root}/SDL_image.h")
sdlimage_minor=$(sed -ne 's/^#define SDL_IMAGE_MINOR_VERSION  *//p' "${sdlimage_root}/SDL_image.h")
sdlimage_patch=$(sed -ne 's/^#define SDL_IMAGE_PATCHLEVEL  *//p' "${sdlimage_root}/SDL_image.h")
sdlimage_version="${sdlimage_major}.${sdlimage_minor}.${sdlimage_patch}"
echo "Building Android prefab package for SDL_image version $sdlimage_version"

if test ! -d "${sdl_build_root}"; then
    echo "sdl_build_root is not defined or is not a directory."
    echo "Set this environment folder to the root of an android SDL${sdlimage_major} prefab build"
    echo "This usually is SDL/build-android-prefab"
    exit 1
fi

prefabhome="${build_root}/prefab-${sdlimage_version}"
rm -rf "$prefabhome"
mkdir -p "${prefabhome}"

build_cmake_projects() {
    for android_abi in $android_abis; do

        rm -rf "${build_root}/build_${android_abi}/prefix"

        for build_shared_libs in ON OFF; do
            echo "Configuring CMake project for $android_abi (shared=${build_shared_libs})"
            cmake -S "${sdlimage_root}" -B "${build_root}/build_${android_abi}/shared_${build_shared_libs}" \
                -DSDL2IMAGE_DEPS_SHARED=ON \
                -DSDL2IMAGE_VENDORED=ON \
                -DSDL2IMAGE_BACKEND_STB=OFF \
                -DSDL2IMAGE_AVIF=OFF \
                -DSDL2IMAGE_BMP=ON \
                -DSDL2IMAGE_GIF=ON \
                -DSDL2IMAGE_JPG=ON \
                -DSDL2IMAGE_JXL=OFF \
                -DSJPEG_ANDROID_NDK_PATH="${ANDROID_NDK_HOME}" \
                -DSDL2IMAGE_LBM=ON \
                -DSDL2IMAGE_PCX=ON \
                -DSDL2IMAGE_PNG=ON \
                -DSDL2IMAGE_PNM=ON \
                -DSDL2IMAGE_QOI=ON \
                -DSDL2IMAGE_SVG=ON \
                -DSDL2IMAGE_TGA=ON \
                -DSDL2IMAGE_TIF=ON \
                -DSDL2IMAGE_WEBP=ON \
                -DSDL2IMAGE_XCF=ON \
                -DSDL2IMAGE_XPM=ON \
                -DSDL2IMAGE_XV=ON \
                -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
                -DSDL${sdlimage_major}_DIR="${sdl_build_root}/build_${android_abi}/prefix/lib/cmake/SDL${sdlimage_major}" \
                -DANDROID_PLATFORM=${android_platform} \
                -DANDROID_ABI=${android_abi} \
                -DBUILD_SHARED_LIBS=${build_shared_libs} \
                -DCMAKE_INSTALL_PREFIX="${build_root}/build_${android_abi}/prefix" \
                -DCMAKE_INSTALL_INCLUDEDIR=include \
                -DCMAKE_INSTALL_LIBDIR=lib \
                -DCMAKE_BUILD_TYPE=Release \
                -DSDL${sdlimage_major}IMAGE_SAMPLES=OFF \
                -DSDL${sdlimage_major}IMAGE_TESTS=OFF \
                -GNinja

            echo "Building CMake project for $android_abi (shared=${build_shared_libs})"
            cmake --build "${build_root}/build_${android_abi}/shared_${build_shared_libs}"

            echo "Installing CMake project for $android_abi (shared=${build_shared_libs})"
            cmake --install "${build_root}/build_${android_abi}/shared_${build_shared_libs}"
        done
    done
}

pom_filename="SDL${sdlimage_major}_image-${sdlimage_version}.pom"
pom_filepath="${prefabhome}/${pom_filename}"
create_pom_xml() {
    echo "Creating ${pom_filename}"
    cat >"${pom_filepath}" <<EOF
<project>
  <modelVersion>4.0.0</modelVersion>
  <groupId>org.libsdl.android</groupId>
  <artifactId>SDL${sdlimage_major}_image</artifactId>
  <version>${sdlimage_version}</version>
  <packaging>aar</packaging>
  <name>SDL${sdlimage_major}_image</name>
  <description>The AAR for SDL${sdlimage_major}_image</description>
  <url>https://libsdl.org/</url>
  <licenses>
    <license>
      <name>zlib License</name>
      <url>https://github.com/libsdl-org/SDL_image/blob/main/LICENSE.txt</url>
      <distribution>repo</distribution>
    </license>
  </licenses>
  <developers>
    <developer>
      <name>Sam Lantinga</name>
      <email>slouken@libsdl.org</email>
      <organization>SDL</organization>
      <organizationUrl>https://www.libsdl.org</organizationUrl>
    </developer>
  </developers>
  <scm>
    <connection>scm:git:https://github.com/libsdl-org/SDL_image</connection>
    <developerConnection>scm:git:ssh://github.com:libsdl-org/SDL_image.git</developerConnection>
    <url>https://github.com/libsdl-org/SDL_image</url>
  </scm>
  <distributionManagement>
    <snapshotRepository>
      <id>ossrh</id>
      <url>https://s01.oss.sonatype.org/content/repositories/snapshots</url>
    </snapshotRepository>
    <repository>
      <id>ossrh</id>
      <url>https://s01.oss.sonatype.org/service/local/staging/deploy/maven2/</url>
    </repository>
  </distributionManagement>
</project>
EOF
}

create_aar_androidmanifest() {
    echo "Creating AndroidManifest.xml"
    cat >"${aar_root}/AndroidManifest.xml" <<EOF
<manifest
    xmlns:android="http://schemas.android.com/apk/res/android"
    package="org.libsdl.android" android:versionCode="1"
    android:versionName="1.0">
	<uses-sdk android:minSdkVersion="16"
              android:targetSdkVersion="29"/>
</manifest>
EOF
}

echo "Creating AAR root directory"
aar_root="${prefabhome}/SDL${sdlimage_major}_image-${sdlimage_version}"
mkdir -p "${aar_root}"

aar_metainfdir_path=${aar_root}/META-INF
mkdir -p "${aar_metainfdir_path}"
cp "${sdlimage_root}/LICENSE.txt" "${aar_metainfdir_path}"

prefabworkdir="${aar_root}/prefab"
mkdir -p "${prefabworkdir}"

cat >"${prefabworkdir}/prefab.json" <<EOF
{
  "schema_version": 2,
  "name": "SDL${sdlimage_major}_image",
  "version": "${sdlimage_version}",
  "dependencies": ["SDL${sdlimage_major}"]
}
EOF

modulesworkdir="${prefabworkdir}/modules"
mkdir -p "${modulesworkdir}"

create_shared_sdl_image_module() {
    echo "Creating SDL${sdlimage_major}_image prefab module"
    for android_abi in $android_abis; do
        sdl_moduleworkdir="${modulesworkdir}/SDL${sdlimage_major}_image"
        mkdir -p "${sdl_moduleworkdir}"

        abi_build_prefix="${build_root}/build_${android_abi}/prefix"

        cat >"${sdl_moduleworkdir}/module.json" <<EOF
{
  "export_libraries": ["//SDL${sdlimage_major}:SDL${sdlimage_major}"],
  "library_name": "libSDL${sdlimage_major}_image"
}
EOF
        mkdir -p "${sdl_moduleworkdir}/include"
        cp -r "${abi_build_prefix}/include/SDL${sdlimage_major}/"* "${sdl_moduleworkdir}/include/"

        abi_sdllibdir="${sdl_moduleworkdir}/libs/android.${android_abi}"
        mkdir -p "${abi_sdllibdir}"
        cat >"${abi_sdllibdir}/abi.json" <<EOF
{
  "abi": "${android_abi}",
  "api": ${android_api},
  "ndk": ${android_ndk},
  "stl": "${android_stl}",
  "static": false
}
EOF
        cp "${abi_build_prefix}/lib/libSDL${sdlimage_major}_image.so" "${abi_sdllibdir}"
    done
}

create_static_sdl_image_module() {
    echo "Creating SDL${sdlimage_major}_image-static prefab module"
    for android_abi in $android_abis; do
        sdl_moduleworkdir="${modulesworkdir}/SDL${sdlimage_major}_image-static"
        mkdir -p "${sdl_moduleworkdir}"

        abi_build_prefix="${build_root}/build_${android_abi}/prefix"

        cat >"${sdl_moduleworkdir}/module.json" <<EOF
{
  "export_libraries": ["//SDL${sdlimage_major}:SDL${sdlimage_major}-static"],
  "library_name": "libSDL${sdlimage_major}_image"
}
EOF
        mkdir -p "${sdl_moduleworkdir}/include"
        cp -r "${abi_build_prefix}/include/SDL${sdlimage_major}/"* "${sdl_moduleworkdir}/include"

        abi_sdllibdir="${sdl_moduleworkdir}/libs/android.${android_abi}"
        mkdir -p "${abi_sdllibdir}"
        cat >"${abi_sdllibdir}/abi.json" <<EOF
{
  "abi": "${android_abi}",
  "api": ${android_api},
  "ndk": ${android_ndk},
  "stl": "${android_stl}",
  "static": true
}
EOF
        cp "${abi_build_prefix}/lib/libSDL${sdlimage_major}_image.a" "${abi_sdllibdir}"
    done
}

create_shared_module() {
    modulename=$1
    libraryname=$2
    export_libraries=$3
    echo "Creating $modulename prefab module"

    export_libraries_json="[]"
    if test "x$export_libraries" != "x"; then
        export_libraries_json="["
        for export_library in $export_libraries; do
            if test "x$export_libraries_json" != "x["; then
                export_libraries_json="$export_libraries_json, "
            fi
            export_libraries_json="$export_libraries_json\"$export_library\""
        done
        export_libraries_json="$export_libraries_json]"
    fi

    for android_abi in $android_abis; do
        sdl_moduleworkdir="${modulesworkdir}/$modulename"
        mkdir -p "${sdl_moduleworkdir}"

        abi_build_prefix="${build_root}/build_${android_abi}/prefix"

        cat >"${sdl_moduleworkdir}/module.json" <<EOF
{
  "export_libraries": $export_libraries_json,
  "library_name": "$libraryname"
}
EOF

        abi_sdllibdir="${sdl_moduleworkdir}/libs/android.${android_abi}"
        mkdir -p "${abi_sdllibdir}"
        cat >"${abi_sdllibdir}/abi.json" <<EOF
{
  "abi": "${android_abi}",
  "api": ${android_api},
  "ndk": ${android_ndk},
  "stl": "${android_stl}"
}
EOF
        cp "${abi_build_prefix}/lib/$libraryname.so" "${abi_sdllibdir}"
    done
}

build_cmake_projects

create_pom_xml

create_aar_androidmanifest

create_shared_sdl_image_module

create_static_sdl_image_module

create_shared_module external_zlib libz ""
head -n28 "${sdlimage_root}/external/zlib/zlib.h" | tail -n25 >"${aar_metainfdir_path}/LICENSE.zlib.txt"

create_shared_module external_libpng libpng16 ":external_zlib"
cp "${sdlimage_root}/external/libpng/LICENSE" "${aar_metainfdir_path}/LICENSE.libpng.txt"

create_shared_module external_libjpeg libjpeg ""
cp "${sdlimage_root}/external/jpeg/README" "${aar_metainfdir_path}/LICENSE.libjpeg.txt"

create_shared_module external_libtiff libtiff ""
cp "${sdlimage_root}/external/libtiff/COPYRIGHT" "${aar_metainfdir_path}/LICENSE.libtiff.txt"

create_shared_module external_libwebp libwebp ""
cp "${sdlimage_root}/external/libwebp/COPYING" "${aar_metainfdir_path}/LICENSE.libwebp.txt"

#create_shared_module libbrotlicommon libbrotlicommon ""
#create_shared_module libbrotlidec libbrotlidec ":libbrotlicommon"
#create_shared_module libbrotlienc libbrotlienc ":libbrotlicommon"
#cp "${sdlimage_root}/external/libjxl/third_party/brotli/LICENSE" "${aar_metainfdir_path}/LICENSE.brotli.txt"

#create_shared_module external_libjxl libjxl ":brotlienc :brotlidec"
#cp "${sdlimage_root}/external/libjxl/LICENSE" "${aar_metainfdir_path}/LICENSE.libjxl.txt"

pushd "${aar_root}"
    aar_filename="SDL${sdlimage_major}_image-${sdlimage_version}.aar"
    zip -r "${aar_filename}" AndroidManifest.xml prefab META-INF
    zip -Tv "${aar_filename}" 2>/dev/null ;
    mv "${aar_filename}" "${prefabhome}"
popd

maven_filename="SDL${sdlimage_major}_image-${sdlimage_version}.zip"

pushd "${prefabhome}"
    zip_filename="SDL${sdlimage_major}_image-${sdlimage_version}.zip"
    zip "${maven_filename}" "${aar_filename}" "${pom_filename}" 2>/dev/null;
    zip -Tv "${zip_filename}" 2>/dev/null;
popd

echo "Prefab zip is ready at ${prefabhome}/${aar_filename}"
echo "Maven archive is ready at ${prefabhome}/${zip_filename}"
