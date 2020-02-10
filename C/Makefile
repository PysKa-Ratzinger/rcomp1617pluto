CC=g++
CFLAGS=-O2 -Wall -Wextra -Werror -pedantic-errors -g `pkg-config --cflags gtkmm-3.0`
LDFLAGS=-lncurses `pkg-config --libs gtkmm-3.0` -lpthread

SRCDIR = ./src
OBJDIR = ./build
BINDIR = ./bin
TARGET = main

SOURCES = $(wildcard $(SRCDIR)/*.cpp $(SRCDIR)/**/*.cpp $(SRCDIR)/**/**/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

dir_guard=@mkdir -p $(@D)

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(dir_guard)
	$(CC) $+ -o $@ $(LDFLAGS)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(dir_guard)
	$(CC) $(CFLAGS) -c $< -I./src -o $@

run: $(BINDIR)/$(TARGET)
	$(BINDIR)/$(TARGET) ${ARGS}

clean:
	rm -rvf $(BINDIR) $(OBJDIR)

