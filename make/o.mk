$(objdir):
	mkdir -p $(objdir)/src/brogue
	mkdir -p $(objdir)/src/platform
	mkdir -p $(objdir)/src/variants

$(objdir)/%.o : %.c src/brogue/Rogue.h src/brogue/Globals.h src/brogue/GlobalsBase.h vars/cppflags vars/cflags vars/extra_version make/o.mk | $(objdir)
	$(CC) $(cppflags) $(cflags) -c $< -o $@
