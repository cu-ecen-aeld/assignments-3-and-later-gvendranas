# Compipile it all
all: aesdsocket
# Creating clean command
.PHONY: clean
clean:
	rm -rf aesdsocket *.o
# Creating application
aesdsocket: aesdsocket.c
	$(CC) -Wall -Werror -g aesdsocket.c -o aesdsocket 