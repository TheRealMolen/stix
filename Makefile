# based on https://gist.github.com/ponderomotion/5104375
# plus a load of gumpf to make a .app bundle!

SHELL = /bin/sh

CXX = g++
 
FFLAGS = -g -std=c++11
 
TARGET = stix
OBJDIR = obj
SRCDIR = stix
BINDIR = bin
VPATH = $(SRCDIR):$(OBJDIR)
FULLTARGET = $(BINDIR)/$(TARGET)

OBJFILES = main.o text.o

#INCLUDES = -I/Library/Frameworks/SDL.framework/Headers/
#INCLUDES += -I/Library/Frameworks/SDL_mixer.framework/Headers/
INCLUDES = -Iinclude

LINCLUDES  = -F/Library/Frameworks
LINCLUDES += -framework SDL2 -framework Cocoa
LINCLUDES += -framework SDL2_mixer

.PHONY: clean uploadmac

 
.SUFFIXES: .m .o
.m.o: ; @mkdir -p $(BINDIR) $(OBJDIR) 
	$(CXX) -c $(FFLAGS) -o $(OBJDIR)/$@ $< $(INCLUDES)

.SUFFIXES: .cpp .o
.cpp.o: ; @mkdir -p $(BINDIR) $(OBJDIR)
	$(CXX) -c $(FFLAGS) -o $(OBJDIR)/$@ $< $(INCLUDES)
 
$(FULLTARGET): $(OBJFILES)
	$(CXX) $(FFLAGS) -o $@ $(addprefix $(OBJDIR)/, $(OBJFILES)) $(LINCLUDES)
	cp $(FULLTARGET) ./stixapp

clean:
	@rm -rf *~ $(BINDIR) $(OBJDIR)  $(SRCDIR)/*~ *.out
	rm -rf $(APPBUNDLE)

%.o:

# dependencies
main.o: Makefile


# packaging fun, from https://stackoverflow.com/a/23898134

APPNAME=stix
APPBUNDLE=$(APPNAME).app
APPBUNDLECONTENTS=$(APPBUNDLE)/Contents
APPBUNDLEEXE=$(APPBUNDLECONTENTS)/MacOS
APPBUNDLERESOURCES=$(APPBUNDLECONTENTS)/Resources
APPBUNDLEICON=$(APPBUNDLECONTENTS)/Resources
appbundle: macosx/$(APPNAME).icns
	rm -rf $(APPBUNDLE)
	mkdir $(APPBUNDLE)
	mkdir $(APPBUNDLE)/Contents
	mkdir $(APPBUNDLE)/Contents/MacOS
	mkdir $(APPBUNDLE)/Contents/Resources
	mkdir $(APPBUNDLE)/Contents/Frameworks
	cp macosx/Info.plist $(APPBUNDLECONTENTS)/
	cp macosx/PkgInfo $(APPBUNDLECONTENTS)/
	cp macosx/$(APPNAME).icns $(APPBUNDLEICON)/
	cp -R /Library/Frameworks/SDL2.framework $(APPBUNDLE)/Contents/Frameworks
	cp -R /Library/Frameworks/SDL2_mixer.framework $(APPBUNDLE)/Contents/Frameworks
	cp $(FULLTARGET) $(APPBUNDLEEXE)/$(APPNAME)
	cp -R data $(APPBUNDLEEXE)

macosx/$(APPNAME).icns: macosx/$(APPNAME)Icon.png
	rm -rf macosx/$(APPNAME).iconset
	mkdir macosx/$(APPNAME).iconset
	sips -z 16 16     macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_16x16.png
	sips -z 32 32     macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_16x16@2x.png
	sips -z 32 32     macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_32x32.png
	sips -z 64 64     macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_32x32@2x.png
	sips -z 128 128   macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_128x128.png
	sips -z 256 256   macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_128x128@2x.png
	sips -z 256 256   macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_256x256.png
	sips -z 512 512   macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_256x256@2x.png
	sips -z 512 512   macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_512x512.png
	cp macosx/$(APPNAME)Icon.png macosx/$(APPNAME).iconset/icon_512x512@2x.png
	iconutil -c icns -o macosx/$(APPNAME).icns macosx/$(APPNAME).iconset
	rm -r macosx/$(APPNAME).iconset


uploadmac:
	butler push $(APPBUNDLE) molen/stix:osx