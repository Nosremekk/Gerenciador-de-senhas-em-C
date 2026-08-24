#ifndef ARQUIVO_H
#define ARQUIVO_H

#include "registro.h"

int obter_caminho_dados(char *destino, size_t tamanho, const char *nome_arquivo);
int carregar_banco(BancoDeDados *banco);
int salvar_banco(const BancoDeDados *banco);
int exportar_banco_csv(const BancoDeDados *banco, const char *caminho_saida);
int exportar_backup_criptografado(const char *pasta_destino);

#endif