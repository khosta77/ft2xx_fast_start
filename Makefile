TARGET=./a.out
CC=g++
SRCS=\
	./main.cpp
STD=-std=c++17
LIBS=-lftd2xx -ljpeg
JPEGLIB=-I /opt/homebrew/Cellar/jpeg-turbo/*/include -L /opt/homebrew/Cellar/jpeg-turbo/*/lib

all: clean $(TARGET)

$(TARGET):
	$(CC) $(STD) $(JPEGLIB) $(LIBS) -o $(TARGET) $(SRCS)

build: $(TARGET)

clean:
	rm -rf $(TARGET)
