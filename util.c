#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sodium.h>
#include "util.h"

#define CLIPBOARD_TIMEOUT_SEGUNDOS 30

void ler_senha_oculta(const char *mensagem, char *buffer, size_t tamanho)
{
    struct termios antigo, novo;

    printf("%s", mensagem);
    fflush(stdout);

    if (tcgetattr(STDIN_FILENO, &antigo) != 0)
    {
        if (fgets(buffer, tamanho, stdin) != NULL)
        {
            buffer[strcspn(buffer, "\r\n")] = '\0';
        }
        return;
    }

    novo = antigo;
    novo.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

    tcsetattr(STDIN_FILENO, TCSANOW, &novo);

    if (fgets(buffer, tamanho, stdin) != NULL)
    {
        buffer[strcspn(buffer, "\r\n")] = '\0';
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &antigo);
    printf("\n");
}

void gerar_senha_aleatoria(char *buffer, size_t tamanho)
{
    static const char conjunto[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "!@#$%^&*()-_=+[]{}|;:,.<>?";

    size_t total_chars = sizeof(conjunto) - 1;

    for (size_t i = 0; i < tamanho; i++)
    {
        uint32_t indice = randombytes_uniform((uint32_t)total_chars);
        buffer[i] = conjunto[indice];
    }
    buffer[tamanho] = '\0';
}

static void limpar_clipboard_sistema(void)
{
    FILE *pipe = popen("wl-copy --clear 2>/dev/null || xclip -selection clipboard /dev/null 2>/dev/null", "w");
    if (pipe != NULL)
    {
        pclose(pipe);
    }
}

static void agendar_limpeza_clipboard(void)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        setsid();
        sleep(CLIPBOARD_TIMEOUT_SEGUNDOS);
        limpar_clipboard_sistema();
        _exit(0);
    }
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

    if (escritos == len && status == 0)
    {
        agendar_limpeza_clipboard();
        return 1;
    }

    return 0;
}

int ler_string_timeout(char *destino, size_t tamanho, int timeout_segundos)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval tv;
    tv.tv_sec = timeout_segundos;
    tv.tv_usec = 0;

    int resultado = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

    if (resultado == 0)
    {
        return -1;
    }
    else if (resultado < 0)
    {
        return 0;
    }

    if (fgets(destino, tamanho, stdin) == NULL)
    {
        return 0;
    }

    if (strchr(destino, '\n') == NULL)
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    destino[strcspn(destino, "\r\n")] = '\0';

    return 1;
}