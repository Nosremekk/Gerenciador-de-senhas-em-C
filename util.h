#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

#define TIMEOUT_INATIVIDADE_SEGUNDOS 300

void ler_senha_oculta(const char *mensagem, char *buffer, size_t tamanho);
void gerar_senha_aleatoria(char *buffer, size_t tamanho);
int copiar_para_clipboard(const char *texto);
int ler_string_timeout(char *destino, size_t tamanho, int timeout_segundos);

#endif