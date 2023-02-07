windows/icon.o: windows/icon.rc make/brogue_windows.mk
	windres $< $@

bin/brogue.exe: $(objects) windows/icon.o vars/cflags vars/LDFLAGS vars/libs vars/objects make/brogue_windows.mk
	$(CC) $(cflags) $(LDFLAGS) -o $@ $(objects) windows/icon.o $(libs)
	mt -manifest windows/brogue.exe.manifest '-outputresource:bin/brogue.exe;1'
