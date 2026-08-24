#include <stdio.h>
#include <stdlib.h>
#include <sodium.h>
#include "registro.h"
#include "arquivo.h"
#include "autenticacao.h"
#include "operacoes.h"
#include "auditoria.h"

static void bloquear_sessao(BancoDeDados *banco)
{
    printf("\n\nSessao bloqueada por inatividade. Memoria limpa.\n");

    sodium_memzero(chave_mestra, sizeof(chave_mestra));
    banco_liberar(banco);

    while (!autenticar())
    {
        printf("Falha na re-autenticacao. Tente novamente.\n");
    }

    banco_inicializar(banco);
    carregar_banco(banco);
    printf("\nSessao restaurada com sucesso!\n");
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
        printf("7 - Alterar senha mestra\n");
        printf("8 - Exportar / Backup do banco\n");
        printf("9 - Sair do servico\n");

        int op = 0;
        int status = ler_opcao_menu(&op);

        if (status == -1)
        {
            bloquear_sessao(&banco);
            continue;
        }

        if (status == 0)
        {
            printf("Opcao invalida. Tente novamente.\n");
            continue;
        }

        switch (op)
        {
            case 1: adicionar_registro(&banco); break;
            case 2: listar_registros(&banco); break;
            case 3: modificar_registro(&banco); break;
            case 4: deletar_registro(&banco); break;
            case 5: pesquisar_registro(&banco); break;
            case 6: executar_auditoria_banco(&banco); break;
            case 7: alterar_senha_mestra(&banco); break;
            case 8: exportar_dados_menu(&banco); break;
            case 9:
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