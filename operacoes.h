#ifndef OPERACOES_H
#define OPERACOES_H

#include "registro.h"

void adicionar_registro(BancoDeDados *banco);
void listar_registros(BancoDeDados *banco);
void modificar_registro(BancoDeDados *banco);
void deletar_registro(BancoDeDados *banco);
void pesquisar_registro(BancoDeDados *banco);

#endif