#ifndef AUTENTICACAO_H
#define AUTENTICACAO_H

#include <sodium.h>

extern unsigned char chave_mestra[crypto_secretbox_KEYBYTES];

int autenticar(void);

#endif