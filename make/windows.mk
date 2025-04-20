windows/resources.o: windows/resources.rc make/windows.mk
	windres $< $@
