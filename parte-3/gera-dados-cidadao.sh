#!/bin/bash

## Este script imprime no stdout os dados de um cidadao aleatório EXISTENTE no ficheiro cidadaos.dat

## O output depois pode ser usado como input do programa cidadao:
##   ./cidadao                        # introduzir os dados manualmente
##   ./cidadao < dados_cidadao.txt    # podemos fazer ./gera-dados-cidadao.sh > dados_cidadao.txt previamente para depois usarmos os dados múltiplas vezes

## O utilizador pode passar como argumento uma string (por exemplo, uma localidade) e o script vai apenas buscar cidadãos dessa localidade
## Se o utilizador usar mais do que um argumento, o script vai buscar a intersecção desses argumentos, e.g. ./gera-dados-cidadao.sh Lisboa Pedro vai trazer um Pedro de Lisboa

cmd="./show-cidadaos.sh"
for arg in "$@"; do cmd="$cmd | grep -w \"$arg\""; done
cidadao=$(eval $cmd | shuf -n1)
num_utente=$(echo $cidadao | cut -d'|' -f2 | xargs)
nome=$(echo $cidadao | cut -d'|' -f3 | xargs)
printf "$num_utente\n$nome\n" # > dados_cidadao.txt # Dantes gerava sempre ficheiro, agora tirei essa opção, se o utilizador quiser, faz ele o redirecionamento.