TARGET=./a.out
CC=g++
SRCS=\
	./main.cpp
STD=-std=c++17
LIBS=-lftd2xx

all: clean $(TARGET)

$(TARGET):
	$(CC) $(STD) $(LIBS) -o $(TARGET) $(SRCS)

build: $(TARGET)

clean:
	rm -rf $(TARGET)
