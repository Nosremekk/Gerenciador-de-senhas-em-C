#ifndef AUDITORIA_H
#define AUDITORIA_H

#include "registro.h"

typedef struct
{
    int pontuacao;
    int comprimento;
    int tem_maiuscula;
    int tem_minuscula;
    int tem_numero;
    int tem_simbolo;
    const char *classificacao;
} AnaliseSenha;

AnaliseSenha auditar_senha_individual(const char *senha);
void executar_auditoria_banco(const BancoDeDados *banco);

#endif