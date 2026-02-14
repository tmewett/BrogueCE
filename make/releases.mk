# Release archives

common_bin := bin/assets bin/keymap.txt

define make_release_base
	mkdir $@
	cp README.md $@/README.txt
	cp CHANGELOG.md $@/CHANGELOG.txt
	cp LICENSE.txt $@
endef

# Flatten bin/ in the Windows archive
BrogueCE-windows:
	$(make_release_base)
	cp -r $(common_bin) bin/{brogue.exe,brogue-cmd.bat} $@

BrogueCE-macos:
	$(make_release_base)
	cp -r Brogue.app $@/"Brogue CE.app"

BrogueCE-linux:
	$(make_release_base)
	cp brogue $@
	cp -r --parents $(common_bin) bin/brogue $@
	cp linux/make-link-for-desktop.sh $@

# macOS app bundle

# $* is the matched %
icon_%.png: bin/assets/icon.png
	convert $< -resize $* $@

macos/Brogue.icns: icon_32.png icon_128.png icon_256.png icon_512.png
	png2icns $@ $^
	$(RM) $^

Brogue.app: bin/brogue
	mkdir -p $@/Contents/{MacOS,Resources}
	cp macos/Info.plist $@/Contents
	cp bin/brogue $@/Contents/MacOS
	cp -r macos/Brogue.icns bin/assets $@/Contents/Resources

macos/sdl2.rb:
	curl -L 'https://raw.githubusercontent.com/Homebrew/homebrew-core/master/Formula/s/sdl2.rb' >$@
	patch $@ macos/sdl2-deployment-target.patch

# iPad

# Define project and scheme names
PROJECT = ios/iBrogueCE_iPad.xcodeproj
SCHEME = iBrogueCE_iPad

# Target to build the app for a simulator (Debug configuration)
BrogueCE-ios-build:
	xcodebuild build -project $(PROJECT) -scheme $(SCHEME) -configuration Debug -sdk iphonesimulator -allowProvisioningUpdates

# Target to build an archive (for distribution or testing on a physical device)
BrogueCE-ios-release:
	xcodebuild archive -project $(PROJECT) -scheme $(SCHEME) -configuration Release -destination "generic/platform=iOS"

# Target to run tests
BrogueCE-ios-test:
	xcodebuild test -project $(PROJECT) -scheme $(SCHEME) -destination 'platform=iOS Simulator,name=iPhone 15'

# Target to clean the build directory
BrogueCE-ios-clean:
	xcodebuild clean -project $(PROJECT) -scheme $(SCHEME)

BrogueCE-ios: BrogueCE-ios-build BrogueCE-ios-clean
