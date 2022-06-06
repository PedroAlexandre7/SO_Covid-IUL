#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos
##
## Aluno: Nº: 99184      Nome: Pedro Alexandre
## Nome do Módulo: lista_cidadaos.sh
## Descrição/Explicação do Módulo: 
##  Este módulo tem o objetivo de criar um ficheiro cidadaos.txt com determinadas informações correspondentes aos cidadãos apresentados no ficheiro listagem.txt.
##  O funcionamento deste Módulo é descrito ao longo do mesmo.
##
###############################################################################

if [ $# != 0 ]; then #verifica se o número de args está correto e se listagem.txt é passivel de ser lido
	echo 'Erro: Síntaxe: '$0''
	exit -1
elif ! [ -r listagem.txt ]; then
  echo "Erro: listagem.txt não existe na corrente diretoria ou não é legível"
  exit -2
fi


cat listagem.txt | sed "s/ | /:/g" | awk -F "[-:]" '{print (( 10000+NR )) ":" $2 ":" (( 2021-$6 )) ":" $8 ":" $10 ":0"}' > cidadaos.txt #Para ser possível retirar nomes de cidadãos e localidades com mais que uma palavra o sed transforma os separadores do listagem.txt dados pelo cat em ":". Depois de o sed mudar os separadores do listagem.txt o awk retira as informações necessárias e organiza-as, criando assim uma lista de cidadãos com as informações de cada um organizadas da seguinte forma: Primeiro o número de utente q é dado pela soma de 10000 e o número da linha do mesmo na listagem.txt, dps o nome, a idade (dada pela subtração do ano 2021 pelo ano de nascimento), a localidade apresentada, o número de telémovel e o número de vacinas efetuadas, que é dada como 0.
cat cidadaos.txt # apresenta no terminal o conteúdo guardado no ficheiro cidadaos.txt na linha anterior