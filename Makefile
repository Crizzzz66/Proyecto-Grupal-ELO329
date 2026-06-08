CXX = g++
CXXFLAGS = -Wall -O2 -std=c++17

TARGET = programa
SRC = main.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)