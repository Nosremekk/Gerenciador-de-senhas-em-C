#include <stdio.h>
#include <string.h>
#include <sodium.h>
#include "autenticacao.h"
#include "util.h"

unsigned char chave_mestra[crypto_secretbox_KEYBYTES];

static int obter_ou_criar_salt(unsigned char salt[crypto_pwhash_SALTBYTES])
{
    FILE *f = fopen("salt.bin", "rb");
    if (f != NULL)
    {
        size_t lidos = fread(salt, 1, crypto_pwhash_SALTBYTES, f);
        fclose(f);
        if (lidos == crypto_pwhash_SALTBYTES)
        {
            return 1;
        }
    }

    randombytes_buf(salt, crypto_pwhash_SALTBYTES);
    f = fopen("salt.bin", "wb");
    if (f == NULL)
    {
        return 0;
    }
    size_t escritos = fwrite(salt, 1, crypto_pwhash_SALTBYTES, f);
    fclose(f);
    return (escritos == crypto_pwhash_SALTBYTES);
}

static int derivar_chave(const char *senha, unsigned char out_chave[crypto_secretbox_KEYBYTES])
{
    unsigned char salt[crypto_pwhash_SALTBYTES];
    if (!obter_ou_criar_salt(salt))
    {
        return 0;
    }

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
    FILE *arquivo;
    char hash_armazenada[crypto_pwhash_STRBYTES];
    char temp_senha[128];

    arquivo = fopen("senha_mestra.txt", "r");
    if (arquivo != NULL)
    {
        if (fgets(hash_armazenada, sizeof(hash_armazenada), arquivo) == NULL)
        {
            fclose(arquivo);
            printf("Erro ao ler a senha mestra.\n");
            return 0;
        }
        hash_armazenada[strcspn(hash_armazenada, "\r\n")] = '\0';
        fclose(arquivo);

        ler_senha_oculta("Digite a senha mestra: ", temp_senha, sizeof(temp_senha));

        if (crypto_pwhash_str_verify(hash_armazenada, temp_senha, strlen(temp_senha)) == 0)
        {
            if (!derivar_chave(temp_senha, chave_mestra))
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
        printf("Senha mestra incorreta. Fechando sistema...\n");
        return 0;
    }

    printf("Nenhuma senha mestra encontrada.\n");
    ler_senha_oculta("Crie uma senha mestra: ", temp_senha, sizeof(temp_senha));

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

    if (!derivar_chave(temp_senha, chave_mestra))
    {
        sodium_memzero(temp_senha, sizeof(temp_senha));
        printf("Erro ao derivar chave de criptografia.\n");
        return 0;
    }

    sodium_memzero(temp_senha, sizeof(temp_senha));

    arquivo = fopen("senha_mestra.txt", "w");
    if (arquivo == NULL)
    {
        printf("Erro ao criar arquivo da senha mestra.\n");
        return 0;
    }

    fprintf(arquivo, "%s\n", hash_armazenada);
    fclose(arquivo);

    printf("Senha mestra criada! Acesso liberado.\n");
    return 1;
}