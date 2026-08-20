#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

void ler_senha_oculta(const char *mensagem, char *destino, size_t tamanho);
void gerar_senha_aleatoria(char *destino, size_t tamanho_senha);
int copiar_para_clipboard(const char *texto);

#endif