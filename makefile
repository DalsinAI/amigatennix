
#
# Tennix! SDL Port
# Copyright (C) 2003, 2007, 2008, 2009 Thomas Perl <thp@thpinfo.com>
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
# MA  02110-1301, USA.
#

-include config.mk

CONFIGURE_OUTPUT := config.mk config.h dependencies.mk

ifeq ($(SILENT),0)
    SILENTMSG := @true
    SILENTCMD := 
else
    SILENTMSG := @echo
    SILENTCMD := @
endif

# Object files for "tennix"
TENNIX_BIN := tennix
TENNIX_OBJ += src/tennix.o src/game.o src/graphics.o src/input.o
TENNIX_OBJ += src/util.o src/sound.o src/animation.o src/archive.o
TENNIX_OBJ += src/network.o src/tennixpy.o

# Object files for "tnxar"
ARCHIVE_BIN := tnxar
ARCHIVE_OBJ += src/archivetool.o src/archive.o

# Data files for tennix.tnx
TENNIX_TNX := tennix.tnx
TENNIX_TNX_DATA += $(wildcard data/*.ogg)
TENNIX_TNX_DATA += $(wildcard data/*.ttf)
TENNIX_TNX_DATA += $(wildcard data/*.png)

ifeq ($(HAVE_VOICE_FILES),1)
TENNIX_TNX_DATA += $(wildcard voice/*.ogg)
endif

ifeq ($(HAVE_PYTHON),1)
TENNIX_TNX_DATA += $(wildcard data/*.py)
endif

TENNIX_TNX_DATA := $(sort $(TENNIX_TNX_DATA))

# Installable files
TENNIX_PNG := data/tennix.png
TENNIX_DESKTOP := data/tennix.desktop
TENNIX_MAN := data/tennix.6

all: $(TENNIX_BIN) $(ARCHIVE_BIN) $(TENNIX_TNX)

%.o: %.cc
	$(SILENTMSG) "  CXX     $@"
	$(SILENTCMD)$(CXX) -c $(CFLAGS) -o $@ $<

$(TENNIX_BIN): $(TENNIX_OBJ)
	$(SILENTMSG) "  LD      $@"
	$(SILENTCMD)$(CXX) -o $@ $^ $(LDFLAGS)

$(ARCHIVE_BIN): $(ARCHIVE_OBJ)
	$(SILENTMSG) "  LD      $@"
	$(SILENTCMD)$(CXX) -o $@ $^

$(TENNIX_TNX): $(ARCHIVE_BIN) $(TENNIX_TNX_DATA)
	$(SILENTMSG) "  TNXAR   $@"
	$(SILENTCMD)./$(ARCHIVE_BIN) -cf $@ $(TENNIX_TNX_DATA)

install: $(TENNIX_BIN) $(TENNIX_TNX) $(TENNIX_PNG) $(TENNIX_DESKTOP) $(TENNIX_MAN)
	$(SILENTMSG) "  INSTALL $(TENNIX_BIN)"
	$(SILENTCMD)$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin/
	$(SILENTCMD)$(INSTALL) -m 755 $(TENNIX_BIN) $(DESTDIR)$(PREFIX)/bin/
	$(SILENTMSG) "  INSTALL $(TENNIX_TNX)"
	$(SILENTCMD)$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/tennix/
	$(SILENTCMD)$(INSTALL) -m 644 $(TENNIX_TNX) $(DESTDIR)$(PREFIX)/share/tennix/
	$(SILENTMSG) "  INSTALL $(TENNIX_PNG)"
	$(SILENTCMD)$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/pixmaps/
	$(SILENTCMD)$(INSTALL) -m 644 $(TENNIX_PNG) $(DESTDIR)$(PREFIX)/share/pixmaps/
	$(SILENTMSG) "  INSTALL $(TENNIX_DESKTOP)"
	$(SILENTCMD)$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/applications/
	$(SILENTCMD)$(INSTALL) -m 644 $(TENNIX_DESKTOP) $(DESTDIR)$(PREFIX)/share/applications/
	$(SILENTMSG) "  INSTALL $(TENNIX_MAN)"
	$(SILENTCMD)$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/man/man6/
	$(SILENTCMD)$(INSTALL) -m 644 $(TENNIX_MAN) $(DESTDIR)$(PREFIX)/share/man/man6/

clean:
	$(SILENTMSG) "  CLEAN"
	$(SILENTCMD)$(RM) -f $(TENNIX_BIN) $(TENNIX_OBJ)
	$(SILENTCMD)$(RM) -f $(ARCHIVE_BIN) $(ARCHIVE_OBJ)
	$(SILENTCMD)$(RM) -f $(TENNIX_TNX)

distclean: clean
	$(SILENTMSG) "  DISTCLEAN"
	$(SILENTCMD)$(RM) -f $(CONFIGURE_OUTPUT)

$(CONFIGURE_OUTPUT): configure
	$(SILENTMSG) "  CONFIGURE"
	$(SILENTCMD)./$<

-include dependencies.mk

.PHONY: install clean distclean
