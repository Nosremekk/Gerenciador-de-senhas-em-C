# Gerenciador de Senhas em C

Um gerenciador de credenciais de linha de comando desenvolvido em C, com foco em modularização, gerenciamento manual de memória, persistência de dados e segurança utilizando a biblioteca [libsodium](https://doc.libsodium.org/).

O projeto armazena credenciais localmente e utiliza uma senha mestra para autenticação e derivação da chave utilizada na criptografia das senhas.

> Projeto desenvolvido com fins educacionais e de estudo de programação em C, sistemas Linux/POSIX, gerenciamento de memória, persistência de dados e conceitos de segurança aplicada.

## Funcionalidades

### Autenticação

- Criação de senha mestra na primeira execução.
- Verificação da senha mestra utilizando Argon2id através da libsodium.
- Derivação de uma chave de criptografia de 256 bits a partir da senha mestra.
- Ocultação dos caracteres digitados no terminal utilizando `termios`.
- Limpeza de buffers sensíveis utilizando `sodium_memzero`.

### Gerenciamento de credenciais

- Adicionar credenciais.
- Listar registros cadastrados.
- Selecionar registros por ID.
- Modificar registros.
- Remover registros.
- Buscar por ID.
- Buscar parcialmente por serviço.
- Buscar parcialmente por usuário.
- Busca sem diferenciação entre letras maiúsculas e minúsculas.
- Reutilização de IDs removidos.

Cada registro contém:

```text
ID
Serviço
Usuário
Senha
```

O banco de dados utiliza uma estrutura dinâmica armazenada na heap e aumenta sua capacidade conforme necessário através de `realloc`.

## Criptografia

As senhas armazenadas em disco são protegidas utilizando a biblioteca libsodium.

O projeto utiliza:

```text
crypto_secretbox_easy
```

com o algoritmo:

```text
XSalsa20-Poly1305
```

A operação fornece confidencialidade e autenticação dos dados criptografados.

A chave utilizada para a criptografia é derivada da senha mestra utilizando:

```text
Argon2id
```

através de:

```text
crypto_pwhash
```

O salt utilizado na derivação da chave é armazenado separadamente.

## Gerador de Senhas

O programa possui um gerador de senhas aleatórias utilizando:

```text
randombytes_uniform
```

da libsodium.

As senhas podem possuir entre 8 e 64 caracteres e podem incluir:

- Letras minúsculas.
- Letras maiúsculas.
- Números.
- Caracteres especiais.

## Auditoria de Senhas

O projeto possui um módulo de auditoria capaz de analisar as senhas armazenadas.

A análise considera fatores como:

- Comprimento.
- Variedade de caracteres.
- Conjuntos de caracteres utilizados.
- Estimativa de força.
- Reutilização de senhas.

As senhas são classificadas em diferentes níveis de força.

## Clipboard

As senhas podem ser copiadas diretamente para a área de transferência.

O programa possui suporte para:

```text
wl-copy
```

em ambientes Wayland e:

```text
xclip
```

em ambientes X11.

Após a cópia, um processo separado pode limpar automaticamente a área de transferência após 30 segundos.

A implementação utiliza recursos POSIX como:

```text
fork()
setsid()
sleep()
```

## Bloqueio Automático

O programa possui bloqueio automático por inatividade.

O menu principal utiliza:

```text
select()
```

para aguardar a entrada do usuário.

Após um período de inatividade:

1. A sessão é bloqueada.
2. Dados sensíveis são removidos da memória.
3. O usuário precisa se autenticar novamente.
4. O banco de dados é carregado novamente após a autenticação.

## Troca da Senha Mestra

O programa permite alterar a senha mestra.

O processo envolve:

1. Confirmação da senha atual.
2. Criação de uma nova senha mestra.
3. Geração de uma nova chave derivada.
4. Atualização dos dados de autenticação.
5. Atualização da proteção do banco de dados.

## Backup e Exportação

### Backup

O programa permite criar backups dos arquivos utilizados pelo gerenciador.

### Exportação CSV

Também é possível exportar as credenciais para um arquivo CSV legível.

Como essa operação gera um arquivo sem a mesma proteção criptográfica do banco, a exportação exige confirmação da senha mestra.

> O arquivo CSV contém informações sensíveis e deve ser protegido ou removido após o uso.

## Estrutura do Projeto

```text
.
├── main.c
├── autenticacao.c
├── autenticacao.h
├── arquivo.c
├── arquivo.h
├── registro.c
├── registro.h
├── operacoes.c
├── operacoes.h
├── auditoria.c
├── auditoria.h
├── util.c
├── util.h
├── Makefile
├── README.md
└── LICENSE
```

### `main.c`

Ponto de entrada da aplicação.

Responsável por:

- Inicializar a aplicação.
- Autenticar o usuário.
- Inicializar o banco de dados.
- Controlar o menu principal.
- Gerenciar o fluxo principal da sessão.

### `autenticacao.c`

Responsável pela autenticação e gerenciamento da senha mestra.

Principais responsabilidades:

- Criação da senha mestra.
- Verificação da senha.
- Derivação da chave criptográfica.
- Gerenciamento do salt.
- Troca da senha mestra.

### `arquivo.c`

Responsável pela persistência dos dados.

Principais responsabilidades:

- Carregar o banco de dados.
- Salvar registros.
- Criptografar dados sensíveis.
- Descriptografar dados.
- Realizar operações de backup e exportação.

### `registro.c`

Responsável pela estrutura de dados do banco.

Utiliza:

```text
malloc
realloc
free
```

para gerenciar dinamicamente os registros.

Também implementa:

- Inicialização.
- Liberação de memória.
- Expansão de capacidade.
- Validação de IDs.
- Busca de registros.

### `operacoes.c`

Contém os fluxos principais de interação com o usuário.

Implementa operações como:

- Adicionar credenciais.
- Listar registros.
- Modificar registros.
- Deletar registros.
- Pesquisar por ID.
- Pesquisar por serviço.
- Pesquisar por usuário.
- Gerar senhas.
- Copiar senhas.
- Exportar e realizar backup.

### `auditoria.c`

Responsável pela análise das senhas armazenadas.

Realiza:

- Análise individual de senhas.
- Classificação de força.
- Verificação de características.
- Detecção de reutilização.

### `util.c`

Contém funções auxiliares do projeto.

Inclui:

- Leitura de senhas sem exibir caracteres no terminal.
- Geração aleatória de senhas.
- Integração com clipboard.
- Limpeza automática do clipboard.
- Funções auxiliares de entrada.

## Pré-requisitos

O projeto requer:

- GCC ou outro compilador compatível com C.
- Make.
- `pkg-config`.
- libsodium.

Para suporte ao clipboard:

- `wl-clipboard` em Wayland.
- Ou `xclip` em X11.

## Instalação das dependências

### Fedora

```bash
sudo dnf install gcc make pkgconf-pkg-config libsodium-devel wl-clipboard
```

Para ambientes X11:

```bash
sudo dnf install xclip
```

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install build-essential make pkg-config libsodium-dev wl-clipboard
```

Para ambientes X11:

```bash
sudo apt install xclip
```

## Compilação

Clone o repositório:

```bash
git clone https://github.com/Nosremekk/Gerenciador-de-senhas-em-C.git
cd Gerenciador-de-senhas-em-C
```

Compile utilizando:

```bash
make
```

Execute com:

```bash
./gerenciador_senhas
```

## Outros comandos

Limpar arquivos compilados:

```bash
make clean
```

Instalar:

```bash
make install
```

Desinstalar:

```bash
make uninstall
```

## Tecnologias e Conceitos Utilizados

- C
- GCC
- Make
- Makefile
- libsodium
- Argon2id
- XSalsa20-Poly1305
- `crypto_secretbox`
- `crypto_pwhash`
- `randombytes_uniform`
- `sodium_memzero`
- Gerenciamento manual de memória
- `malloc`
- `realloc`
- `free`
- Structs
- Ponteiros
- Arquivos e persistência
- POSIX
- `termios`
- `select`
- `fork`
- `setsid`
- Clipboard em Wayland e X11

## Objetivos do Projeto

Este projeto foi desenvolvido como uma forma de estudar, na prática, diferentes áreas da programação em C.

Entre os principais tópicos explorados estão:

- Estruturas de dados.
- Modularização.
- Ponteiros.
- Gerenciamento dinâmico de memória.
- Arquivos e persistência.
- Tratamento de entrada do usuário.
- Programação em ambientes Linux.
- APIs POSIX.
- Processos.
- Bibliotecas externas.
- Criptografia aplicada.
- Autenticação.
- Gerenciamento de dados sensíveis.
- Organização de projetos maiores em C.

## Limitações e Observações

Este projeto possui foco educacional.

Apesar de utilizar primitivas criptográficas modernas fornecidas pela libsodium, um gerenciador de senhas é uma aplicação de segurança crítica. A utilização em ambientes reais exige revisão de código, testes extensivos e auditoria de segurança independente.

Não é recomendado utilizar versões experimentais do projeto como única forma de armazenamento de credenciais importantes.

## Licença

Este projeto está distribuído sob a licença MIT.
