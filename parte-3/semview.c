#include "common.h"
#include "utils.h"
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>

int main() {
    int id = semget( IPC_KEY, 1, 0 );
    exit_on_error(id, "semget");

    int val = semctl(id, 0, GETVAL);
    printf("O valor do semáforo #0 é : %d\n", val);
}