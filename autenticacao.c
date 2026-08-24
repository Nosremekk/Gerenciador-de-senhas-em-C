#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sodium.h>
#include "autenticacao.h"
#include "arquivo.h"
#include "util.h"

#define MAX_TENTATIVAS 3

unsigned char chave_mestra[crypto_secretbox_KEYBYTES];

static int carregar_salt(unsigned char salt[crypto_pwhash_SALTBYTES])
{
    char caminho[1024];
    if (!obter_caminho_dados(caminho, sizeof(caminho), "salt.bin"))
    {
        return 0;
    }

    FILE *f = fopen(caminho, "rb");
    if (f == NULL)
    {
        return 0;
    }

    size_t lidos = fread(salt, 1, crypto_pwhash_SALTBYTES, f);
    fclose(f);

    return (lidos == crypto_pwhash_SALTBYTES);
}

static int criar_e_salvar_salt(unsigned char salt[crypto_pwhash_SALTBYTES])
{
    char caminho[1024];
    if (!obter_caminho_dados(caminho, sizeof(caminho), "salt.bin"))
    {
        return 0;
    }

    randombytes_buf(salt, crypto_pwhash_SALTBYTES);

    int fd = open(caminho, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        return 0;
    }

    FILE *f = fdopen(fd, "wb");
    if (f == NULL)
    {
        close(fd);
        return 0;
    }

    size_t escritos = fwrite(salt, 1, crypto_pwhash_SALTBYTES, f);
    fclose(f);

    return (escritos == crypto_pwhash_SALTBYTES);
}

static int derivar_chave(const char *senha, const unsigned char salt[crypto_pwhash_SALTBYTES], unsigned char out_chave[crypto_secretbox_KEYBYTES])
{
    if (crypto_pwhash(out_chave,
                      crypto_secretbox_KEYBYTES,
                      senha,
                      strlen(senha),
                      salt,
                      crypto_pwhash_OPSLIMIT_INTERACTIVE,
                      crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_DEFAULT) != 0)
    {
        return 0;
    }

    return 1;
}

int autenticar(void)
{
    char caminho_senha[1024];
    if (!obter_caminho_dados(caminho_senha, sizeof(caminho_senha), "senha_mestra.txt"))
    {
        return 0;
    }

    FILE *arquivo;
    char hash_armazenada[crypto_pwhash_STRBYTES];
    char temp_senha[128];
    unsigned char salt[crypto_pwhash_SALTBYTES];

    arquivo = fopen(caminho_senha, "r");
    if (arquivo != NULL)
    {
        if (fgets(hash_armazenada, sizeof(hash_armazenada), arquivo) == NULL)
        {
            fclose(arquivo);
            printf("Erro ao ler arquivo de senha mestra.\n");
            return 0;
        }
        hash_armazenada[strcspn(hash_armazenada, "\r\n")] = '\0';
        fclose(arquivo);

        if (!carregar_salt(salt))
        {
            printf("Erro critico: 'senha_mestra.txt' existe, mas 'salt.bin' esta ausente ou corrompido.\n");
            return 0;
        }

        int tentativas = 0;
        unsigned int delay_segundos = 1;

        while (tentativas < MAX_TENTATIVAS)
        {
            ler_senha_oculta("Digite a senha mestra: ", temp_senha, sizeof(temp_senha));

            if (crypto_pwhash_str_verify(hash_armazenada, temp_senha, strlen(temp_senha)) == 0)
            {
                if (!derivar_chave(temp_senha, salt, chave_mestra))
                {
                    sodium_memzero(temp_senha, sizeof(temp_senha));
                    printf("Erro ao derivar chave de criptografia.\n");
                    return 0;
                }
                sodium_memzero(temp_senha, sizeof(temp_senha));
                printf("Senha mestra correta! Acesso liberado.\n");
                return 1;
            }

            sodium_memzero(temp_senha, sizeof(temp_senha));
            tentativas++;

            if (tentativas < MAX_TENTATIVAS)
            {
                printf("Senha incorreta. Tentativas restantes: %d\n", MAX_TENTATIVAS - tentativas);
                printf("Aguarde %u segundo(s)...\n", delay_segundos);
                sleep(delay_segundos);
                delay_segundos *= 2;
            }
        }

        printf("\nLimite de %d tentativas excedido. Acesso bloqueado por seguranca.\n", MAX_TENTATIVAS);
        return 0;
    }

    printf("Nenhuma configuracao existente encontrada. Iniciando primeiro acesso.\n");
    ler_senha_oculta("Crie uma senha mestra: ", temp_senha, sizeof(temp_senha));

    if (!criar_e_salvar_salt(salt))
    {
        sodium_memzero(temp_senha, sizeof(temp_senha));
        printf("Erro ao criar arquivo 'salt.bin'.\n");
        return 0;
    }

    if (crypto_pwhash_str(
            hash_armazenada,
            temp_senha,
            strlen(temp_senha),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
    {
        sodium_memzero(temp_senha, sizeof(temp_senha));
        printf("Erro ao gerar hash de seguranca da senha.\n");
        return 0;
    }

    if (!derivar_chave(temp_senha, salt, chave_mestra))
    {
        sodium_memzero(temp_senha, sizeof(temp_senha));
        printf("Erro ao derivar chave de criptografia.\n");
        return 0;
    }

    sodium_memzero(temp_senha, sizeof(temp_senha));

    int fd = open(caminho_senha, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        printf("Erro ao salvar 'senha_mestra.txt'.\n");
        return 0;
    }

    arquivo = fdopen(fd, "w");
    if (arquivo == NULL)
    {
        close(fd);
        printf("Erro ao salvar 'senha_mestra.txt'.\n");
        return 0;
    }

    fprintf(arquivo, "%s\n", hash_armazenada);
    fclose(arquivo);

    printf("Senha mestra configurada com sucesso! Acesso liberado.\n");
    return 1;
}

int alterar_senha_mestra(BancoDeDados *banco)
{
    char caminho_senha[1024];
    char caminho_salt[1024];
    if (!obter_caminho_dados(caminho_senha, sizeof(caminho_senha), "senha_mestra.txt") ||
        !obter_caminho_dados(caminho_salt, sizeof(caminho_salt), "salt.bin"))
    {
        return 0;
    }

    char senha_atual[128];
    char nova_senha[128];
    char confirma_senha[128];
    char hash_atual[crypto_pwhash_STRBYTES];
    char nova_hash[crypto_pwhash_STRBYTES];
    unsigned char novo_salt[crypto_pwhash_SALTBYTES];
    unsigned char nova_chave[crypto_secretbox_KEYBYTES];

    FILE *arquivo = fopen(caminho_senha, "r");
    if (arquivo == NULL)
    {
        printf("Erro ao abrir 'senha_mestra.txt'.\n");
        return 0;
    }

    if (fgets(hash_atual, sizeof(hash_atual), arquivo) == NULL)
    {
        fclose(arquivo);
        printf("Erro ao ler hash atual.\n");
        return 0;
    }
    hash_atual[strcspn(hash_atual, "\r\n")] = '\0';
    fclose(arquivo);

    ler_senha_oculta("Digite a senha mestra atual: ", senha_atual, sizeof(senha_atual));

    if (crypto_pwhash_str_verify(hash_atual, senha_atual, strlen(senha_atual)) != 0)
    {
        sodium_memzero(senha_atual, sizeof(senha_atual));
        printf("Senha mestra atual incorreta.\n");
        return 0;
    }
    sodium_memzero(senha_atual, sizeof(senha_atual));

    ler_senha_oculta("Digite a nova senha mestra: ", nova_senha, sizeof(nova_senha));
    ler_senha_oculta("Confirme a nova senha mestra: ", confirma_senha, sizeof(confirma_senha));

    if (strcmp(nova_senha, confirma_senha) != 0)
    {
        sodium_memzero(nova_senha, sizeof(nova_senha));
        sodium_memzero(confirma_senha, sizeof(confirma_senha));
        printf("As senhas digitadas nao coincidem.\n");
        return 0;
    }
    sodium_memzero(confirma_senha, sizeof(confirma_senha));

    randombytes_buf(novo_salt, sizeof(novo_salt));

    if (crypto_pwhash_str(
            nova_hash,
            nova_senha,
            strlen(nova_senha),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
    {
        sodium_memzero(nova_senha, sizeof(nova_senha));
        printf("Erro ao gerar novo hash de seguranca.\n");
        return 0;
    }

    if (!derivar_chave(nova_senha, novo_salt, nova_chave))
    {
        sodium_memzero(nova_senha, sizeof(nova_senha));
        printf("Erro ao derivar nova chave criptografica.\n");
        return 0;
    }
    sodium_memzero(nova_senha, sizeof(nova_senha));

    unsigned char chave_antiga[crypto_secretbox_KEYBYTES];
    memcpy(chave_antiga, chave_mestra, sizeof(chave_antiga));

    memcpy(chave_mestra, nova_chave, sizeof(chave_mestra));
    sodium_memzero(nova_chave, sizeof(nova_chave));

    if (!salvar_banco(banco))
    {
        memcpy(chave_mestra, chave_antiga, sizeof(chave_mestra));
        sodium_memzero(chave_antiga, sizeof(chave_antiga));
        printf("Erro ao regravar o banco com a nova chave. Operacao cancelada.\n");
        return 0;
    }
    sodium_memzero(chave_antiga, sizeof(chave_antiga));

    int fd_salt = open(caminho_salt, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd_salt != -1)
    {
        FILE *f_salt = fdopen(fd_salt, "wb");
        if (f_salt != NULL)
        {
            fwrite(novo_salt, 1, sizeof(novo_salt), f_salt);
            fclose(f_salt);
        }
        else
        {
            close(fd_salt);
        }
    }

    int fd_hash = open(caminho_senha, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd_hash != -1)
    {
        FILE *f_hash = fdopen(fd_hash, "w");
        if (f_hash != NULL)
        {
            fprintf(f_hash, "%s\n", nova_hash);
            fclose(f_hash);
        }
        else
        {
            close(fd_hash);
        }
    }

    printf("Senha mestra alterada e banco re-encriptado com sucesso!\n");
    return 1;
}