//Code by Katarzyna Ludwa

#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

int errno;

struct komunikat {
    long mtype;
};

void send(int msqid) {
    struct komunikat kom = {2};
    if (msgsnd(msqid, &kom, 0, 0) == -1) {
        perror("Błąd przy wysyłaniu komunikatu");
        exit(2);
    }
}

void rcv(int msqid) {
    struct komunikat kom;
    if (msgrcv(msqid, &kom, 0, 1, 0) == -1) {
        perror("Błąd przy odbieraniu komunikatu");
        exit(3);
    }
}

int main() {
    key_t key = ftok(".", 'B');
    int msqid = msgget(key, 0640 | IPC_CREAT);

    if (msqid == -1) {
        perror("Błąd przy tworzeniu/dołączaniu do kolejki");
        return 1;
    }

    printf("Tu c1, mój identyfikator kolejki: %d\n", msqid);

    //send(msqid);

    for (int i = 1; i < 20; i += 2) {
        rcv(msqid);

        /* początek sekcji krytycznej */
        for (int j = 0; j < i; j++) {
            printf("%d ", i);
            fflush(stdout);
        }
        sleep(1);
        printf("\n");
        /* koniec sekcji krytycznej */

        send(msqid);
    }

    struct msqid_ds buf;

    if (msgctl(msqid, IPC_RMID, &buf) == -1) {
        perror("Błąd przy usuwaniu kolejki");
        return 4;
    }
    printf("Usunięto kolejkę\n");

    return 0;
}
