CC = g++
CFLAGS = -g -c
TARGET = compfs
OBJS = main.o scanner.o parser.o token.o codeGen.o
$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)
main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp
scanner.o: scanner.cpp
	$(CC) $(CFLAGS) scanner.cpp
parser.o: parser.cpp
	$(CC) $(CFLAGS) parser.cpp
token.o: token.cpp
	$(CC) $(CFLAGS) token.cpp
codeGen.o: codeGen.cpp
	$(CC) $(CFLAGS) codeGen.cpp
clean:
	/bin/rm -f *.o *.asm $(TARGET)
