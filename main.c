#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sodium.h>
#include "registro.h"
#include "arquivo.h"
#include "autenticacao.h"
#include "operacoes.h"
#include "auditoria.h"

static int ler_opcao(void)
{
    char entrada[100];
    while (1)
    {
        char *fim;
        long valor;
        printf("Escolha uma opcao: ");
        if (fgets(entrada, sizeof(entrada), stdin) == NULL)
        {
            printf("Erro ao ler entrada.\n");
            continue;
        }
        entrada[strcspn(entrada, "\n")] = '\0';
        if (entrada[0] == '\0')
        {
            printf("Digite uma opcao.\n");
            continue;
        }
        errno = 0;
        valor = strtol(entrada, &fim, 10);
        if (errno == ERANGE || valor < INT_MIN || valor > INT_MAX)
        {
            printf("Opcao invalida.\n");
            continue;
        }
        if (*fim != '\0')
        {
            printf("Digite apenas um numero.\n");
            continue;
        }
        return (int)valor;
    }
}

int main(void)
{
    if (sodium_init() < 0)
    {
        printf("Erro ao iniciar a biblioteca de seguranca.\n");
        return 1;
    }

    if (!autenticar())
    {
        return 1;
    }

    printf("Bem vindo ao gerenciador de senhas!\n");

    BancoDeDados banco;
    banco_inicializar(&banco);
    carregar_banco(&banco);

    while (1)
    {
        printf("\n");
        printf("1 - Adicionar senha\n");
        printf("2 - Listar senhas\n");
        printf("3 - Modificar registro\n");
        printf("4 - Deletar registro\n");
        printf("5 - Procurar por senha\n");
        printf("6 - Auditar seguranca do banco\n");
        printf("7 - Sair do servico\n");

        int op = ler_opcao();

        switch (op)
        {
            case 1:
                adicionar_registro(&banco);
                break;
            case 2:
                listar_registros(&banco);
                break;
            case 3:
                modificar_registro(&banco);
                break;
            case 4:
                deletar_registro(&banco);
                break;
            case 5:
                pesquisar_registro(&banco);
                break;
            case 6:
                executar_auditoria_banco(&banco);
                break;
            case 7:
                banco_liberar(&banco);
                sodium_memzero(chave_mestra, sizeof(chave_mestra));
                printf("Saindo da aplicacao.\n");
                return 0;
            default:
                printf("Comando invalido. Tente novamente.\n");
                break;
        }
    }
}