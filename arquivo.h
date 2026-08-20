#ifndef ARQUIVO_H
#define ARQUIVO_H

#include "registro.h"

int carregar_banco(BancoDeDados *banco);
int salvar_banco(const BancoDeDados *banco);

#endif