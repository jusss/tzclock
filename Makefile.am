AUTOMAKE_OPTIONS = dist-bzip2
bin_PROGRAMS = TzClock screenSize
TzClock_SOURCES = src/TzClock.c src/TzClockCairo.c src/ParseZone.c src/TzClockConfig.c src/list.c src/TzClockDisp.h src/TzClockConfig.h src/TimeZone.h src/ParseZone.h src/list.h src/TzClockIcon.xpm src/TzClockIcon_small.xpm
screenSize_SOURCES = src/screenSize.c
AM_CPPFLAGS = $(DEPS_CFLAGS)
LIBS = $(DEPS_LIBS)
EXTRA_DIST = TzClock.desktop icons/48x48/tzclock.png icons/128x128/tzclock.png icons/scalable/tzclock.svg TzClock.man COPYING AUTHORS
Applicationsdir = $(datadir)/applications
Applications_DATA = TzClock.desktop 
Icondir = $(datadir)/icons/hicolor/scalable/apps
Icon_DATA = icons/scalable/tzclock.svg
Icon128dir = $(datadir)/icons/hicolor/128x128/apps
Icon128_DATA = icons/128x128/tzclock.png
Icon48dir = $(datadir)/icons/hicolor/48x48/apps
Icon48_DATA = icons/48x48/tzclock.png
man1_MANS = TzClock.man

