# Gerenciador de Senhas em C

Um gerenciador de senhas para terminal (CLI) desenvolvido em C com foco em segurança criptográfica moderna utilizando a biblioteca **libsodium**.

---

## Funcionalidades

- **Criptografia Autenticada:** Senhas armazenadas em disco são cifradas usando `crypto_secretbox_easy` (XSalsa20-Poly1305) com chaves derivadas da senha mestra.
- **Hash Seguro de Senha Mestra:** Proteção da chave mestra via algoritmo Argon2id (`crypto_pwhash`).
- **Gerenciamento Dinâmico de Memória:** Estrutura expansível em tempo de execução via `realloc`, permitindo registros ilimitados.
- **Auditoria de Senhas:** Analisador integrado de complexidade, identificação de senhas fracas e detecção de reutilização de credenciais.
- **Gerador de Senhas Aleatórias:** Criação de senhas criptograficamente fortes com comprimento personalizável (`randombytes_uniform`).
- **Ocultação de Entrada:** Leitura segura no terminal desabilitando o eco visual via `<termios.h>`.
- **Área de Transferência:** Suporte para cópia direta da credencial para o clipboard (`wl-copy` / `xclip`).
- **Limpeza de Memória:** Sanitização segura de buffers sensíveis com `sodium_memzero`.

---

## Pré-requisitos

### Fedora / RHEL
```bash
sudo dnf install gcc make libsodium-devel wl-clipboard
```

### Ubuntu / Debian
```bash
sudo apt update
sudo apt install gcc make libsodium-dev wl-clipboard
```

---

## Compilação e Execução

Clone o repositório e compile o projeto usando o `Makefile`

```bash
# Compilar o binário
make

# Executar a aplicação
./gerenciador

# Limpar arquivos compilados
make clean
```

---

## Estrutura do Projeto

| Arquivo | Descrição |
| :--- | :--- |
| `main.c` | Ponto de entrada, menu principal e controle de fluxo. |
| `autenticacao.c` / `autenticacao.h` | Criação/validação da senha mestra e derivação da chave de 256 bits. |
| `registro.c` / `registro.h` | Gerenciamento dinâmico da estrutura de dados em memória. |
| `arquivo.c` / `arquivo.h` | Cifragem, decifragem e persistência em disco (`senhas.txt`). |
| `operacoes.c` / `operacoes.h` | Rotinas de cadastro, listagem, busca, edição e exclusão. |
| `auditoria.c` / `auditoria.h` | Relatórios de força e detecção de duplicidade de senhas. |
| `util.c` / `util.h` | Leitura oculta no terminal, gerador randômico e integração com clipboard. |
| `Makefile` | Automação da compilação e linkedição com a libsodium. |

---

## Licença

Distribuído sob a licença MIT.
