CC = gcc
CFLAGS = -Wall

TARGETS = cidadao servidor

all : $(TARGETS)

clean :
	rm -f $(TARGETS)

cidadao : cidadao.c common.h
	$(CC) $(CFLAGS) cidadao.c -o cidadao

servidor : servidor.c common.h
	$(CC) $(CFLAGS) servidor.c -o servidor
