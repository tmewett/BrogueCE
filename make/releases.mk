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

# The homebrew-core commit to take the SDL3 and sdl2-compat formulae from. Pinning both to one commit makes macOS
# builds reproducible and keeps the two versions matched (sdl2-compat loads SDL3 at runtime). Update it deliberately,
# to a commit where both formulae are versions known to work: e.g. sdl2-compat 2.32.72 scales mouse coordinates
# wrongly on HiDPI displays (libsdl-org/sdl2-compat#624).
# bf15382d (2026-10-19): SDL3 3.4.18, sdl2-compat 2.32.74
HOMEBREW_CORE_COMMIT := bf15382d54581a9a5f9836fbe141cdd1b57eff67
HOMEBREW_FORMULA_URL := https://raw.githubusercontent.com/Homebrew/homebrew-core/$(HOMEBREW_CORE_COMMIT)/Formula

macos/sdl3.orig.rb:
	curl -fL '$(HOMEBREW_FORMULA_URL)/s/sdl3.rb' >$@

macos/sdl3.rb: macos/sdl3.orig.rb
	patch -o $@ macos/sdl3.orig.rb macos/sdl-deployment-target.patch

macos/sdl2-compat.orig.rb:
	curl -fL '$(HOMEBREW_FORMULA_URL)/s/sdl2-compat.rb' >$@

macos/sdl2-compat.rb: macos/sdl2-compat.orig.rb
	patch -o $@ macos/sdl2-compat.orig.rb macos/sdl2-compat-deployment-target.patch
