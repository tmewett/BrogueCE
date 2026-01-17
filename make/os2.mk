os2/icon.res: os2/icon.rc make/os2.mk
	wrc -qr $< -fo=$@

os2/brogue.lib: os2/brogue.def make/os2.mk
	emximp -o $@ $<
