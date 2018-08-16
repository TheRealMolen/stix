# based on https://gist.github.com/ponderomotion/5104375

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

 
.SUFFIXES: .m .o
.m.o: ; @mkdir -p $(BINDIR) $(OBJDIR) 
	$(CXX) -c $(FFLAGS) -o $(OBJDIR)/$@ $< $(INCLUDES)

.SUFFIXES: .cpp .o
.cpp.o: ; @mkdir -p $(BINDIR) $(OBJDIR)
	$(CXX) -c $(FFLAGS) -o $(OBJDIR)/$@ $< $(INCLUDES)
 
$(FULLTARGET): $(OBJFILES)
	$(CXX) $(FFLAGS) -o $@ $(addprefix $(OBJDIR)/, $(OBJFILES)) $(LINCLUDES)

.PHONEY: clean
clean:
	@rm -rf *~ $(BINDIR) $(OBJDIR)  $(SRCDIR)/*~ *.out

%.o:

# dependencies
main.o: Makefile