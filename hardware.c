#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sodium.h>
#include "hardware.h"

#define DISPOSITIVO_SERIAL "/dev/ttyACM0"

static int configurar_porta_serial(int fd)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) != 0)
    {
        return 0;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= (CREAD | CLOCAL);

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY | IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VTIME] = 30;
    tty.c_cc[VMIN] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        return 0;
    }

    tcflush(fd, TCIOFLUSH);
    return 1;
}

static void bytes_para_hex(const unsigned char *entrada, size_t tamanho, char *saida)
{
    for (size_t i = 0; i < tamanho; i++)
    {
        sprintf(saida + (i * 2), "%02x", entrada[i]);
    }
    saida[tamanho * 2] = '\0';
}

static int hex_para_bytes(const char *hex, unsigned char *saida, size_t tamanho_saida)
{
    for (size_t i = 0; i < tamanho_saida; i++)
    {
        unsigned int valor;
        if (sscanf(hex + (i * 2), "%02x", &valor) != 1)
        {
            return 0;
        }
        saida[i] = (unsigned char)valor;
    }
    return 1;
}

int hardware_obter_chave(const unsigned char *desafio, size_t tamanho_desafio, unsigned char *saida_chave)
{
    int fd = open(DISPOSITIVO_SERIAL, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        return 0;
    }

    if (!configurar_porta_serial(fd))
    {
        close(fd);
        return 0;
    }

    usleep(500000);

    char desafio_hex[tamanho_desafio * 2 + 2];
    bytes_para_hex(desafio, tamanho_desafio, desafio_hex);
    strcat(desafio_hex, "\n");

    size_t tamanho_msg = strlen(desafio_hex);
    if (write(fd, desafio_hex, tamanho_msg) != (ssize_t)tamanho_msg)
    {
        close(fd);
        return 0;
    }

    char resposta_hex[128];
    size_t lidos_total = 0;

    while (lidos_total < sizeof(resposta_hex) - 1)
    {
        char ch;
        ssize_t n = read(fd, &ch, 1);
        if (n <= 0)
        {
            break;
        }
        if (ch == '\n' || ch == '\r')
        {
            if (lidos_total > 0)
            {
                break;
            }
            continue;
        }
        resposta_hex[lidos_total++] = ch;
    }
    resposta_hex[lidos_total] = '\0';

    close(fd);

    if (strlen(resposta_hex) != HARDWARE_RESPOSTA_BYTES * 2)
    {
        sodium_memzero(resposta_hex, sizeof(resposta_hex));
        return 0;
    }

    if (!hex_para_bytes(resposta_hex, saida_chave, HARDWARE_RESPOSTA_BYTES))
    {
        sodium_memzero(resposta_hex, sizeof(resposta_hex));
        return 0;
    }

    sodium_memzero(resposta_hex, sizeof(resposta_hex));
    return 1;
}
