#!/usr/bin/env bash
cd `dirname $0`
sed s?PATH?`pwd`?g > brogue.desktop <<-end
	[Desktop Entry]
	Version=1.0
	Type=Application
	Name=Brogue
	GenericName=Roguelike
	Comment=Brave the Dungeons of Doom!
	Comment[de]=Meistere die Dungeons der Verdammnis!
	Comment[ru]=Бросьте вызов Подземельям Погибели!
	Exec=PATH/brogue
	Path=PATH
	Icon=PATH/bin/assets/icon.png
	Terminal=false
	Categories=Game;
end

chmod +x brogue.desktop

echo "Created brogue.desktop."
echo "If your desktop is in the default location, type:"
echo "mv brogue.desktop ~/Desktop"
