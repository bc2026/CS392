CC            = gcc
C_FILE        = client.c
TARGET        = client
SERVER_FILE   = server.c
SERVER_TARGET = server
CFLAGS        = -Ofast -Wall -Werror -pedantic-errors

.PHONY: all clean

all: $(TARGET) $(SERVER_TARGET)

$(TARGET): $(C_FILE)
	$(CC) $(CFLAGS) $(C_FILE) -o $(TARGET)

$(SERVER_TARGET): $(SERVER_FILE)
	$(CC) $(CFLAGS) $(SERVER_FILE) -o $(SERVER_TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe $(SERVER_TARGET) $(SERVER_TARGET).exe
