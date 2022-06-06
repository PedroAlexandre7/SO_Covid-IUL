#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos
##
## Aluno: Nº: 99184      Nome: Pedro Alexandre
## Nome do Módulo: agendamento.sh
## Descrição/Explicação do Módulo: 
##  Este módulo tem como objetivo criar um ficheiro chamado agenda.txt, com a data de vacinação da doença Covid-19 dos vários cidadãos já registados e os enfermeiros que irão realizar a vacinação aos cidadãos. Estes têm que se encontrar
##  registados no ficheiro cidadaos.txt. (o script lista_cidadaos.sh tem como objetivo criar esse ficheiro a partir de outro chamado listagem.txt).
##  O funcionamento deste Módulo é descrito ao longo do mesmo.
##
###############################################################################

if [ $# != 0 ]; then #verifica se o número de args está correto e se cidadaos.txt e enfermeiros.txt são passiveis de serem lidos
	echo 'Erro: Síntaxe: '$0''
	exit -1
elif ! [ -r cidadaos.txt ]; then
  echo "Erro: cidadaos.txt não existe na corrente diretoria ou não é legível"
  exit -2
elif ! [ -r enfermeiros.txt ]; then
  echo "Erro: enfermeiros.txt não existe na corrente diretoria ou não é legível"
  exit -3
fi

if [ -e agenda.txt ]; then #apaga, caso exista, o ficheiro agenda.txt
  rm agenda.txt
fi

i=$(cat cidadaos.txt | wc -l) #variável que guarda o número de cidadãos para estes serem tratados agendados iterativamente
thi=$(date +"%Y-%m-%d") #identifica a data para as consultas a serem agendadas

# O seguinte while percorre os cidadãos e guarda numa variável ("final") os cidadãos que têm um enfermeiro no centro de saúde da sua localidade
# É composto por 5 variáveis, nas quais 3 identificam, qdo juntas, "o nome e número da cédula pessoal do enfermeiro que vai efetuar a vacinação, o nome e número de utente do cidadão que vai ser vacinado, o centro de saúde onde vai ser realizada a vacinação e a data da vacinação". Estas são guardadas juntas no final da variavel final.
while [ $i -ge 1 ]; do 
  sec=$(tail -$i cidadaos.txt | head -1 | awk -F [:] '{print $2 ":" $1 ":CS" $4}') #  sec=second  identifica "o nome e número de utente do cidadão, bem como o centro de saúde onde este pode ser vacinado"
  city=$(echo $sec | awk -F [:] '{print $3":"}')
  if ( grep -q "$city.*1$" enfermeiros.txt ); then #o if tem como objetivo garatir que apenas se guarda na várivel final cidadãos q possuam um enfermeiro disponível na localidade, para isso usa-se a variável "city"
    fir=$(grep "$city" enfermeiros.txt | awk -F[:] '{print $2 ":" $1}') #  fir=first  identifica "o nome e número da cédula pessoal do enfermeiro que vai efetuar a vacinação" através da variavel "city"
    final="$final\n$fir:$sec:$thi" #contém as informações do agendamento separando cada cidadao por \n
  fi
  i=$(( $i - 1 )) #itera o while 
done

echo -e $final | sort | tail -n +2 > agenda.txt #guarda linha a linha, organiza e guarda o "final" (o tail tem como objetivo não guardar um parágrafo vazio criado qdo o final é "criado no script", mas é facultativo)
cat agenda.txt #apresenta a as consultas agendadas