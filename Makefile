CC = gcc
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags libsodium)
LIBS = $(shell pkg-config --libs libsodium)
PREFIX = $(HOME)/.local/bin
TARGET = gerenciador_senhas

all: $(TARGET)

$(TARGET): main.o registro.o arquivo.o autenticacao.o operacoes.o util.o auditoria.o hardware.o
	$(CC) main.o registro.o arquivo.o autenticacao.o operacoes.o util.o auditoria.o hardware.o -o $(TARGET) $(LIBS)

main.o: main.c registro.h arquivo.h autenticacao.h operacoes.h auditoria.h
	$(CC) $(CFLAGS) -c main.c

registro.o: registro.c registro.h
	$(CC) $(CFLAGS) -c registro.c

arquivo.o: arquivo.c arquivo.h registro.h autenticacao.h
	$(CC) $(CFLAGS) -c arquivo.c

autenticacao.o: autenticacao.c autenticacao.h util.h arquivo.h hardware.h
	$(CC) $(CFLAGS) -c autenticacao.c

hardware.o: hardware.c hardware.h
	$(CC) $(CFLAGS) -c hardware.c

operacoes.o: operacoes.c operacoes.h registro.h arquivo.h util.h auditoria.h
	$(CC) $(CFLAGS) -c operacoes.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

auditoria.o: auditoria.c auditoria.h registro.h
	$(CC) $(CFLAGS) -c auditoria.c

install: $(TARGET)
	mkdir -p $(PREFIX)
	install -m 755 $(TARGET) $(PREFIX)/$(TARGET)

uninstall:
	rm -f $(PREFIX)/$(TARGET)

clean:
	rm -f *.o $(TARGET)