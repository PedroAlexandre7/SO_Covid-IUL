/******************************************************************************
 ** ISCTE-IUL: Trabalho prático de Sistemas Operativos
 **
 ** Aluno: Nº:99184       Nome: Pedro Alexandre
 ** Nome do Módulo: cidadao.c
 ** Descrição/Explicação do Módulo: 
 ** Este módulo tem como função simular a chegada de um cidadão ao seu centro de saúde para iniciar a sua vacinação
 ** O funcionamento deste módulo é descrito ao longo do mesmo.
 **
 ******************************************************************************/
#include "common.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void cria_cidadao();
void adiciona_cidadao();
void handlerSIGINT();
void ask_permission();
void handlerSIGUSR1();
void handlerSIGUSR2();
void handlerSIGTERM();

Cidadao cidadao; //estrutura que contém as informações de um cidadão

int main () {

    cria_cidadao();
    adiciona_cidadao();
    signal(SIGINT, handlerSIGINT);
    ask_permission();
    signal(SIGUSR1, handlerSIGUSR1);
    signal(SIGUSR2, handlerSIGUSR2);
    signal(SIGTERM, handlerSIGTERM);
    
    for(;;) // ciclo que garante que o processo só termina pela "ativação" dos sinais SIGINT, SIGUSR2 e SIGTERM
        pause();

}




void cria_cidadao(){ // função responsável por definir os atributos da estrutura cidadão
    printf("Insira o número de utente: ");
    scanf("%d", &cidadao.num_utente);
    printf("Insira o nome:             ");
    my_gets(cidadao.nome, 100);
    printf("Insira a idade:            ");
    scanf("%d", &cidadao.idade);
    printf("Insira a localidade:       ");
    my_gets(cidadao.localidade, 100);
    printf("Insira o nº de telemóvel:  ");
    my_gets(cidadao.nr_telemovel, 100);

    sucesso("C1) Dados Cidadão: %d; %s; %d; %s; %s; 0",cidadao.num_utente,cidadao.nome,cidadao.idade,cidadao.localidade,cidadao.nr_telemovel);

    cidadao.PID_cidadao = getpid(); // linha responsável por definir o pid do cidadao para posterior identificação do processo/cidadao
    sucesso("C2) PID Cidadão: %d",cidadao.PID_cidadao);
}

void adiciona_cidadao(){ // função responsável por guardar os atributos para posterior uso do servidor num ficheiro
    if(!access(FILE_PEDIDO_VACINA, F_OK)){ // verifica se o ficheiro pedidovacina.txt pode ser acedido, caso sim o processo encerra pois este provavelmente encontra-se a ser usado pelo servidor
        erro("C3) Não é possível iniciar o processo de vacinação neste momento");
        exit(1);
    }else{ 
        sucesso("C3) Ficheiro FILE_PEDIDO_VACINA pode ser criado");
        FILE *pedido; // inicia a estrutura que vai permitir criar e aceder ao ficheiro de contacto com o servidor
        pedido = fopen(FILE_PEDIDO_VACINA, "w");
        if(pedido){ // verifica se o ficheiro pedidovacina.txt foi criado para poder guardar os atributos do cidadao
            fprintf(pedido, "%d:%s:%d:%s:%s:0:%d",cidadao.num_utente,cidadao.nome,cidadao.idade,cidadao.localidade,cidadao.nr_telemovel,cidadao.PID_cidadao); // acrescenta os atributos do cidadão ao ficheiro mencionado
            fclose(pedido);
            sucesso("C4) Ficheiro FILE_PEDIDO_VACINA criado e preenchido");
        } else
            erro("C4) Não é possível criar o ficheiro FILE_PEDIDO_VACINA");
    }
}

void handlerSIGINT(){ // handler responsável por encerrar a vacinação do cidadao. este remove o ficheiro com as informções que o servidor utiliza e encerra o processo do cidadao

    sucesso("C5) O cidadão cancelou a vacinação, o pedido nº %d foi cancelado", cidadao.PID_cidadao);
    remove(FILE_PEDIDO_VACINA);
    exit(0);
}

void ask_permission(){ // função responsável contactar o servidor sobre um possivel cidadao a vacinar
    if(access(FILE_PID_SERVIDOR, F_OK)){ // verifica se o ficheiro servidor.pid pode ser acedido.
        erro("C6) Não existe ficheiro FILE_PID_SERVIDOR!");
    }
    else{ // segmento de código que, abre o ficheiro com o pid do servidor e usa-o para enviar um sinal de um possivel cidadao a vacinar
        FILE *fserverpid; 
        fserverpid = fopen(FILE_PID_SERVIDOR, "r");
        char serverPid[8];
        my_fgets(serverPid,8,fserverpid);
        kill(atoi(serverPid),SIGUSR1);
        sucesso("C6) Sinal enviado ao Servidor: %s", serverPid);
    }
}

void handlerSIGUSR1(){ // handler que remove o ficheiro pedidovacina.txt quando o servidor já não o necessita
    sucesso("C7) Vacinação do cidadão com o pedido nº %d em curso", cidadao.PID_cidadao);
    remove(FILE_PEDIDO_VACINA);
}

void handlerSIGUSR2(){ // handler que termina o processo quando a vacinação se encontra terminada
    sucesso("C8) Vacinação do cidadão com o pedido nº %d concluida", cidadao.PID_cidadao);
    exit(0);
}

void handlerSIGTERM(){ // handler utilizado quando por alguma razão já não é possivel realizar o pedido ou a vacinção, como por exemplo qdo não existem enfermeiros/vagas disponíveis, qdo o servidor filho responsável pela vacinação do cidadao termina, ou qdo o servidor encerra
    sucesso("C9) Não é possível vacinar o cidadão no pedido nº %d", cidadao.PID_cidadao);
    remove(FILE_PEDIDO_VACINA);
    exit(0);
}


