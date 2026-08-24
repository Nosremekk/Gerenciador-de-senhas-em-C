#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sodium.h>
#include "operacoes.h"
#include "arquivo.h"
#include "util.h"
#include "auditoria.h"

int ler_opcao_menu(int *opcao_out)
{
    char buffer[32];
    printf("Escolha uma opcao: ");
    fflush(stdout);

    int status = ler_string_timeout(buffer, sizeof(buffer), TIMEOUT_INATIVIDADE_SEGUNDOS);
    if (status == -1)
    {
        return -1;
    }
    if (status == 0 || buffer[0] == '\0')
    {
        return 0;
    }

    char *fim;
    long valor = strtol(buffer, &fim, 10);
    if (*fim != '\0')
    {
        return 0;
    }

    *opcao_out = (int)valor;
    return 1;
}

static void ler_string(const char *mensagem, char *destino, size_t tamanho)
{
    while (1)
    {
        printf("%s", mensagem);
        if (fgets(destino, tamanho, stdin) == NULL)
        {
            printf("Erro ao ler entrada.\n");
            continue;
        }
        if (strchr(destino, '\n') == NULL)
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
            {
            }
            printf("Entrada muito longa. Tente novamente.\n");
            continue;
        }
        destino[strcspn(destino, "\n")] = '\0';
        if (destino[0] == '\0')
        {
            printf("A entrada nao pode estar vazia.\n");
            continue;
        }
        return;
    }
}

static int ler_inteiro(const char *mensagem)
{
    char entrada[100];
    while (1)
    {
        char *fim;
        long valor;
        printf("%s", mensagem);
        if (fgets(entrada, sizeof(entrada), stdin) == NULL)
        {
            printf("Erro ao ler entrada.\n");
            continue;
        }
        entrada[strcspn(entrada, "\n")] = '\0';
        if (entrada[0] == '\0')
        {
            printf("Digite um numero.\n");
            continue;
        }
        errno = 0;
        valor = strtol(entrada, &fim, 10);
        if (errno == ERANGE || valor < INT_MIN || valor > INT_MAX)
        {
            printf("Numero invalido.\n");
            continue;
        }
        if (*fim != '\0')
        {
            printf("Digite apenas um numero inteiro.\n");
            continue;
        }
        return (int)valor;
    }
}

static void definir_ou_gerar_senha(char *destino, size_t tamanho)
{
    printf("Como deseja definir a senha?\n");
    printf("1 - Digitar manualmente\n");
    printf("2 - Gerar senha forte aleatoria\n");

    int op;
    while (1)
    {
        op = ler_inteiro("Opcao: ");
        if (op == 1 || op == 2)
        {
            break;
        }
        printf("Opcao invalida. Tente novamente.\n");
    }

    if (op == 1)
    {
        ler_senha_oculta("Senha: ", destino, tamanho);
        AnaliseSenha a = auditar_senha_individual(destino);
        printf("Classificacao da senha digitada: [%s]\n", a.classificacao);
    }
    else
    {
        int tam;
        while (1)
        {
            tam = ler_inteiro("Tamanho da senha (entre 8 e 64): ");
            if (tam >= 8 && tam <= 64 && (size_t)tam < tamanho)
            {
                break;
            }
            printf("Tamanho invalido. Escolha entre 8 e 64.\n");
        }
        gerar_senha_aleatoria(destino, (size_t)tam);
        printf("Senha gerada com sucesso: %s\n", destino);
    }
}

static void mostrar_registro(Registro *registro)
{
    printf("\n--- Registro Encontrado ---\n");
    printf("Servico: %s\n", registro->servico);
    printf("Usuario: %s\n", registro->user);
    printf("ID: %d\n", registro->id);
    
    AnaliseSenha a = auditar_senha_individual(registro->senha);
    printf("Forca da senha: [%s]\n", a.classificacao);

    printf("1 - Exibir senha na tela\n");
    printf("2 - Copiar senha para area de transferencia\n");
    printf("3 - Voltar\n");

    int opcao = ler_inteiro("Opcao: ");
    if (opcao == 1)
    {
        printf("Senha: %s\n", registro->senha);
    }
    else if (opcao == 2)
    {
        if (copiar_para_clipboard(registro->senha))
        {
            printf("Senha copiada! A area de transferencia sera limpa automaticamente em 30 segundos.\n");
        }
        else
        {
            printf("Falha ao copiar. Verifique se o wl-clipboard ou xclip esta instalado.\n");
        }
    }
}

void adicionar_registro(BancoDeDados *banco)
{
    int id_livre = retorna_id(banco);
    if (!banco_garantir_capacidade(banco, (size_t)id_livre + 1))
    {
        printf("Erro ao alocar memoria para novo registro.\n");
        return;
    }

    Registro *novo = &banco->itens[id_livre];
    novo->id = id_livre;

    printf("Processo de insercao de nova senha.\n");
    ler_string("Servico: ", novo->servico, sizeof(novo->servico));
    ler_string("User: ", novo->user, sizeof(novo->user));
    definir_ou_gerar_senha(novo->senha, sizeof(novo->senha));
    novo->ativo = 1;

    if ((size_t)id_livre >= banco->tamanho)
    {
        banco->tamanho = (size_t)id_livre + 1;
    }

    if (!salvar_banco(banco))
    {
        novo->ativo = 0;
        printf("Erro ao salvar banco de dados.\n");
        return;
    }
    printf("Registro salvo com sucesso.\n");
}

void listar_registros(BancoDeDados *banco)
{
    int encontrados = 0;
    printf("\nSuas credenciais cadastradas:\n");
    for (size_t i = 0; i < banco->tamanho; i++)
    {
        if (banco->itens[i].ativo == 0)
        {
            continue;
        }
        printf("[%d] Servico: %-20s | Usuario: %s\n", banco->itens[i].id, banco->itens[i].servico, banco->itens[i].user);
        encontrados++;
    }
    if (encontrados == 0)
    {
        printf("Nenhum registro cadastrado.\n");
        return;
    }

    printf("\nDeseja interagir com algum registro?\n");
    printf("1 - Selecionar por ID\n");
    printf("2 - Voltar ao menu principal\n");
    int op = ler_inteiro("Opcao: ");
    if (op == 1)
    {
        int id_escolhido = ler_inteiro("Digite o ID: ");
        Registro *r = obter_registro_por_id(banco, id_escolhido);
        if (r != NULL)
        {
            mostrar_registro(r);
        }
        else
        {
            printf("ID nao encontrado.\n");
        }
    }
}

void modificar_registro(BancoDeDados *banco)
{
    int id_modificado = ler_inteiro("Digite o ID do registro que deseja modificar: ");
    Registro *r = obter_registro_por_id(banco, id_modificado);
    if (r == NULL)
    {
        printf("ID nao encontrado.\n");
        return;
    }

    ler_string("Novo Servico: ", r->servico, sizeof(r->servico));
    ler_string("Novo User: ", r->user, sizeof(r->user));
    definir_ou_gerar_senha(r->senha, sizeof(r->senha));

    if (!salvar_banco(banco))
    {
        printf("Erro ao salvar alteracoes.\n");
        return;
    }
    printf("Registro modificado com sucesso.\n");
}

void deletar_registro(BancoDeDados *banco)
{
    int id_escolhido = ler_inteiro("Insira o ID do registro que deseja deletar: ");
    Registro *r = obter_registro_por_id(banco, id_escolhido);
    if (r == NULL)
    {
        printf("ID nao encontrado.\n");
        return;
    }

    r->ativo = 0;
    if (!salvar_banco(banco))
    {
        r->ativo = 1;
        printf("Erro ao salvar alteracoes.\n");
        return;
    }
    printf("Registro removido.\n");
}

static void pesquisar_por_id(BancoDeDados *banco)
{
    int id_busca = ler_inteiro("Digite o ID que deseja procurar: ");
    Registro *r = obter_registro_por_id(banco, id_busca);
    if (r != NULL)
    {
        mostrar_registro(r);
    }
    else
    {
        printf("ID nao encontrado.\n");
    }
}

static void pesquisar_por_servico(BancoDeDados *banco)
{
    char servico_busca[50];
    int encontrados = 0;
    ler_string("Digite o termo de busca para o servico: ", servico_busca, sizeof(servico_busca));

    for (size_t i = 0; i < banco->tamanho; i++)
    {
        if (banco->itens[i].ativo == 1 && strcasestr(banco->itens[i].servico, servico_busca) != NULL)
        {
            mostrar_registro(&banco->itens[i]);
            encontrados++;
        }
    }
    if (encontrados == 0)
    {
        printf("Nenhum registro encontrado correspondente ao termo informado.\n");
    }
}

static void pesquisar_por_usuario(BancoDeDados *banco)
{
    char user_busca[50];
    int encontrados = 0;
    ler_string("Digite o termo de busca para o usuario: ", user_busca, sizeof(user_busca));

    for (size_t i = 0; i < banco->tamanho; i++)
    {
        if (banco->itens[i].ativo == 1 && strcasestr(banco->itens[i].user, user_busca) != NULL)
        {
            mostrar_registro(&banco->itens[i]);
            encontrados++;
        }
    }
    if (encontrados == 0)
    {
        printf("Nenhum registro encontrado correspondente ao termo informado.\n");
    }
}

void pesquisar_registro(BancoDeDados *banco)
{
    while (1)
    {
        printf("\nDe qual maneira voce deseja realizar a busca?\n");
        printf("1 - Por ID\n");
        printf("2 - Por servico\n");
        printf("3 - Por usuario\n");
        printf("4 - Sair da busca\n");
        int busca_escolhida = ler_inteiro("Escolha: ");
        switch (busca_escolhida)
        {
            case 1:
                pesquisar_por_id(banco);
                break;
            case 2:
                pesquisar_por_servico(banco);
                break;
            case 3:
                pesquisar_por_usuario(banco);
                break;
            case 4:
                return;
            default:
                printf("Opcao inexistente. Tente novamente.\n");
                break;
        }
    }
}

void exportar_dados_menu(const BancoDeDados *banco)
{
    printf("\n=== EXPORTACAO E BACKUP ===\n");
    printf("1 - Exportar Backup Criptografado (Recomendado para seguranca)\n");
    printf("2 - Exportar em Texto Puro / CSV (Exige senha mestra)\n");
    printf("3 - Cancelar\n");

    int op = ler_inteiro("Opcao: ");

    if (op == 1)
    {
        char destino[256];
        ler_string("Digite o caminho da pasta de destino (ex: . ou /home/usuario/backup): ", destino, sizeof(destino));

        if (exportar_backup_criptografado(destino))
        {
            printf("Backup dos arquivos criptografados gerado com sucesso em '%s'!\n", destino);
        }
        else
        {
            printf("Erro ao gerar backup. Verifique se o caminho existe e tem permissao de escrita.\n");
        }
    }
    else if (op == 2)
    {
        char confirmacao_senha[128];
        char hash_armazenada[crypto_pwhash_STRBYTES];
        char caminho_hash[1024];

        if (!obter_caminho_dados(caminho_hash, sizeof(caminho_hash), "senha_mestra.txt"))
        {
            printf("Erro ao validar credencial.\n");
            return;
        }

        FILE *f = fopen(caminho_hash, "r");
        if (f == NULL || fgets(hash_armazenada, sizeof(hash_armazenada), f) == NULL)
        {
            if (f) fclose(f);
            printf("Erro ao carregar credencial de validacao.\n");
            return;
        }
        hash_armazenada[strcspn(hash_armazenada, "\r\n")] = '\0';
        fclose(f);

        printf("\n[AVISO DE SEGURANCA] O arquivo gerado contera todas as senhas legiveis em texto puro.\n");
        ler_senha_oculta("Confirme sua senha mestra para continuar: ", confirmacao_senha, sizeof(confirmacao_senha));

        if (crypto_pwhash_str_verify(hash_armazenada, confirmacao_senha, strlen(confirmacao_senha)) != 0)
        {
            sodium_memzero(confirmacao_senha, sizeof(confirmacao_senha));
            printf("Senha mestra incorreta. Exportacao cancelada.\n");
            return;
        }
        sodium_memzero(confirmacao_senha, sizeof(confirmacao_senha));

        char nome_arquivo[256];
        ler_string("Nome do arquivo de destino (ex: senhas.csv): ", nome_arquivo, sizeof(nome_arquivo));

        if (exportar_banco_csv(banco, nome_arquivo))
        {
            printf("Banco exportado com sucesso para '%s'!\n", nome_arquivo);
        }
        else
        {
            printf("Erro ao salvar arquivo exportado.\n");
        }
    }
}