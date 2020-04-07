CC=g++
CFLAGS=-Wall
EXEC=apparmin

all: cleanAll $(EXEC)

apparmin: apparmin.o
	$(CC) -o $@ $^

apparmin.o: apparmin.cpp
	$(CC) -o apparmin.o -c apparmin.cpp $(CFLAGS)

clean:
	rm -rf *.o

cleanAll: clean
	rm -rf $(EXEC)

