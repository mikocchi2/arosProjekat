all: chat

chat: chat.c
	gcc -o chat chat.c -lrt -lpthread