bin/brogue bin/brogue.exe: $(objects) vars/cflags vars/LDFLAGS vars/libs vars/objects make/brogue.mk
	$(CC) $(cflags) $(LDFLAGS) -o $@ $(objects) $(libs)
# on windows, embedding the manifest modifies the executable, preventing debugging
ifeq ($(SYSTEM),WINDOWS)
ifeq ($(DEBUG),NO)
	mt -manifest windows/brogue.exe.manifest '-outputresource:bin/brogue.exe;1'
endif
endif
