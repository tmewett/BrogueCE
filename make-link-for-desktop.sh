#!/usr/bin/env bash
cd `dirname $0`
sed s?PATH?`pwd`?g > brogue.desktop <<-end
	[Desktop Entry]
	Version=1.0
	Type=Application
	Name=Brogue 1.7.2
	GenericName=Roguelike
	Comment=Brave the Dungeons of Doom!
	Exec=PATH/brogue
	Path=PATH
	Icon=PATH/bin/brogue-icon.png
	Terminal=false

end

chmod +x brogue.desktop

echo "Created brogue.desktop."
echo "If your desktop is in the default location, type:"
echo "mv brogue.desktop ~/Desktop"

