/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos
 **
 ** Aluno: Pedro Alexandre Nº: 99184      Nome: Pedro Alexandre
 ** Nome do Módulo: servidor.c v3
 ** Descrição/Explicação do Módulo: 
 ** O funcionamento deste módulo é descrito ao longo do mesmo.
 **
 ******************************************************************************/
#include "common.h"
#include "utils.h"
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>

/* Variáveis globais */
int msg_id;             // ID da Fila de Mensagens IPC usada
int sem_id;             // ID do array de Semáforos IPC usado
int shm_id;             // ID da Memória Partilhada IPC usada
Database* db;           // Database utilizada, que estará em Memória Partilhada
MsgCliente mensagem;    // Variável que tem a mensagem enviada do Cidadao para o Servidor
MsgServidor resposta;   // Variável que tem a mensagem de resposta enviadas do Servidor para o Cidadao
int vaga_ativa;         // Índice da BD de Vagas que foi reservado pela função reserva_vaga()
int PID_pai;

/* Protótipos de funções */
void init_ipc();                    
void init_database();               
void espera_mensagem_cidadao();     
void trata_mensagem_cidadao();      
void envia_resposta_cidadao();      
void processa_pedido();             
void vacina();                      
void cancela_pedido();              
void servidor_dedicado();           
int reserva_vaga(int, int);         
void liberta_vaga(int);             
void termina_servidor(int);         
void termina_servidor_dedicado(int);
void sem_mutex_up();
void sem_mutex_down();

int main() {
    signal(SIGINT, termina_servidor);   // Arma o sinal SIGINT para que se receber <CTRL+C>, chama a função que termina o Servidor
    signal(SIGCHLD, SIG_IGN);
    // S1) Chama a função init_ipc(), que tenta criar uma fila de mensagens que tem a KEY IPC_KEY definida em common.h (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas). Deve assumir que a fila de mensagens já foi criada. Se tal não aconteceu, dá erro e termina com exit status 1. Esta função, em caso de sucesso, preenche a variável global msg_id;
    init_ipc();
    // S2) Chama a função init_database(), que inicia a base de dados
    init_database();
    while (TRUE) {
         // S3) Chama a função espera_mensagem_cidadao(), que espera uma mensagem (na fila de mensagens com o tipo = 1) e preenche a mensagem enviada pelo processo Cidadão na variável global mensagem; em caso de erro, termina com erro e exit status 1;
        espera_mensagem_cidadao();
         // S4) O comportamento do processo Servidor agora irá depender da mensagem enviada pelo processo Cidadão no campo pedido:
        trata_mensagem_cidadao();
    }
}

/**
 * S1) Chama a função init_ipc(), que tenta criar:
 *     • uma fila de mensagens IPC;
 *     • um array de semáforos IPC de dimensão 1;
 *     • uma memória partilhada IPC de dimensão suficiente para conter um elemento Database.
 *     Todos estes elementos têm em comum serem criados com a KEY IPC_KEY definida em common.h (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas), e com permissões 0600. Se qualquer um destes elementos IPC já existia anteriormente, dá erro e termina com exit status 1. Esta função, em caso de sucesso, preenche as variáveis globais respetivas msg_id, sem_id, e shm_id;
 *     O semáforo em questão será usado com o padrão “Mutex”, pelo que será iniciado com o valor 1;
 */
void init_ipc() {
    debug("<");

    PID_pai=getpid(); //guarda o pid do pai para caso seja dado um sinal SIGINT num processo filho (ao fazer Ctrl+c no servidor principal no moba são enviados SIGINT's para os servidores todos)

    // S1) Tenta criar:
    // Todos estes elementos têm em comum serem criados com a KEY IPC_KEY definida em common.h (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas), e com permissões 0600. Se qualquer um destes elementos IPC já existia anteriormente, dá erro e termina com exit status 1. Esta função, em caso de sucesso, preenche as variáveis globais respetivas msg_id, sem_id, e shm_id;
    // • uma fila de mensagens IPC;
    msg_id = msgget( IPC_KEY, IPC_CREAT | IPC_EXCL | 0600 );
    exit_on_error(msg_id, "S1) Fila de Mensagens com a Key definida já existe ou não pode ser criada");

    // • um array de semáforos IPC de dimensão 1;
    sem_id = semget(IPC_KEY, 1, IPC_CREAT | IPC_EXCL | 0600 );
    exit_on_error(sem_id, "S1) Semáforo com a Key definida já existe ou não pode ser criada");
    
    // O semáforo em questão será usado com o padrão “Mutex”, pelo que será iniciado com o valor 1;
    int status = semctl(sem_id, 0, SETVAL, 1);
    exit_on_error(status, "S1) Semáforo com a Key definida não pode ser iniciado com o valor 1");

    // • uma memória partilhada IPC de dimensão suficiente para conter um elemento Database.
    shm_id = shmget(IPC_KEY,sizeof(Database), IPC_CREAT | 0600);
    exit_on_error(shm_id, "S1) Memória Partilhada com a Key definida já existe ou não pode ser criada");

    sucesso("S1) Criados elementos IPC com a Key 0x%x: MSGid %d, SEMid %d, SHMid %d", IPC_KEY, msg_id, sem_id, shm_id);

    debug(">");
}

/**
 * Lê um ficheiro binário
 * @param   filename    Nome do ficheiro a ler
 * @param   buffer      Ponteiro para o buffer onde armazenar os dados
 * @param   maxsize     Tamanho máximo do ficheiro a ler
 * @return              Número de bytes lidos, ou 0 em caso de erro
 */
int read_binary(char* filename, void* buffer, const size_t maxsize) {
    struct stat st;
    // A função stat() preenche uma estrutura com dados do ficheiro, incluindo o tamanho do ficheiro.
    // stat() retorna -1 se erro
    exit_on_error(stat(filename, &st), "Erro no cálculo do tamanho do ficheiro");
    // O tamanho do ficheiro é maior do que o tamanho do buffer alocado?
    if (st.st_size > maxsize)
        exit_on_error(-1, "O buffer não tem espaço para o ficheiro");

    FILE* f = fopen(filename, "r");
    // fopen retorna NULL se erro
    exit_on_null(f, "Erro na abertura do ficheiro");

    // fread está a ler st.st_size elementos, logo retorna um valor < st.st_size se erro
    if (fread(buffer, 1, st.st_size, f) < st.st_size)
        exit_on_error(-1, "Erro na leitura do ficheiro");

    fclose(f);
    return st.st_size; // retorna o tamanho do ficheiro
}

/**
 * Grava um ficheiro binário
 * @param   filename    Nome do ficheiro a escrever
 * @param   buffer      Ponteiro para o buffer que contém os dados
 * @param   size        Número de bytes a escrever
 * @return              Número de bytes escrever, ou 0 em caso de erro
 */
int save_binary(char* filename, void* buffer, const size_t size) {
    FILE* f = fopen(filename, "w");
    // fopen retorna NULL se erro
    exit_on_null(f, "Erro na abertura do ficheiro");
   
    // fwrite está a escrever size elementos, logo retorna um valor < size se erro
    if (fwrite(buffer, 1, size, f) < size)
        exit_on_error(-1, "Erro na escrita do ficheiro");

    fclose(f);
    return size;
}

/**
 * S2) Inicia a base de dados:
 *     • Associa a variável global db com o espaço de Memória Partilhada alocado para shm_id; se não o conseguir, dá erro e termina com exit status 1;
 *     • Lê o ficheiro FILE_CIDADAOS e armazena o seu conteúdo na base de dados usando a função read_binary(), assim preenchendo os campos db->cidadaos e db->num_cidadaos. Se não o conseguir, dá erro e termina com exit status 1;
 *     • Lê o ficheiro FILE_ENFERMEIROS e armazena o seu conteúdo na base de dados usando a função read_binary(), assim preenchendo os campos db->enfermeiros e db->num_enfermeiros. Se não o conseguir, dá erro e termina com exit status 1;
 *     • Inicia o array db->vagas, colocando todos os campos de todos os elementos com o valor -1.
 */
void init_database() {
    debug("<");

    // S2) Inicia a base de dados:
    // • Associa a variável global db com o espaço de Memória Partilhada alocado para shm_id; se não o conseguir, dá erro e termina com exit status 1;

    db = (Database *) shmat(shm_id,NULL,0);
    exit_on_null(db, "S2) Erro a ligar a Memória Dinâmica ao projeto");

    // • Lê o ficheiro FILE_CIDADAOS e armazena o seu conteúdo na base de dados usando a função read_binary(), assim preenchendo os campos db->cidadaos e db->num_cidadaos. Se não o conseguir, dá erro e termina com exit status 1;
    db->num_cidadaos = read_binary(FILE_CIDADAOS,db->cidadaos,sizeof(db->cidadaos))/sizeof(Cidadao);
    if(!db->num_cidadaos)
        debug("O ficheiro FILE_CIDADAOS está vazio");

    // • Lê o ficheiro FILE_ENFERMEIROS e armazena o seu conteúdo na base de dados usando a função read_binary(), assim preenchendo os campos db->enfermeiros e db->num_enfermeiros. Se não o conseguir, dá erro e termina com exit status 1;
    db->num_enfermeiros = read_binary(FILE_ENFERMEIROS,db->enfermeiros,sizeof(db->enfermeiros))/sizeof(Enfermeiro);
    if(!db->num_enfermeiros)
        debug("O ficheiro FILE_ENFERMEIROS está vazio");

    // • Inicia a Base de Dados de Vagas, db->vagas, colocando o campo index_cidadao de todos os elementos com o valor -1.
    for(int i = 0; i < MAX_VAGAS; i++)
        db->vagas[i].index_cidadao = -1;

    sucesso("S2) Base de dados carregada com %d cidadãos e %d enfermeiros", db->num_cidadaos, db->num_enfermeiros);

    debug(">");
}

/**
 * Espera uma mensagem (na fila de mensagens com o tipo = 1) e preenche a variável global mensagem, assim como preenche o tipo da resposta com o PID_cidadao recebido.
 * Em caso de erro, termina com erro e exit status 1;
 */
void espera_mensagem_cidadao() {
    debug("<");

    // Espera uma mensagem (na fila de mensagens com o tipo = 1) e preenche a variável global mensagem,
    // assim como preenche o tipo da resposta com o PID_cidadao recebido.
    int status = msgrcv(msg_id, &mensagem, sizeof(mensagem.dados), 1, 0);
    exit_on_error(status, "Não é possível ler a mensagem do Cidadao");
    resposta.tipo=mensagem.dados.PID_cidadao;
    sucesso("Cidadão enviou mensagem");

    debug(">");
}

/**
 * Tratamento das mensagens do Cidadao
 */
void trata_mensagem_cidadao() {
    debug("<");

    // S4) O comportamento do processo Servidor agora irá depender da variável global mensagem enviada pelo processo Cidadão no campo pedido

    // S4.1) Se o pedido for PEDIDO, imprime uma mensagem e avança para o passo S5;
    if(mensagem.dados.pedido==PEDIDO){
        sucesso("S4.1) Novo pedido de vacinação de %d: %d, %s", mensagem.dados.PID_cidadao, mensagem.dados.num_utente, mensagem.dados.nome);
        processa_pedido();
    } else {
        // S4.2) Se o estado for CANCELAMENTO, imprime uma mensagem, e avança para o passo S10;
        sucesso("S4.2) Cancelamento de vacinação de %d: %d, %s", mensagem.dados.PID_cidadao, mensagem.dados.num_utente, mensagem.dados.nome);
        cancela_pedido();
    }

    debug(">");
}

/**
 * Estando a mensagem de resposta do processo Servidor na variável global resposta, envia essa mensagem para a fila de mensagens com o tipo = PID_Cidadao 
 */
void envia_resposta_cidadao() {
    debug("<");

    // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
    // exit_on_error(<var>, "Não é possível enviar resposta para o cidadão");
    // sucesso("Resposta para o cidadão enviada");
    int status = msgsnd(msg_id, &resposta, sizeof(resposta.dados), 0);
    exit_on_error(status, "Não é possível enviar resposta para o cidadão");
    sucesso("Resposta para o cidadão enviada");

    debug(">");
}

/**
 * S5) Processa um pedido de vacinação e envia uma resposta ao processo Cidadão. Para tal, essa função faz vários checks, atualizando o campo status da resposta:
 */
void processa_pedido() {
    debug("<");

    
    /*inteiros também usados para reservar a vaga*/
    int i_c; //index_cidadao
    int i_e; //index_enfermeiro

    // S5.1) Procura o num_utente e nome na base de dados (BD) de Cidadãos:
    resposta.dados.status=OK; //reinicializar o status a 0
    //sem_mutex_down que garante que a memória partilhada não é acedida até que todos os passos importantes para a agendação do cidadao sejam concluidos
    sem_mutex_down(); //este sem_mutex_down e os próximos sem_mutex_up's garantem que não seja acedida à memória partilhada até que o fique alterada a disponibilidade do enfermeiro para a enventualidade de este ficar reservado (pois, se o servidor pai correr esta função antes que o filho diga que o enfermeiro está indiponivel, o mesmo será agendado para duas vacinações)

    for(i_c = 0; i_c < db->num_cidadaos; i_c++)
        if(db->cidadaos[i_c].num_utente==mensagem.dados.num_utente){
            if(strncmp(db->cidadaos[i_c].nome, mensagem.dados.nome, 99)){ //garante que o numero e nome estão na base de dados
                i_c=db->num_cidadaos; //deste modo o status vai ser desconhecido
                break;
            }
            //       • Se o utilizador (Cidadão) for encontrado na BD Cidadãos, os dados do cidadão deverão ser copiados da BD Cidadãos para o campo cidadao da resposta;
            resposta.dados.cidadao = db->cidadaos[i_c];
            break;
        }
    //       • Se o utilizador (Cidadão) não for encontrado na BD Cidadãos => status = DESCONHECIDO;
    if(i_c==db->num_cidadaos){
        resposta.dados.status=DESCONHECIDO;
        erro("S5.1) Cidadão %d, %s  não foi encontrado na BD Cidadãos", mensagem.dados.num_utente, mensagem.dados.nome);
    } else { // caso n seja desconhecido
        //       • Se o Cidadão na BD Cidadãos tiver estado_vacinacao = 2 => status = VACINADO;
        if (resposta.dados.cidadao.estado_vacinacao >= 2) //nao testado!
            resposta.dados.status=VACINADO;
        //       • Se o Cidadão na BD Cidadãos tiver PID_cidadao > 0 => status = EMCURSO; caso contrário, 
        if (resposta.dados.cidadao.PID_cidadao > 0) //nao testado!
            resposta.dados.status=EMCURSO;

        sucesso("S5.1) Cidadão %d, %s encontrado, estado_vacinacao=%d, status=%d", resposta.dados.cidadao.num_utente, resposta.dados.cidadao.nome, resposta.dados.cidadao.estado_vacinacao, resposta.dados.status);
        if(resposta.dados.status==OK){ // afeta o PID_cidadao da BD Cidadãos com o valor do PID_cidadao da mensagem;
            db->cidadaos[i_c].PID_cidadao=mensagem.dados.PID_cidadao;

            // S5.2) Caso o Cidadão esteja em condições de ser vacinado (i.e., se status não for DESCONHECIDO, VACINADO nem EMCURSO), procura o enfermeiro correspondente na BD Enfermeiros:
            char cscidadao[100]; //centro saúde correspondente à localidade do cidadao
            snprintf( cscidadao, 99, "CS%s", resposta.dados.cidadao.localidade);
            for(i_e = 0; i_e < db->num_enfermeiros; i_e++)
                if(!strncmp(db->enfermeiros[i_e].CS_enfermeiro,cscidadao,99))
                    break;

            if(i_e==db->num_enfermeiros){ //       • Se não houver centro de saúde, ou não houver nenhum enfermeiro no centro de saúde correspondente => status = NAOHAENFERMEIRO;
                resposta.dados.status=NAOHAENFERMEIRO;
                erro("S5.2) Enfermeiro do CS %s não foi encontrado na BD Cidadãos", resposta.dados.cidadao.localidade);
            } else {
                if(!db->enfermeiros[i_e].disponibilidade){ //       • Se há enfermeiro, mas este não tiver disponibilidade => status = AGUARDAR.
                    resposta.dados.status=AGUARDAR;
                    sucesso("S5.2) Enfermeiro do CS %s encontrado, disponibilidade=%d, status=%d", resposta.dados.cidadao.localidade, db->enfermeiros[i_e].disponibilidade, resposta.dados.status);
                } else {
                    sucesso("S5.2) Enfermeiro do CS %s encontrado, disponibilidade=%d, status=%d", resposta.dados.cidadao.localidade, db->enfermeiros[i_e].disponibilidade, resposta.dados.status);
                    // S5.3) Caso o enfermeiro esteja disponível, procura uma vaga para vacinação na BD Vagas. Para tal, chama a função reserva_vaga(Index_Cidadao, Index_Enfermeiro) usando os índices do Cidadão e do Enfermeiro nas respetivas BDs:
                    //      • Se essa função tiver encontrado e reservado uma vaga => status = OK;
                    //      • Se essa função não conseguiu encontrar uma vaga livre => status = AGUARDAR.
                    if(reserva_vaga(i_c, i_e)<0){
                        resposta.dados.status=AGUARDAR;
                        erro("S5.3) Não foi encontrada nenhuma vaga livre para vacinação, status=%d", resposta.dados.status);
                    } else //o status irá se manter-se OK se não foi alterado
                        sucesso("S5.3) Foi reservada a vaga %d para vacinação, status=%d", vaga_ativa, resposta.dados.status);
                }
            }
        }
    }
    

    // S5.4) Se no final de todos os checks, status for OK, chama a função vacina(),
    if(resposta.dados.status==OK)
        vacina();
    else { // S5.4) caso contrário, atualiza o PID_cidadao=-1 na BD de Cidadãos e chama a função envia_resposta_cidadao(), que envia a resposta de erro ao Cidadão;
        if(resposta.dados.status==NAOHAENFERMEIRO || resposta.dados.status==AGUARDAR) //garante que seja acedido aos cidadãos quando estes foram alterados para evitar que o PID_cidadao seja alterado quando não deve, ou quando o status=EMCURSO
            db->cidadaos[i_c].PID_cidadao=-1;
        sem_mutex_up(); //sem_mutex_up que liberta a memória pois esta já não é precisa para o tratamento deste cidadão
        envia_resposta_cidadao();
        
    }
    debug(">");
}

/**
 * S6) Processa a vacinação
 */
void vacina() {
    debug("<");

    // S6.1) Cria um processo filho através da função fork();
    int PID_filho = fork();
    if(PID_filho<-1){
        sem_mutex_up();//sem_mutex_up que liberta a memória pois esta já não é precisa para o tratamento deste cidadão (um SIGTERM aos filhos iria complementar o uso desta função)
        
        exit_on_error(PID_filho, "S6.1) Não foi possível criar um novo processo");
    }
    
    if (!PID_filho) {   // Processo FILHO
        // S6.2) O processo filho chama a função servidor_dedicado();
        servidor_dedicado();
    } else {     // Processo PAI
        // S6.3) O processo pai regista o process ID do processo filho no campo PID_filho na BD de Vagas com o índice da variável global vaga_ativa;
        db->vagas[vaga_ativa].PID_filho=PID_filho;
        sucesso("S6.1) Criado um processo filho com PID_filho=%d",PID_filho);
    }

    debug(">");
}

/**
 * S7) Servidor Dedicado
 */
void servidor_dedicado() {
    debug("<");

    // S7.1) Arma o sinal SIGTERM;
    signal(SIGTERM, termina_servidor_dedicado);

    // S7.2) Envia a resposta para o Cidadao, chamando a função envia_resposta_cidadao(). Implemente também esta função, que envia a mensagem resposta para o cidadao, contendo os dados do Cidadao preenchidos em S5.1 e o campo status = OK;
    envia_resposta_cidadao();

    // S7.3) Coloca a disponibilidade do enfermeiro afeto à vaga_ativa com o valor 0 (Indisponível);
    db->enfermeiros[db->vagas[vaga_ativa].index_enfermeiro].disponibilidade=0;
    sucesso("S7.3) Enfermeiro associado à vaga %d indisponível", vaga_ativa);

    // S7.4) Imprime uma mensagem;
    // S7.4) Aguarda (em espera passiva!) TEMPO_CONSULTA segundos;
    sucesso("S7.4) Vacina em curso para o cidadão %d, %s, e com o enfermeiro %d, %s na vaga %d", resposta.dados.cidadao.num_utente, resposta.dados.cidadao.nome, db->enfermeiros[db->vagas[vaga_ativa].index_enfermeiro].ced_profissional, db->enfermeiros[db->vagas[vaga_ativa].index_enfermeiro].nome, vaga_ativa);
    //este sem_mutex_up e o anterior sem_mutex_down garantem que não seja acedida à memória partilhada até que o fique alterada a disponibilidade do enfermeiro para a enventualidade de este ficar reservado (pois, se o servidor pai correr esta função antes que o filho diga que o enfermeiro está indiponivel, o mesmo será agendado para duas vacinações)
    sem_mutex_up(); //e que as linhas anteriores que utilizaram a base de dados, tenham exclusividade à mesma

    sleep(TEMPO_CONSULTA);

    // S7.5) Envia nova resposta para o Cidadao, chamando a função envia_resposta_cidadao() contendo os dados do Cidadao preenchidos em S5.1 e o campo status = TERMINADA, para indicar que a consulta terminou com sucesso;
    resposta.dados.cidadao.estado_vacinacao++;
    resposta.dados.status=TERMINADA;
    envia_resposta_cidadao();


    // S7.6) Atualiza os dados do cidadão (incrementa estado_vacinacao) na BD de Cidadãos
    // S7.6) Atualiza os dados do cidadão (PID_cidadao = -1) na BD de Cidadãos
    sem_mutex_down(); //este sem_mutex_down e o próximo sem_mutex_up garantem que a base de dados na memoria partilhada não seja acedida por outro processo até que este façam as alterações necessárias seguintes que são importantes para serem feitas juntas para terminar o registo da vacinação do cidadao
    db->cidadaos[db->vagas[vaga_ativa].index_cidadao].PID_cidadao = -1;
    // S7.6) Atualiza os dados do enfermeiro (incrementa nr_vacinas_dadas) na BD de Enfermeiros;
    // S7.6) Atualiza os dados do enfermeiro (coloca disponibilidade=1) na BD de Enfermeiros;
    sucesso("S7.6) Cidadão atualizado na BD para estado_vacinacao=%d, Enfermeiro atualizado na BD para nr_vacinas_dadas=%d e disponibilidade=%d", ++db->cidadaos[db->vagas[vaga_ativa].index_cidadao].estado_vacinacao, ++db->enfermeiros[db->vagas[vaga_ativa].index_enfermeiro].nr_vacinas_dadas, ++db->enfermeiros[db->vagas[vaga_ativa].index_enfermeiro].disponibilidade);

    // S7.7) Liberta a vaga vaga_ativa da BD de Vagas, invocando a função liberta_vaga(vaga_ativa);
    liberta_vaga(vaga_ativa);
    sem_mutex_up(); //funcionalidade descrita no sem_mutex_down anterior

    // S7.8) Termina o processo Servidor Dedicado (filho) com exit status 0.
    sucesso("S7.8) Servidor dedicado Terminado"); 
    
    debug(">");
    exit(0);
}

/**
 * S8) Tenta reservar uma vaga livre na BD de Vagas
 */
int reserva_vaga(int index_cidadao, int index_enfermeiro) {
    debug("<");

    // S8.1) Procura uma vaga livre (index_cidadao < 0) na BD de Vagas. Se encontrar uma entrada livre:
    for(vaga_ativa = 0; vaga_ativa < MAX_VAGAS; vaga_ativa++)
        // S8.1.1) Atualiza o valor da variável global vaga_ativa com o índice da vaga encontrada;
        if(db->vagas[vaga_ativa].index_cidadao==-1)
        break;
    if(vaga_ativa==MAX_VAGAS) //Retorna o valor de -1 se não encontrou nenhuma vaga
        return -1;

    sucesso("S8.1.1) Encontrou uma vaga livre com o index %d", vaga_ativa);

    // S8.1.2) Atualiza a entrada de Vagas vaga_ativa com o índice do cidadão e do enfermeiro
    db->vagas[vaga_ativa].index_cidadao=index_cidadao;
    db->vagas[vaga_ativa].index_enfermeiro=index_enfermeiro;

    // S8.1.3) Retorna o valor do índice de vagas vaga_ativa
    return vaga_ativa;

    debug(">");
}

/**
 * S9) Liberta a vaga da BD de Vagas, colocando o campo index_cidadao dessa entrada da BD de Vagas com o valor -1
 */
void liberta_vaga(int index_vaga) {
    debug("<");

    db->vagas[vaga_ativa].index_cidadao=-1;
    sucesso("S9) A vaga com o index %d foi libertada", index_vaga);
    
    debug(">");
}

/**
 * Ações quando o servidor processa um pedido de cancelamento do Cidadao
 */
void cancela_pedido() {
    debug("<");

    // S10) Processa o cancelamento de um pedido de vacinação e envia uma resposta ao processo Cidadão. Para este efeito, a função:
    // S10.1) Procura na BD de Vagas a vaga correspondente ao Cidadao em questão (procura por index_cidadao). Se encontrar a entrada correspondente, obtém o PID_filho do Servidor Dedicado correspondente;
    int i;
    int PID_filho;
    sem_mutex_down(); //este sem_mutex_down e proximo sem_mutex_up garantem que a memória partilhada não é acedida por outro processo até que este termine de verificar se o vacinação do cidadao está em curso
    for(i = 0;  i < MAX_VAGAS;  i++)
        if(db->vagas[i].index_cidadao!=-1)
            if(db->cidadaos[db->vagas[i].index_cidadao].num_utente==mensagem.dados.num_utente){
                PID_filho = db->vagas[i].PID_filho;
                break;
            }
    sem_mutex_up(); //funcionalidade descrita no sem_mutex_down anterior
    if(i==MAX_VAGAS)
        erro("S10.1) Não foi encontrada nenhuma sessão do cidadão %d, %s", mensagem.dados.num_utente, mensagem.dados.nome);
    else {
        sucesso("S10.1) Foi encontrada a sessão do cidadão %d, %s na sala com o index %d", mensagem.dados.num_utente, mensagem.dados.nome, i);

    // S10.2) Envia um sinal SIGTERM ao processo Servidor Dedicado (filho) que está a tratar da vacinação;
    kill(PID_filho, SIGTERM);
    sucesso("S10.2) Enviado sinal SIGTERM ao Servidor Dedicado com PID=%d", PID_filho);
    }
    debug(">");
}

/**
 * Ações quando o servidor recebeu um <CTRL+C>
 */
void termina_servidor(int sinal) {
    debug("<");
    if( PID_pai == getpid() ){ //garante que não é executado o sinal SIGINT nos processos filhos, mas sim o SIGTERM que vai ser mandado pelo processo pai
        // S11) Implemente a função termina_servidor(), que irá tratar do fecho do servidor, e que:

        // S11.1) Envia um sinal SIGTERM a todos os processos Servidor Dedicado (filhos) ativos;
        if(semctl(sem_id, 0, GETVAL)) //devido à prioridade deste código, e ao facto de todos seguirem a mesma regra a partir do momento em que seja mandado o kill, este if tenta garantir que "seja fechada a memória" e que a mesma esteja disponivel
            sem_mutex_down(); //este sem_mutex_down e proximo sem_mutex_up garantem que a memória partilhada não é acedida por outro processo até que este termine de verificar quais os processos que devem levar SIGTERM para terminarem
        int count=0;
        for(int i = 0; i < MAX_VAGAS; i++)
            if(db->vagas[i].index_cidadao != -1){
                kill(db->vagas[i].PID_filho, SIGTERM);
                count++;
            }
        sem_mutex_up(); //funcionalidade descrita no sem_mutex_down anterior

        for(; count>0; count--) //tentativa de garantir que só sejam guardadas as infos e os ipc's sejam removidos dps das alterações dos filhos terem sido concluidas
            wait(NULL);

        // S11.2) Grava o ficheiro FILE_ENFERMEIROS, usando a função save_binary();
        save_binary(FILE_ENFERMEIROS,db->enfermeiros,sizeof(Enfermeiro)*db->num_enfermeiros);

        // S11.3) Grava o ficheiro FILE_CIDADAOS, usando a função save_binary();
        save_binary(FILE_CIDADAOS,db->cidadaos,sizeof(Cidadao)*db->num_cidadaos);

        // S11.4) Remove do sistema (IPC Remove) os semáforos, a Memória Partilhada e a Fila de Mensagens.
        msgctl(msg_id, IPC_RMID, NULL);
        semctl(sem_id, 0, IPC_RMID, NULL);
        shmctl(shm_id, IPC_RMID, NULL);
        sucesso("S11.4) Servidor Terminado");

        // S11.5) Termina o processo servidor com exit status 0.
        debug(">");
        
        exit(0);
    }
}

void termina_servidor_dedicado(int sinal) {
    debug("<");

    // S12) Implemente a função termina_servidor_dedicado(), que irá tratar do fecho do servidor dedicado, e que:

    // S12.1) Envia a resposta para o Cidadao, chamando a função envia_resposta_cidadao() com o campo status=CANCELADA, para indicar que a consulta foi cancelada;
    resposta.dados.status=CANCELADA;
    envia_resposta_cidadao();
    sem_mutex_down(); //impede que outro servidor dedicado altere a memória enquanto este é cancelado
    db->enfermeiros[db->vagas[vaga_ativa].index_enfermeiro].disponibilidade=1; //volta meter a disponibilidade do enfermeiro caso tenha sido posta a zero antes que o servidor guarde

    // S12.2) Liberta a vaga vaga_ativa da BD de Vagas, invocando a função liberta_vaga(vaga_ativa);
    liberta_vaga(vaga_ativa);
    sem_mutex_up(); // trabalha em conjunto com o sem_mutex_down anterior

    // S12.3) Termina o processo do servidor dedicado com exit status 0;
    sucesso("S12.3) Servidor Dedicado Terminado");
    debug(">");
    exit(0);
}

/**
 * Altera o semáforo definido para Mutex na aplicação: Lock
 */
void sem_mutex_down() {
    struct sembuf lower = { .sem_num = 0, .sem_op = -1 };
    exit_on_error(semop(sem_id, &lower, 1), "Mutex-Lock: Não foi possível atualizar o semáforo");
    debug("MUTEX: Locked");
}

/**
 * Altera o semáforo definido para Mutex na aplicação: Unlock
 */
void sem_mutex_up() {
    struct sembuf raise = { .sem_num = 0, .sem_op = +1 };
    exit_on_error(semop(sem_id, &raise, 1), "Mutex-Unlock: Não foi possível atualizar o semáforo");
    debug("MUTEX: Unlocked");
}