#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos
##
## Aluno: Nº: 99184      Nome: Pedro Alexandre 
## Nome do Módulo: stats.sh
## Descrição/Explicação do Módulo: 
##  Este módulo tem como objetivo dar informações a um utilizador e/ou "programador" (e possívelmente a outro script caso necessário). As informações podem ser entre o número de cidadãos numa localidade, os cidadãos registados com mais de 60 anos ou os enfermeiros disponíveis.
##  O funcionamento deste Módulo é descrito ao longo do mesmo.
##
###############################################################################

if ! [ -r cidadaos.txt ]; then #verifica se cidadaos.txt e enfermeiros.txt são passiveis de serem lidos
  echo "Erro: cidadaos.txt não existe na corrente diretoria ou não é legível"
  exit -1
elif ! [ -r enfermeiros.txt ]; then
  echo "Erro: enfermeiros.txt não existe na corrente diretoria ou não é legível"
  exit -2
fi

case $1 in

cidadaos) # primeira opção do comando case para quando o primeiro argumento for "cidadaos" e o segundo for uma localidade, o script apresentar a quantidade de cidadãos nessa localidade.
if [[ $# == 2 ]]; then # verifica se o número de argumentos está correto para evitar possiveis problemas.
  n=$(cut -d':' -f4 cidadaos.txt | grep -i "$2" | wc -l) #variável que procura entre as localidades pela localidade dada no 2º argumento no ficheiro cidadaos.txt e devolve o número de linhas com essa localidade (ptt o número total de cidadãos nessa localidade)
  echo "O número de cidadãos registados em $2 é $n." #frase de output esperada com a localidade e o número de cidadaos na mesma
else echo 'Erro: Síntaxe: $0 [ cidadaos "<localidade>" ]' #frase de erro caso se tenha inserido o número errado de argumentos para esta opção do case
fi
;;

registados) 
if [[ $# == 1 ]]; then # verifica se o número de argumentos está correto para evitar possiveis problemas.
  sort -k 3 -t ':' -r cidadaos.txt | awk -F ':' '$3 > 60 {print $2 "\t" $1}' # esta linha, através do sort e do awk, sorteia os cidadãos começando nos com maior e idade e acabando naqueles com 60 anos e apresenta: o nome e o nº de utente do mesmo
else echo 'Erro: Síntaxe: $0 [ registados ]' #frase de erro caso se tenha inserido o número errado de argumentos para esta opção do case
fi
;;
  
enfermeiros)
if [[ $# == 1 ]]; then # verifica se o número de argumentos está correto para evitar possiveis problemas.
  grep -i ".*1$" enfermeiros.txt | cut -d':' -f2 # esta linha procura no ficheiro enfermeiros.txt pelos enfermeiros q no final da sua linha de informações possuem um 1 (de disponíveis) e apresenta o nome que foi inserido na sua linha
else echo 'Erro: Síntaxe: $0 [ enfermeiros ]' #frase de erro caso se tenha inserido o número errado de argumentos para esta opção do case
fi
;;

*) echo 'Erro: Síntaxe: $0 [ cidadaos "<localidade>" | registados | enfermeiros ]' #frase de erro caso se tenha inserido argumentos inválidos/argumentos a mais
;;
esac