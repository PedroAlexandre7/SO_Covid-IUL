/******************************************************************************
 ** ISCTE-IUL: Trabalho prático de Sistemas Operativos
 **
 ** Aluno: Nº:99184       Nome: Pedro Alexandre
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo: 
 **
 ** O funcionamento deste módulo é descrito ao longo do mesmo.
 **
 ******************************************************************************/
#include "common.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>


void cria_servidor();
void definir_espaco_enf();
void iniciar_lista();
void handlerSIGUSR1();
int info_pedido();
int enf_disponivel();
void preencher_vaga(int);
void criar_filho_retornar_espera();
void handlerSIGCHLD();
int libertar_vaga(int, int);
void atualizar_enf(int);
void handlerSIGTERM();
void handlerSIGINT();

Enfermeiro* enf; // estrutura que aponta para as informações dos enfermeiros em memória, alocadas numa função
int Nenf; // inteiro que guarda o número de enfermeiros guardados nas estutura acima
Cidadao cidadao; //estrutura que contém as informações de um cidadão, usado para preparar um vacinação se possivel. também é usado pelos servidores filhos mantendo a informação que estes precisam do cidadao responsaveis

Vaga vagas[NUM_VAGAS]; // vetor com as vagas que o servidor deve tratar simultaneamente
int currentvaga; // inteiro que guarda a vaga que se encontra a ser gerida



int main () {
    cria_servidor();
    definir_espaco_enf();
    iniciar_lista();
    
    signal(SIGUSR1, handlerSIGUSR1);
    signal(SIGINT, handlerSIGINT);
    for(;;){ // ciclo que garante que o processo só termina pela "ativação" do sinal SIGINT
        sucesso("S4) Servidor espera pedidos");
        pause();
        }
}

void cria_servidor(){ // função responsável por preparar as informações necessárias para o acesso ao servidor
    FILE *servidor; // inicia a estrutura que vai permitir criar e aceder ao ficheiro (servidor.pid) com as informações do servidor (pid)
        servidor = fopen(FILE_PID_SERVIDOR, "w");
        if(servidor){ // verifica se o ficheiro servidor.pid foi criado para poder guardar o pid do servidor
            int pid = getpid();
            fprintf(servidor,"%d", pid);
            fclose(servidor);
            sucesso("S1) Escrevi no ficheiro FILE_PID_SERVIDOR o PID: %d", pid);
        } else
            erro("S1) Não consegui registar o servidor!");
}

void definir_espaco_enf(){ // função responsável por alocar espaço para os enfermeiros e guardar as suas informações no servidor
    FILE *fenf; // inicia a estrutura que vai permitir aceder ao ficheiro de contacto entre o servidor e os processos dos cidadãos
        fenf = fopen(FILE_ENFERMEIROS, "rb"); // rb peremite a leitura de binários
        if(fenf){
            fseek (fenf, 0, SEEK_END); // fseek que coloca um apontador de uma estrutura no final do ficheiro enfermeiros.dat
            int size = ftell (fenf); // inteiro que diz em bytes onde se encontra o tal apontador face ao inicio do ficheiro
            Nenf = size/sizeof(Enfermeiro); // definição da variavel Nenf com o número de enfermeiros
            enf = malloc(size); // função que aloca o espaço que é necessário para guardar as informações dos enfermeiros
            fseek(fenf, 0, SEEK_SET); // reseta a posição do tal apontador para o inicio do ficheiro enfermeiros.dat
            for(int i = 0; i<Nenf;i++){
                fread(&enf[i], sizeof(Enfermeiro), 1, fenf); // função que iterativamente copia os dados do ficheiro gravando-os de forma correta para posterior acesso do ponteiro enf
            }
            fclose(fenf);
            sucesso("S2) Ficheiro FILE_ENFERMEIROS tem %d bytes, ou seja, %d enfermeiros",size,Nenf);
        } else
            erro("S2) Não consegui ler o ficheiro FILE_ENFERMEIROS!");
}

void iniciar_lista(){ // função que deixa o vetor vagas definido como vazio
    for(int i = 0; i<NUM_VAGAS;i++)
        vagas[i].index_enfermeiro = -1; // altera iterativamente caga vaga deixando o index dos enfermeiros a zero
    sucesso("S3) Iniciei a lista de %d vagas",NUM_VAGAS);
}
//
//

//
//
//
//
//
//

void handlerSIGUSR1(){ // handler que trata de uma entrada de cidadao, após o sinal que tal indica
    if(info_pedido()!=0) // if que confirma se existe necessidade de terminar "o tratamento do vacinação do cidadão"
    return;
    
    int index = enf_disponivel();
    if(index == -1) // if que confirma se existe necessidade de terminar "o tratamento do vacinação do cidadão"
    return;

    preencher_vaga(index);
    criar_filho_retornar_espera();
    
}


int info_pedido(){ // função que trata de transferir as informações do cidadao para a estrutura do mesmo local
    Cidadao c1; // passivel de ser removido
    cidadao = c1; // passivel de ser removido
    FILE *pedido; // inicia a estrutura que vai permitir aceder ao ficheiro com as informações do cidadao
    pedido = fopen(FILE_PEDIDO_VACINA, "r");
    if(pedido) { // verifica se o ficheiro pedidovacina.txt foi aberto
        if(access(FILE_PEDIDO_VACINA,R_OK)){ // verifica se o ficheiro pedidovacina.txt é passível de ser acedido
            erro("S5.1) Não foi possível ler o ficheiro FILE_PEDIDO_VACINA");
            return -1; //return diferente de 0 para que o handler termine
        } else { 
            fscanf(pedido, "%d:%[^:]:%d:%[^:]:%[^:]:%d:%d",&cidadao.num_utente,cidadao.nome,&cidadao.idade,cidadao.localidade,cidadao.nr_telemovel,&cidadao.estado_vacinacao,&cidadao.PID_cidadao); // função que acede à stream de dados do ficheiro pedidovacina.txt e guarda os conteúdos de uma estrutura cidadao
            fclose(pedido);
            sucesso("S5.1) Dados Cidadão: %d; %s; %d; %s; %s; %d",cidadao.num_utente,cidadao.nome,cidadao.idade,cidadao.localidade,cidadao.nr_telemovel,cidadao.estado_vacinacao);
            }
    } else {
        erro("S5.1) Não foi possível abrir o ficheiro FILE_PEDIDO_VACINA");
        return -1; //return diferente de 0 para que o handler termine
    }
    return 0; //return igual a 0 para que o handler continue a tratar
}

int enf_disponivel(){ //
    Enfermeiro enfdisp;
    char cscidadao[100];
    snprintf( cscidadao, 99, "CS%s", cidadao.localidade);
    int enffound= 0;
    int i;
    for(i = 0; i<Nenf;i++){
        if(!strncmp( enf[i].CS_enfermeiro, cscidadao, 99)) {
            enfdisp=enf[i];
            enffound = i;
            if( enf[i].disponibilidade==1)
                break;
        }
    }

    if(i == Nenf){
        if(enffound)
            erro("S5.2.1) Enfermeiro %d indisponível para o pedido %d para o Centro de Saúde %s",enffound,cidadao.PID_cidadao,enfdisp.CS_enfermeiro);
        else
            erro("S5.2.1) Nenhum enfermeiro registado para o Centro de Saúde %s", cscidadao);
        kill(cidadao.PID_cidadao,SIGTERM);
        return -1; //return igual a -1 para que o handler termine
    }

    sucesso("S5.2.1) Enfermeiro %d disponível para o pedido %d",enffound,cidadao.PID_cidadao);
    for(currentvaga = 0; currentvaga<NUM_VAGAS;currentvaga++)
        if(vagas[currentvaga].index_enfermeiro==-1)
            break;

    if(currentvaga>=NUM_VAGAS){
        kill(cidadao.PID_cidadao,SIGTERM);
        erro("S5.2.2) Não há vaga para vacinação para o pedido %d",cidadao.PID_cidadao);
        return -1; //return igual a -1 para que o handler termine
        }
    sucesso("S5.2.2) Há vaga para vacinação para o pedido %d",cidadao.PID_cidadao);
    return enffound;
}

void preencher_vaga(int index){
    vagas[currentvaga].cidadao = cidadao;
    vagas[currentvaga].index_enfermeiro = index;
    enf[index].disponibilidade=0;
    sucesso("S5.3) Vaga nº %d preenchida para o pedido %d",currentvaga, cidadao.PID_cidadao);
}

void criar_filho_retornar_espera(){
    int pidfilho = fork();
        
        if(pidfilho == -1){
            erro("S5.4) Não foi possível criar o servidor dedicado");
            return;
        }
        
        if(pidfilho == 0){
            signal(SIGTERM, handlerSIGTERM);
            kill(cidadao.PID_cidadao,SIGUSR1);
            sucesso("S5.6.2) Servidor dedicado inicia consulta de vacinação");
            sleep(TEMPO_CONSULTA);
            sucesso("S5.6.3) Vacinação terminada para o cidadão com o pedido nº %d", cidadao.PID_cidadao);
            kill(cidadao.PID_cidadao,SIGUSR2);
            sucesso("S5.6.4) Servidor dedicado termina consulta de vacinação");
            exit(0);
        } else {
            sucesso("S5.4) Servidor dedicado %d criado para o pedido %d",pidfilho,cidadao.PID_cidadao);
            vagas[currentvaga].PID_filho = pidfilho;
            sucesso("S5.5.1) Servidor dedicado %d na vaga %d",pidfilho,currentvaga);
            signal(SIGCHLD, handlerSIGCHLD);
            sucesso("S5.5.2) Servidor aguarda fim do servidor dedicado %d",pidfilho);
        }

}

//
//
//
//
//

void handlerSIGCHLD(){
    int pidfilho= wait(NULL);
    int i;
    for(i = 0; i<NUM_VAGAS; i++)
        if(vagas[i].PID_filho==pidfilho)
            break;
    int index = libertar_vaga(i, pidfilho);
    atualizar_enf(index);
    sucesso("S5.5.3.5) Retorna");
}

int libertar_vaga(int v, int pidfilho){
    int index = vagas[v].index_enfermeiro;
    Vaga v0;
    vagas[v] = v0;
    vagas[v].index_enfermeiro = -1;
    sucesso("S5.5.3.1) Vaga %d que era do servidor dedicado %d libertada",v, pidfilho);
    return index;
}

void atualizar_enf(int index){
    enf[index].disponibilidade=1;
    sucesso("S5.5.3.2) Enfermeiro %d atualizado para disponível",index);

    sucesso("S5.5.3.3) Enfermeiro %d atualizado para %d vacinas dadas",index,++enf[index].num_vac_dadas);

    FILE *fenf;
    fenf = fopen(FILE_ENFERMEIROS, "rb+");
    fseek(fenf,sizeof(Enfermeiro)*index+sizeof(int)+2*100*sizeof(char),SEEK_SET);
    fwrite(&enf[index].num_vac_dadas,sizeof(int),1,fenf);
    fclose(fenf);
    sucesso("S5.5.3.4) Ficheiro FILE_ENFERMEIROS %d atualizado para %d vacinas dadas",index,enf[index].num_vac_dadas);
}

//
//
//
//
//

void handlerSIGTERM(){
    sucesso("S5.6.1) SIGTERM recebido, servidor dedicado termina Cidadão");
    kill(cidadao.PID_cidadao,SIGTERM);
    exit(0);
}

void handlerSIGINT(){
    for(int i = 0; i<NUM_VAGAS;i++)
        if(vagas[i].index_enfermeiro!=-1)
            kill(vagas[i].PID_filho,SIGTERM);
    remove(FILE_PID_SERVIDOR);
    sucesso("S6) Servidor terminado");
    exit(0);
}