#ifndef REGISTRO_H
#define REGISTRO_H

#include <stddef.h>

typedef struct
{
    char servico[50];
    char user[50];
    char senha[512];
    int id;
    int ativo;
} Registro;

typedef struct
{
    Registro *itens;
    size_t tamanho;
    size_t capacidade;
} BancoDeDados;

void banco_inicializar(BancoDeDados *banco);
void banco_liberar(BancoDeDados *banco);
int banco_garantir_capacidade(BancoDeDados *banco, size_t capacidade_minima);
int retorna_id(const BancoDeDados *banco);
int checa_id(int id, const BancoDeDados *banco);
Registro *obter_registro_por_id(BancoDeDados *banco, int id);

#endif