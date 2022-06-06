## ISCTE-IUL: Trabalho pr�tico de Sistemas Operativos
##
## Aluno: N�: 99184      Nome: Pedro Alexandre
## Nome do M�dulo: adiciona_enfermeiros.sh
## Descri��o/Explica��o do M�dulo: 
##
##
###############################################################################
#|| [ $3 != "CS3." ]

if [ $# != 4 ] ; then
	echo 'Erro: S�ntaxe: '$0' "<nome>" <n� de c�dula profissional> "<centro de sa�de associado>" <disponibilidade>'
	exit -1
fi

touch enfermeiros.txt

if grep $3 enfermeiros.txt | grep -q $2; then
  echo "Erro: O enfermeiro j� se encontra registado no Centro de Sa�de." 
elif grep -q $3 enfermeiros.txt; then
  echo "Erro: O Centro de Sa�de introduzido j� tem um enfermeiro registado."
elif grep -q $2 enfermeiros.txt; then
  echo "Erro: O enfermeiro j� est� inscrito noutro Centro de Sa�de." 
else 
  echo "$2:$1:$3:0:$4" >> enfermeiros.txt
fi

cat enfermeiros.txt