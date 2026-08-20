#include <stdio.h>
#include <stdlib.h>
#include <sodium.h>
#include "registro.h"

void banco_inicializar(BancoDeDados *banco)
{
    banco->itens = NULL;
    banco->tamanho = 0;
    banco->capacidade = 0;
}

void banco_liberar(BancoDeDados *banco)
{
    if (banco->itens != NULL)
    {
        sodium_memzero(banco->itens, sizeof(Registro) * banco->capacidade);
        free(banco->itens);
        banco->itens = NULL;
    }
    banco->tamanho = 0;
    banco->capacidade = 0;
}

int banco_garantir_capacidade(BancoDeDados *banco, size_t capacidade_minima)
{
    if (banco->capacidade >= capacidade_minima)
    {
        return 1;
    }

    size_t nova_capacidade = banco->capacidade == 0 ? 8 : banco->capacidade * 2;
    while (nova_capacidade < capacidade_minima)
    {
        nova_capacidade *= 2;
    }

    Registro *novos_itens = realloc(banco->itens, sizeof(Registro) * nova_capacidade);
    if (novos_itens == NULL)
    {
        return 0;
    }

    for (size_t i = banco->capacidade; i < nova_capacidade; i++)
    {
        novos_itens[i].ativo = 0;
        novos_itens[i].id = (int)i;
        novos_itens[i].servico[0] = '\0';
        novos_itens[i].user[0] = '\0';
        novos_itens[i].senha[0] = '\0';
    }

    banco->itens = novos_itens;
    banco->capacidade = nova_capacidade;
    return 1;
}

int retorna_id(const BancoDeDados *banco)
{
    for (size_t i = 0; i < banco->tamanho; i++)
    {
        if (banco->itens[i].ativo == 0)
        {
            return (int)i;
        }
    }
    return (int)banco->tamanho;
}

int checa_id(int id, const BancoDeDados *banco)
{
    if (id < 0 || (size_t)id >= banco->tamanho)
    {
        return 0;
    }
    if (banco->itens[id].ativo == 0)
    {
        return 0;
    }
    return 1;
}

Registro *obter_registro_por_id(BancoDeDados *banco, int id)
{
    if (!checa_id(id, banco))
    {
        return NULL;
    }
    return &banco->itens[id];
}