## ISCTE-IUL: Trabalho prático de Sistemas Operativos
##
## Aluno: Nº: 99184      Nome: Pedro Alexandre
## Nome do Módulo: adiciona_enfermeiros.sh
## Descrição/Explicação do Módulo: 
##
##
###############################################################################
#|| [ $3 != "CS3." ]

if [ $# != 4 ] ; then
	echo 'Erro: Síntaxe: '$0' "<nome>" <nº de cédula profissional> "<centro de saúde associado>" <disponibilidade>'
	exit -1
fi

touch enfermeiros.txt

if grep $3 enfermeiros.txt | grep -q $2; then
  echo "Erro: O enfermeiro já se encontra registado no Centro de Saúde." 
elif grep -q $3 enfermeiros.txt; then
  echo "Erro: O Centro de Saúde introduzido já tem um enfermeiro registado."
elif grep -q $2 enfermeiros.txt; then
  echo "Erro: O enfermeiro já está inscrito noutro Centro de Saúde." 
else 
  echo "$2:$1:$3:0:$4" >> enfermeiros.txt
fi

cat enfermeiros.txt