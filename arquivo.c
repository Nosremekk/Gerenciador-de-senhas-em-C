#include <stdio.h>
#include <string.h>
#include <sodium.h>
#include "arquivo.h"
#include "autenticacao.h"

static int encriptar_senha(const char *plano, char *hex_out, size_t hex_max)
{
    size_t tam_plano = strlen(plano);
    size_t tam_cifrado = crypto_secretbox_MACBYTES + tam_plano;
    unsigned char cifrado[tam_cifrado];
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char pacote[crypto_secretbox_NONCEBYTES + sizeof(cifrado)];

    randombytes_buf(nonce, sizeof(nonce));
    if (crypto_secretbox_easy(cifrado, (const unsigned char *)plano, tam_plano, nonce, chave_mestra) != 0)
    {
        return 0;
    }

    memcpy(pacote, nonce, sizeof(nonce));
    memcpy(pacote + sizeof(nonce), cifrado, sizeof(cifrado));

    if (sodium_bin2hex(hex_out, hex_max, pacote, sizeof(pacote)) == NULL)
    {
        return 0;
    }

    return 1;
}

static int decriptar_senha(const char *hex_in, char *plano_out, size_t plano_max)
{
    size_t tam_hex = strlen(hex_in);
    if (tam_hex % 2 != 0)
    {
        return 0;
    }

    size_t tam_pacote = tam_hex / 2;
    if (tam_pacote <= crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES)
    {
        return 0;
    }

    unsigned char pacote[tam_pacote];
    size_t bin_len;
    if (sodium_hex2bin(pacote, sizeof(pacote), hex_in, tam_hex, NULL, &bin_len, NULL) != 0)
    {
        return 0;
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    memcpy(nonce, pacote, sizeof(nonce));

    size_t tam_cifrado = bin_len - sizeof(nonce);
    unsigned char *cifrado = pacote + sizeof(nonce);

    size_t tam_plano = tam_cifrado - crypto_secretbox_MACBYTES;
    if (tam_plano >= plano_max)
    {
        return 0;
    }

    unsigned char decrypted[tam_plano];
    if (crypto_secretbox_open_easy(decrypted, cifrado, tam_cifrado, nonce, chave_mestra) != 0)
    {
        return 0;
    }

    memcpy(plano_out, decrypted, tam_plano);
    plano_out[tam_plano] = '\0';
    sodium_memzero(decrypted, sizeof(decrypted));

    return 1;
}

int carregar_banco(BancoDeDados *banco)
{
    FILE *arquivo = fopen("senhas.txt", "r");
    if (arquivo == NULL)
    {
        return 0;
    }

    int id_lido;
    char servico[50];
    char user[50];
    char hex_senha[512];
    int carregados = 0;

    while (fscanf(arquivo, "%d|%49[^|]|%49[^|]|%511[^\n]\n",
                  &id_lido, servico, user, hex_senha) == 4)
    {
        size_t indice = (size_t)id_lido;
        if (!banco_garantir_capacidade(banco, indice + 1))
        {
            fclose(arquivo);
            return carregados;
        }

        if (decriptar_senha(hex_senha, banco->itens[indice].senha, sizeof(banco->itens[indice].senha)))
        {
            banco->itens[indice].id = id_lido;
            strncpy(banco->itens[indice].servico, servico, sizeof(banco->itens[indice].servico) - 1);
            banco->itens[indice].servico[sizeof(banco->itens[indice].servico) - 1] = '\0';
            strncpy(banco->itens[indice].user, user, sizeof(banco->itens[indice].user) - 1);
            banco->itens[indice].user[sizeof(banco->itens[indice].user) - 1] = '\0';
            banco->itens[indice].ativo = 1;

            if (indice + 1 > banco->tamanho)
            {
                banco->tamanho = indice + 1;
            }
            carregados++;
        }
    }

    fclose(arquivo);
    return carregados;
}

int salvar_banco(const BancoDeDados *banco)
{
    FILE *arquivo = fopen("senhas.txt", "w");
    if (arquivo == NULL)
    {
        return 0;
    }

    for (size_t i = 0; i < banco->tamanho; i++)
    {
        if (banco->itens[i].ativo == 0)
        {
            continue;
        }

        char hex_senha[512];
        if (!encriptar_senha(banco->itens[i].senha, hex_senha, sizeof(hex_senha)))
        {
            fclose(arquivo);
            return 0;
        }

        fprintf(
            arquivo,
            "%d|%s|%s|%s\n",
            banco->itens[i].id,
            banco->itens[i].servico,
            banco->itens[i].user,
            hex_senha
        );
    }

    fclose(arquivo);
    return 1;
}