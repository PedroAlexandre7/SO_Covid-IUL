#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos
##
## Aluno: Nº: 99184      Nome: Pedro Alexandre 
## Nome do Módulo: menu.sh
## Descrição/Explicação do Módulo: 
##  Este módulo consiste na apresentação de um menu simples para q qualquer utilizador possa: guardar no ficheiro cidadaos.txt o número de cidadaos; adicionar enfermeiros ao ficheiro enfermeiros.txt; verificar estatísticas sobre o número de cidadaos já listados em qualquer localidade e os cidadaos com mais de 60 anos no ficheiro cidadaos.txt, ou também os enfermeiros adicionados no ficheiro enfermeiros.txt disponiveis; agendar consultas de vacinção para os cidadaos no ficheiro cidadaos.txt conforme os enfermeiros disponiveis no centro de saude da localidade listados no ficheiro enfermeiros.txt.
##  Para que seja um sistema mais simples e fechado é constituido por imensos echo's e clear's. Quando estes tiverem apenas como propósito tornar o ambiente amigável ao utilizador, e aos seus olhos, terão um comentário a dizer "simples". Qdo tiverem um objetivo de mostrar opções para posterior escolha através de um case, terão um comentário a dizer "escolhas". Qdo tiverem outro propósito estarão explicados nos comentários.
##  O resto do funcionamento deste Módulo é descrito ao longo do mesmo.
##
###############################################################################


if [ $# != 0 ]; then #verifica se o número de args está correto e se os scripts lista_cidadaos.sh, adiciona_enfermeiros.sh, stats.sh e agendamento.sh existem e correm para posteriomente serem acessados, caso contrário o script acaba
	echo 'Erro: Síntaxe: '$0''
	exit -1
elif ! [ -x lista_cidadaos.sh ]; then
  echo "Erro: lista_cidadaos.sh não existe na corrente diretoria ou não é executável"
  exit -2
elif ! [ -x adiciona_enfermeiros.sh ]; then
  echo "Erro: adiciona_enfermeiros.sh não existe na corrente diretoria ou não é executável"
  exit -3
elif ! [ -x stats.sh ]; then
  echo "Erro: stats.sh não existe na corrente diretoria ou não é executável"
  exit -4
elif ! [ -x agendamento.sh ]; then
  echo "Erro: agendamento.sh não existe na corrente diretoria ou não é executável"
  exit -5
fi



choosen=-1 # variável que permite ao utilizador escolher quando há mais que uma opção numerável. tbm é usada para guardar a cidade para o script do stats procurar [a ideia de usar a mesma variável é da cadeira de microprocessadores, pois em alguns processadores, ter poucas variáveis permite maior velocidade de processamento de um programa]

clear # simples
while [[ $choosen != 0 ]]; do #while que garante que o script só deixa de correr qdo for pedido, ou seja qdo a variavel choosen for 0
  echo -e "  -- Covid-IUL -- \n" # simples
  echo -e -n "O que pretende fazer?\n\n  1. Listar cidadãos\n  2. Adicionar enfermeiro\n  3. Stats\n  4. Agendar vacinação\n  0. Sair\n\nOpção:  " # simples # escolhas
  read choosen # lê a escolha pretendida pelo utilizador
  echo  # simples
  case $choosen in # comando que permite escolher uma opção através da informação lida do input através da variável choosen
    1|1.) clear # simples # caso a escolha seja a opção 1, irá ser executado o script lista_cidadaos.sh
      echo -e "  -- Covid-IUL -- \n" # simples
      echo -e "  Lista de cidadãos registados\n" # simples
      ./lista_cidadaos.sh
    ;;

    2|2.) clear # simples # caso a escolha seja a opção 2, irá ser executado o script adiciona_enfermeiros.sh, com as informções dadas após cada read (dois pontos para o utilizador) como argumentos
      echo -e "  -- Covid-IUL -- \n" # simples
      echo -e "  Adicionar enfermeiro\n" # simples
      echo -n "Nome:  " # simples # indica ao utlizador que o q escrever será o nome q será usado para adicionar ao enfermeiro
      read nome # lê o que o utilizador escrever para ser usado como 1º argumento para o script adiciona_enfermeiros.sh
      echo -n "Nº de Cédula:  " # simples # indica ao utlizador que o q escrever será o Nº de Cédula q será usado para adicionar o enfermeiro à lista dos mesmos
      read num # o mesmo que nome, mas para o 2º arg
      echo -n "Centro de Saúde:  " # simples # o mesmo q o echo anterior, mas para o CS
      read CS # o mesmo que nome, mas para o 3º arg
      echo -n "Disponibilidade:  " # simples # # o mesmo q o echo anterior, mas para a disponibilidade
      read disp # o mesmo que nome, mas para o 4º arg
      echo # simples
      ./adiciona_enfermeiros.sh "$nome" "$num" "$CS" "$disp" #as aspas permitem que o nome e o CS tenham mais espaços para caso como "Nuno Alvez" ou "Viana do Castelo" e que as mensagens de erro dentro do script adiciona_enfermeiros.sh sejam fáceis de entender para o utilizador para o caso de num e disp.
    ;;

    3|3.) clear # simples # caso a escolha seja a opção 3, irá ser mostrado um outro menu para escolher uma das funções do stats.sh
      echo -e "  -- Covid-IUL -- \n" # simples
      echo -e -n "Que informação deseja saber?\n\n  1. Número de cidadãos\n  2. Cidadãos registados\n  3. Enfermeiros disponíveis\n  0. Sair\n\n\nOpção:  " # simples # escolhas
      read choosen # lê a escolha pretendida pelo utilizador
      echo # simples

      case $choosen in # comando que permite escolher uma opção através da informação lida do input através da variável choosen

        1|1.) # caso a escolha seja a opção 1, irá ser executado o script stats.sh com cidadaos como 1º arg, e como 2º arg o q o utilizador escrever após o próximo read
          echo -e "  Número de cidadãos\n" # simples
          echo -n "Localidade:  " # simples # indica ao utlizador que o q escrever será a Localidade em que será verificado o nº de cidadãos registados
          read choosen # lê a localidade para ser usada como 2º argumento do script
          echo # simples
          ./stats.sh cidadaos "$choosen"
        ;;

        2|2.) # caso a escolha seja a opção 2, irá ser executado o script stats.sh com registados como 1º arg
          echo -e "  Cidadãos registados\n" # simples
          ./stats.sh registados
        ;;

        3|3.) # caso a escolha seja a opção 3, irá ser executado o script stats.sh com enfermeiros como 1º arg
          echo -e "  Enfermeiros disponíveis\n" # simples
          ./stats.sh enfermeiros
        ;;

        0|0.)  # caso a escolha seja a opção 4, o case acabará e voltará para o menu principal
          echo "A sair para o menu..." # simples
          sleep .5 # simples # pausa no código para ser vísivel a string anterior
        ;;

        *) echo "Erro: Para selecionar uma opção de um menu deverá inserir o número correspondente." # erro para caso o utilizador n escolha nennhuma das opções anteriores q volta ao menu principal
      esac
    ;;

    4|4.) clear # simples # caso a escolha seja a opção 4, irá ser executado o script agendamento.sh
      echo -e "  -- Covid-IUL -- \n" # simples
      echo -e "  Agendar vacinações\n" # simples
      ./agendamento.sh
      echo -e "\n  Vacinações agendadas!" # simples
    ;;

    0|0.) # caso a escolha seja a opção 0, o while irá ser acabado e o script acabará por deixar de estar a ser corrido
      break
    ;;

    *) 
      echo "Erro: Para selecionar uma opção do menu deverá inserir o número correspondente." # erro para caso o utilizador n escolha nennhuma das opções anteriores
    ;;
  esac

  echo -e -n "\nPrima Enter para continuar...  " # simples # este echo com o read seguinte tem o objetivo de permitir q sejam visiveis "várias páginas q o menu é capaz de apresentar" e q estas só desapareçam para voltar a apresentar o menu qdo o utilizador clicar no Enter, tal como é indicado (mais se o utlizador tiver escrito 0 antes de dar Enter o while n irá correr outra vez)
  read choosen # é lida a variável choosen para caso seja introduzido 0 o menu saia e fazer uma pausa antes de dar clear
  clear # simples

done

echo -n "A sair do menu..." # simples
      sleep .5 # simples # pausa no código para ser vísivel a string anterior
clear # simples