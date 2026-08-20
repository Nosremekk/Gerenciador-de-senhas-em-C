#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sodium.h>
#include "util.h"

void ler_senha_oculta(const char *mensagem, char *destino, size_t tamanho)
{
    struct termios antigo, novo;

    while (1)
    {
        printf("%s", mensagem);
        fflush(stdout);

        if (tcgetattr(STDIN_FILENO, &antigo) != 0)
        {
            if (fgets(destino, tamanho, stdin) == NULL)
            {
                continue;
            }
            destino[strcspn(destino, "\r\n")] = '\0';
            if (destino[0] == '\0')
            {
                printf("A senha nao pode ser vazia.\n");
                continue;
            }
            return;
        }

        novo = antigo;
        novo.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

        if (tcsetattr(STDIN_FILENO, TCSANOW, &novo) != 0)
        {
            if (fgets(destino, tamanho, stdin) == NULL)
            {
                continue;
            }
            destino[strcspn(destino, "\r\n")] = '\0';
            if (destino[0] == '\0')
            {
                printf("A senha nao pode ser vazia.\n");
                continue;
            }
            return;
        }

        char *res = fgets(destino, tamanho, stdin);

        tcsetattr(STDIN_FILENO, TCSANOW, &antigo);
        printf("\n");

        if (res == NULL)
        {
            printf("Erro ao ler senha. Tente novamente.\n");
            continue;
        }

        destino[strcspn(destino, "\r\n")] = '\0';

        if (destino[0] == '\0')
        {
            printf("A senha nao pode ser vazia.\n");
            continue;
        }

        return;
    }
}

void gerar_senha_aleatoria(char *destino, size_t tamanho_senha)
{
    static const char charset[] = 
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "!@#$%&*()-_=+[]{}<>?";
    
    size_t charset_size = sizeof(charset) - 1;

    for (size_t i = 0; i < tamanho_senha; i++)
    {
        uint32_t indice = randombytes_uniform((uint32_t)charset_size);
        destino[i] = charset[indice];
    }
    destino[tamanho_senha] = '\0';
}

int copiar_para_clipboard(const char *texto)
{
    FILE *pipe = popen("wl-copy 2>/dev/null || xclip -selection clipboard 2>/dev/null", "w");
    if (pipe == NULL)
    {
        return 0;
    }

    size_t len = strlen(texto);
    size_t escritos = fwrite(texto, 1, len, pipe);
    int status = pclose(pipe);

    return (escritos == len && status == 0);
}