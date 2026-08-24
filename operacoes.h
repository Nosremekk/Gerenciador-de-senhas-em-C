#ifndef OPERACOES_H
#define OPERACOES_H

#include "registro.h"

int ler_opcao_menu(int *opcao_out);
void adicionar_registro(BancoDeDados *banco);
void listar_registros(BancoDeDados *banco);
void modificar_registro(BancoDeDados *banco);
void deletar_registro(BancoDeDados *banco);
void pesquisar_registro(BancoDeDados *banco);
void exportar_dados_menu(const BancoDeDados *banco);

#endif