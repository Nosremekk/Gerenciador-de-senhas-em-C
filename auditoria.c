#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "auditoria.h"

AnaliseSenha auditar_senha_individual(const char *senha)
{
    AnaliseSenha a;
    memset(&a, 0, sizeof(AnaliseSenha));
    a.comprimento = (int)strlen(senha);

    for (int i = 0; i < a.comprimento; i++)
    {
        unsigned char c = (unsigned char)senha[i];
        if (isupper(c)) a.tem_maiuscula = 1;
        else if (islower(c)) a.tem_minuscula = 1;
        else if (isdigit(c)) a.tem_numero = 1;
        else a.tem_simbolo = 1;
    }

    int variedades = a.tem_maiuscula + a.tem_minuscula + a.tem_numero + a.tem_simbolo;

    if (a.comprimento >= 16 && variedades >= 3)
    {
        a.pontuacao = 4;
        a.classificacao = "MUITO FORTE";
    }
    else if (a.comprimento >= 12 && variedades >= 3)
    {
        a.pontuacao = 3;
        a.classificacao = "FORTE";
    }
    else if (a.comprimento >= 8 && variedades >= 2)
    {
        a.pontuacao = 2;
        a.classificacao = "MEDIA";
    }
    else
    {
        a.pontuacao = 1;
        a.classificacao = "FRACA";
    }

    return a;
}

void executar_auditoria_banco(const BancoDeDados *banco)
{
    int total_ativos = 0;
    int senhas_fracas = 0;
    int senhas_repetidas = 0;

    printf("\n=== RELATORIO DE AUDITORIA DE SEGURANCA ===\n\n");

    for (size_t i = 0; i < banco->tamanho; i++)
    {
        if (banco->itens[i].ativo == 0)
        {
            continue;
        }

        total_ativos++;
        AnaliseSenha a = auditar_senha_individual(banco->itens[i].senha);

        if (a.pontuacao <= 2)
        {
            senhas_fracas++;
            printf("[ALERTA - %s] ID: %d | Servico: %s | Usuario: %s (Tam: %d)\n",
                   a.classificacao, banco->itens[i].id, banco->itens[i].servico, banco->itens[i].user, a.comprimento);
            if (!a.tem_maiuscula) printf("  -> Falta letra maiuscula\n");
            if (!a.tem_minuscula) printf("  -> Falta letra minuscula\n");
            if (!a.tem_numero) printf("  -> Falta numero\n");
            if (!a.tem_simbolo) printf("  -> Falta caractere especial\n");
            if (a.comprimento < 8) printf("  -> Comprimento critico (< 8 caracteres)\n");
        }

        for (size_t j = i + 1; j < banco->tamanho; j++)
        {
            if (banco->itens[j].ativo == 1 && strcmp(banco->itens[i].senha, banco->itens[j].senha) == 0)
            {
                senhas_repetidas++;
                printf("[ALERTA - REUTILIZACAO] Senha identica entre:\n");
                printf("  -> ID %d (%s - %s)\n", banco->itens[i].id, banco->itens[i].servico, banco->itens[i].user);
                printf("  -> ID %d (%s - %s)\n", banco->itens[j].id, banco->itens[j].servico, banco->itens[j].user);
            }
        }
    }

    if (total_ativos == 0)
    {
        printf("Nenhum registro ativo para auditar.\n");
        return;
    }

    printf("\n--- Resumo ---\n");
    printf("Total de registros analisados: %d\n", total_ativos);
    printf("Senhas fracas/medias: %d\n", senhas_fracas);
    printf("Casos de senhas reutilizadas: %d\n", senhas_repetidas);

    if (senhas_fracas == 0 && senhas_repetidas == 0)
    {
        printf("Excelente! Todas as senhas cadastradas estao seguras e exclusivas.\n");
    }
}