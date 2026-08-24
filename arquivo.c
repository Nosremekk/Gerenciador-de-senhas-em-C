#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sodium.h>
#include "arquivo.h"
#include "autenticacao.h"

#define PASTA_CONFIG ".gerenciador_senhas"
#define NOME_BANCO "senhas.dat"
#define NOME_BANCO_TMP "senhas.tmp"

int obter_caminho_dados(char *destino, size_t tamanho, const char *nome_arquivo)
{
    const char *home = getenv("HOME");
    if (home == NULL)
    {
        return 0;
    }

    char pasta[1024];
    snprintf(pasta, sizeof(pasta), "%s/%s", home, PASTA_CONFIG);

    mkdir(pasta, 0700);

    snprintf(destino, tamanho, "%s/%s", pasta, nome_arquivo);
    return 1;
}

static int serializar_banco(const BancoDeDados *banco, unsigned char **buffer_out, size_t *tamanho_out)
{
    uint32_t total_ativos = 0;
    for (size_t i = 0; i < banco->tamanho; i++)
    {
        if (banco->itens[i].ativo == 1)
        {
            total_ativos++;
        }
    }

    size_t tamanho_total = sizeof(uint32_t) + (total_ativos * sizeof(Registro));
    unsigned char *buffer = malloc(tamanho_total);
    if (buffer == NULL)
    {
        return 0;
    }

    memcpy(buffer, &total_ativos, sizeof(uint32_t));
    unsigned char *cursor = buffer + sizeof(uint32_t);

    for (size_t i = 0; i < banco->tamanho; i++)
    {
        if (banco->itens[i].ativo == 1)
        {
            memcpy(cursor, &banco->itens[i], sizeof(Registro));
            cursor += sizeof(Registro);
        }
    }

    *buffer_out = buffer;
    *tamanho_out = tamanho_total;
    return 1;
}

static int deserializar_banco(BancoDeDados *banco, const unsigned char *buffer, size_t tamanho_buffer)
{
    if (tamanho_buffer < sizeof(uint32_t))
    {
        return 0;
    }

    uint32_t total_registros;
    memcpy(&total_registros, buffer, sizeof(uint32_t));

    size_t tamanho_esperado = sizeof(uint32_t) + (total_registros * sizeof(Registro));
    if (tamanho_buffer != tamanho_esperado)
    {
        return 0;
    }

    const unsigned char *cursor = buffer + sizeof(uint32_t);
    int carregados = 0;

    for (uint32_t i = 0; i < total_registros; i++)
    {
        Registro reg;
        memcpy(&reg, cursor, sizeof(Registro));
        cursor += sizeof(Registro);

        if (reg.id < 0)
        {
            continue;
        }

        size_t indice = (size_t)reg.id;
        if (!banco_garantir_capacidade(banco, indice + 1))
        {
            return carregados;
        }

        banco->itens[indice] = reg;
        banco->itens[indice].ativo = 1;

        if (indice + 1 > banco->tamanho)
        {
            banco->tamanho = indice + 1;
        }
        carregados++;
    }

    return carregados;
}

int carregar_banco(BancoDeDados *banco)
{
    char caminho_banco[1024];
    if (!obter_caminho_dados(caminho_banco, sizeof(caminho_banco), NOME_BANCO))
    {
        return 0;
    }

    FILE *arquivo = fopen(caminho_banco, "rb");
    if (arquivo == NULL)
    {
        return 0;
    }

    if (fseek(arquivo, 0, SEEK_END) != 0)
    {
        fclose(arquivo);
        return 0;
    }

    long tamanho_arquivo = ftell(arquivo);
    if (tamanho_arquivo <= (long)(crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES))
    {
        fclose(arquivo);
        return 0;
    }
    rewind(arquivo);

    unsigned char *pacote = malloc((size_t)tamanho_arquivo);
    if (pacote == NULL)
    {
        fclose(arquivo);
        return 0;
    }

    size_t lidos = fread(pacote, 1, (size_t)tamanho_arquivo, arquivo);
    fclose(arquivo);

    if (lidos != (size_t)tamanho_arquivo)
    {
        free(pacote);
        return 0;
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    memcpy(nonce, pacote, crypto_secretbox_NONCEBYTES);

    size_t tamanho_cifrado = (size_t)tamanho_arquivo - crypto_secretbox_NONCEBYTES;
    unsigned char *cifrado = pacote + crypto_secretbox_NONCEBYTES;
    size_t tamanho_plano = tamanho_cifrado - crypto_secretbox_MACBYTES;

    unsigned char *plano = malloc(tamanho_plano);
    if (plano == NULL)
    {
        free(pacote);
        return 0;
    }

    if (crypto_secretbox_open_easy(plano, cifrado, tamanho_cifrado, nonce, chave_mestra) != 0)
    {
        free(pacote);
        free(plano);
        return 0;
    }

    int resultado = deserializar_banco(banco, plano, tamanho_plano);

    sodium_memzero(plano, tamanho_plano);
    free(plano);
    free(pacote);

    return resultado;
}

int salvar_banco(const BancoDeDados *banco)
{
    char caminho_tmp[1024];
    char caminho_final[1024];

    if (!obter_caminho_dados(caminho_tmp, sizeof(caminho_tmp), NOME_BANCO_TMP) ||
        !obter_caminho_dados(caminho_final, sizeof(caminho_final), NOME_BANCO))
    {
        return 0;
    }

    unsigned char *plano = NULL;
    size_t tamanho_plano = 0;

    if (!serializar_banco(banco, &plano, &tamanho_plano))
    {
        return 0;
    }

    size_t tamanho_cifrado = crypto_secretbox_MACBYTES + tamanho_plano;
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    unsigned char *cifrado = malloc(tamanho_cifrado);
    if (cifrado == NULL)
    {
        sodium_memzero(plano, tamanho_plano);
        free(plano);
        return 0;
    }

    if (crypto_secretbox_easy(cifrado, plano, tamanho_plano, nonce, chave_mestra) != 0)
    {
        sodium_memzero(plano, tamanho_plano);
        free(plano);
        free(cifrado);
        return 0;
    }

    sodium_memzero(plano, tamanho_plano);
    free(plano);

    int fd = open(caminho_tmp, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        free(cifrado);
        return 0;
    }

    FILE *arquivo = fdopen(fd, "wb");
    if (arquivo == NULL)
    {
        close(fd);
        free(cifrado);
        return 0;
    }

    if (fwrite(nonce, 1, sizeof(nonce), arquivo) != sizeof(nonce) ||
        fwrite(cifrado, 1, tamanho_cifrado, arquivo) != tamanho_cifrado)
    {
        fclose(arquivo);
        remove(caminho_tmp);
        free(cifrado);
        return 0;
    }

    free(cifrado);

    if (fflush(arquivo) != 0)
    {
        fclose(arquivo);
        remove(caminho_tmp);
        return 0;
    }

    if (fclose(arquivo) != 0)
    {
        remove(caminho_tmp);
        return 0;
    }

    if (rename(caminho_tmp, caminho_final) != 0)
    {
        remove(caminho_tmp);
        return 0;
    }

    return 1;
}

int exportar_banco_csv(const BancoDeDados *banco, const char *caminho_saida)
{
    int fd = open(caminho_saida, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        return 0;
    }

    FILE *f = fdopen(fd, "w");
    if (f == NULL)
    {
        close(fd);
        return 0;
    }

    //Exportar em csv para ser compativel
    fprintf(f, "id,servico,usuario,senha\n");

    for (size_t i = 0; i < banco->tamanho; i++)
    {
        if (banco->itens[i].ativo == 1)
        {
            fprintf(f, "%d,\"%s\",\"%s\",\"%s\"\n",
                    banco->itens[i].id,
                    banco->itens[i].servico,
                    banco->itens[i].user,
                    banco->itens[i].senha);
        }
    }

    fclose(f);
    return 1;
}

static int copiar_arquivo_binario(const char *origem, const char *destino)
{
    FILE *src = fopen(origem, "rb");
    if (src == NULL) return 0;

    int fd_dst = open(destino, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd_dst == -1)
    {
        fclose(src);
        return 0;
    }

    FILE *dst = fdopen(fd_dst, "wb");
    if (dst == NULL)
    {
        close(fd_dst);
        fclose(src);
        return 0;
    }

    unsigned char buffer[4096];
    size_t lidos;
    while ((lidos = fread(buffer, 1, sizeof(buffer), src)) > 0)
    {
        fwrite(buffer, 1, lidos, dst);
    }

    fclose(src);
    fclose(dst);
    return 1;
}

int exportar_backup_criptografado(const char *pasta_destino)
{
    char origem[1024];
    char destino[1024];
    const char *arquivos[] = {"senhas.dat", "senha_mestra.txt", "salt.bin"};

    for (int i = 0; i < 3; i++)
    {
        if (!obter_caminho_dados(origem, sizeof(origem), arquivos[i]))
        {
            return 0;
        }

        snprintf(destino, sizeof(destino), "%s/%s.backup", pasta_destino, arquivos[i]);
        if (!copiar_arquivo_binario(origem, destino))
        {
            return 0;
        }
    }

    return 1;
}