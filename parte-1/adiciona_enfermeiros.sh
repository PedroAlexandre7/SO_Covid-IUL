#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos
##
## Aluno: Nº: 99184      Nome: Pedro Alexandre
## Nome do Módulo: adiciona_enfermeiros.sh
## Descrição/Explicação do Módulo: 
##  Este módulo tem como objetivo criar e guardar a informação dos enfermeiros para posterior agendação de consultas pelo script agendamento.sh. Para tal é necessário introduzir os seguintes dados do enfermeiro: Nome, nº de cédula profissional, centro de saúde associado ao mesmo e a sua disponibilidade para efetuar vacinas.
##  O funcionamento deste Módulo é descrito ao longo do mesmo.
##
###############################################################################

if [ $# != 4 ]; then # conjunto de ifs com os seguintes objetivos: impedir que sejam dados mais que 4 args; impedir que os args do nome n seja letras e espaços; impedir q os args do nº de cédula n sejam algarismos; impedir nomes de centros de saúde q n tenham "CS"; e impedir args para a disponibilidade diferentes de 1 ou 0 
	echo 'Erro: Síntaxe: '$0' "<nome>" <nº de cédula profissional> "<centro de saúde associado>" <disponibilidade>'
	exit -1
elif ! [[ $1 =~ [a-zA-Z] ]]; then
  echo -e "Erro: $1 não é um nome válido"
  exit -2 
elif [[ $2 =~ [^0-9] ]]; then
  echo -e "Erro: $2 não é um número válido"
  exit -3 
elif ! echo $3 | grep -q "^CS.*"; then
  echo -e 'Erro: O nome do centro de saúde deve começar com "CS"'
	exit -4
elif [ $4 != 0 ] && [ $4 != 1 ]; then
	echo -e 'Erro: [Disponibilidade = 1 ou 0]'
	exit -5
fi

touch enfermeiros.txt #touch para evitar possiveis erros ao acessar enfermeiros.txt e permitir o if seguinte
if ! [ -r enfermeiros.txt ]; then # verifica se enfermeiros.txt é passível de ser lido e escrito
  echo "Erro: enfermeiros.txt não existe na corrente diretoria ou não é legível"
  exit -6
elif ! [ -w enfermeiros.txt ]; then # os ifs estão separados para facilitar a leitura do erro
  echo "Erro: enfermeiros.txt não pode ser escrito"
  exit -7
fi

if grep -i "$3" enfermeiros.txt | grep -q -i "$2"; then # conjunto de ifs para verificar se o enfermeiro pode ser adicionado procurando pelo centro de saúde e ou nome do enfermeiros no ficheiro enfermeiros.txt. No 1º verifica se o enfermeiro se encontra no centro de saúde dado
  echo -e "Erro: O enfermeiro já se encontra registado no Centro de Saúde.\n" 
elif grep -q -i "$3" enfermeiros.txt; then # verifica se o centro de saúde dado contém algum enfermeiro (procurando pelo centro de saúde no ficheiro enfermeiros.txt)
  echo -e "Erro: O Centro de Saúde introduzido já tem um enfermeiro registado.\n"
elif grep -q -i "$2" enfermeiros.txt; then # verifica se o enfermeiro já se encontra inscrito noutro Centro de Saúde (procurando pelo enfermeiro no ficheiro enfermeiros.txt)
  echo -e "Erro: O enfermeiro já está inscrito noutro Centro de Saúde.\n" 
else
  echo "$2:$1:$3:0:$4" >> enfermeiros.txt # adiciona numa nova linha do ficheiro enfermeiros.txt o nome, nº de cédula, centro de saúde, nº de vacinações efetuadas (sendo por default zero) e a disponibilidade do mesmo
fi

cat enfermeiros.txt # apresenta os enfermeiros registados