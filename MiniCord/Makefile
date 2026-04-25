CC = gcc
CFLAGS = -O2 -Wall -Wextra

TARGET = minicord_server
SRC = main.c server.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

clean:
	rm -f $(TARGET) $(OBJ)