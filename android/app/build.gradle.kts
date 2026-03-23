plugins {
    id("com.android.application")
}

android {
    namespace = "org.broguece.game"
    compileSdk = 35
    ndkVersion = "30.0.14904198"

    defaultConfig {
        applicationId = "org.broguece.game"
        minSdk = 24
        targetSdk = 35
        versionCode = 1
        versionName = "1.15.1"

        ndk {
            abiFilters += listOf("arm64-v8a", "armeabi-v7a", "x86_64")
        }

        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                )
                cFlags += listOf(
                    "-std=c99",
                    "-Wall",
                    "-Wno-parentheses",
                    "-Wno-unused-result",
                    "-Wno-format",
                    "-Wno-incompatible-pointer-types-discards-qualifiers",
                    "-O2",
                )
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/jni/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    signingConfigs {
        getByName("debug") // uses ~/.android/debug.keystore automatically
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            signingConfig = signingConfigs.getByName("debug")
        }
        debug {
            isDebuggable = true
            isJniDebuggable = true
        }
    }

    sourceSets {
        getByName("main") {
            // Game assets (tiles, icons, keymap)
            assets.srcDirs("../../bin/assets")
            // SDL2's required Java glue classes (SDLActivity, etc.)
            java.srcDirs(
                "src/main/java",
                "../SDL2/android-project/app/src/main/java"
            )
        }
    }
}
