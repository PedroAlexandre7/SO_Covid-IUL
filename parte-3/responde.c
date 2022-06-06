#include "common.h" 
#include "utils.h"
#include <sys/msg.h>

int main() {
    int id = msgget( IPC_KEY, 0 );
    exit_on_error(id, "Erro no msgget.");

    MsgCliente m;
    int status0 = msgrcv(id, &m, sizeof(m.dados), 1, 0);
    exit_on_error(status0, "erro ao receber");

    MsgServidor mensagem;
    mensagem.tipo = m.dados.PID_cidadao;
    mensagem.dados.status = OK;
    mensagem.dados.cidadao.num_utente = 12345;
    strcpy(mensagem.dados.cidadao.nome,"Ana");
    mensagem.dados.cidadao.idade = 19;
    strcpy(mensagem.dados.cidadao.localidade, "Lisboa");
    strcpy(mensagem.dados.cidadao.nr_telemovel, "924931749");
    mensagem.dados.cidadao.estado_vacinacao = 0;
    int status = msgsnd(id, &mensagem, sizeof(mensagem.dados), 0);
    exit_on_error(status, "Não é possível enviar mensagem para o servidor");
    printf("enviado\n");
}