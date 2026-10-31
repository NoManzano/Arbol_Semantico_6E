CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LDLIBS = -lsfml-graphics -lsfml-window -lsfml-system
TARGET = bin/arbol_semantico
SOURCES = src/main.cpp src/arbol_semantico.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)
