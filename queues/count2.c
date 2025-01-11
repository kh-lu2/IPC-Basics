//Code by Katarzyna Ludwa

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>

int errno;

struct komunikat {
    long mtype;
};

void send(int msqid) {
    struct komunikat kom = {1};
    if (msgsnd(msqid, &kom, 0, 0) == -1) {
        perror("Błąd przy wysyłaniu komunikatu");
        exit(2);
    }
}

void rcv(int msqid) {
    struct komunikat kom;
    if (msgrcv(msqid, &kom, 0, 2, 0) == -1) {
        perror("Błąd przy odbieraniu komunikatu");
        exit(3);
    }
}

int main() {
    key_t key = ftok(".", 'B');
    int msqid = msgget(key, 0640 | IPC_CREAT);

    if (msqid == -1) {
        perror("Błąd przy tworzeniu/dolaczaniu do kolejki");
        return 1;
    }
    printf("Tu c2, moj identyfikator kolejki: %d\n", msqid);

    send(msqid);

    for (int i = 2; i < 20; i += 2) {
        rcv(msqid);

        /* poczatek sekcji krytycznej */
        for (int j = 0; j < i; j++) {
            printf("%d ", j);
            fflush(stdout);
        }
        printf("\n");
        sleep(1);
        /* koniec sekcji krytycznej */

        send(msqid);
    }

    return 0;
}
