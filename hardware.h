#ifndef HARDWARE_H
#define HARDWARE_H

#include <stddef.h>

#define HARDWARE_RESPOSTA_BYTES 32

int hardware_obter_chave(const unsigned char *desafio, size_t tamanho_desafio, unsigned char *saida_chave);

#endif
