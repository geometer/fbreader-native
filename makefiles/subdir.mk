include $(ROOTDIR)/makefiles/opts.mk

HEADERS = $(wildcard *.h)
SOURCES_CPP = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp, %.o, $(SOURCES_CPP))

.SUFFIXES: .cpp .o .h

.cpp.o:
	@echo -n 'Compiling $@ ...'
	@$(CC) $(CFLAGS) $(INCLUDE) $<
	@echo ' OK'

all: $(OBJECTS)

clean:
	@$(RM) *.o *.d

-include *.d
