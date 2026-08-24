#ifndef AUTENTICACAO_H
#define AUTENTICACAO_H

#include <sodium.h>
#include "registro.h"

extern unsigned char chave_mestra[crypto_secretbox_KEYBYTES];

int autenticar(void);
int alterar_senha_mestra(BancoDeDados *banco);

#endif