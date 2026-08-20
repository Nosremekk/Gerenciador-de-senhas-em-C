CC = gcc
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags libsodium)
LIBS = $(shell pkg-config --libs libsodium)

gerenciador: main.o registro.o arquivo.o autenticacao.o operacoes.o util.o auditoria.o
	$(CC) main.o registro.o arquivo.o autenticacao.o operacoes.o util.o auditoria.o -o gerenciador $(LIBS)

main.o: main.c registro.h arquivo.h autenticacao.h operacoes.h auditoria.h
	$(CC) $(CFLAGS) -c main.c

registro.o: registro.c registro.h
	$(CC) $(CFLAGS) -c registro.c

arquivo.o: arquivo.c arquivo.h registro.h autenticacao.h
	$(CC) $(CFLAGS) -c arquivo.c

autenticacao.o: autenticacao.c autenticacao.h util.h
	$(CC) $(CFLAGS) -c autenticacao.c

operacoes.o: operacoes.c operacoes.h registro.h arquivo.h util.h auditoria.h
	$(CC) $(CFLAGS) -c operacoes.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

auditoria.o: auditoria.c auditoria.h registro.h
	$(CC) $(CFLAGS) -c auditoria.c

clean:
	rm -f *.o gerenciador