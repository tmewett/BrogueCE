bin/brogue$(.exe): $(objects) vars/cflags vars/LDFLAGS vars/libs vars/objects make/brogue.mk
	$(CC) $(cflags) $(LDFLAGS) -o $@ $(objects) $(libs)
ifeq ($(SYSTEM),WINDOWS)
	mt -manifest windows/brogue.exe.manifest '-outputresource:bin/brogue.exe;1'
endif
