windows/icon.o: windows/icon.rc make/windows.mk
	windres $< $@
