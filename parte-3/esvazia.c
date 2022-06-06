#include "common.h" 
#include "utils.h"
#include <sys/msg.h>

int main() {
    int id, status;
    id = msgget( IPC_KEY, 0 );
    exit_on_error(id, "Erro no msgget.");
    printf("Estou a usar a fila de mensagens id=%d\n", id);
    
    MsgCliente m;

    status = msgrcv(id, &m, sizeof(m.dados), 1, 0);
    exit_on_error(status, "erro ao receber");
    printf("A mensagem foi:\n tipo: %ld \nnum_utente: %d \nnome: %s \nPID_cidadao: %d\n", m.tipo,m.dados.num_utente,m.dados.nome,m.dados.PID_cidadao);

    return(0);
}
