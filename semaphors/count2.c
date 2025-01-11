//Code by Katarzyna Ludwa

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdlib.h>

int errno;

void P(int semid, int sem) {
    struct sembuf sb = {sem, -1, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("Błąd przy opuszczaniu semafora");
        exit(2);
    }
}

void V(int semid, int sem) {
    struct sembuf sb = {sem, 1, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("Błąd przy podnoszeniu semafora");
        exit(3);
    }
}

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
} arg;

int main() {
    key_t key = ftok(".", 'A');
    int semid = semget(key, 3, 0640 | IPC_CREAT);
    if (semid == -1) {
        perror("Błąd przy tworzeniu/dołączaniu do zbioru semaforów");
        return 1;
    }
    printf("Tu c2, mój identyfikator zbioru semaforów: %d\n", semid);

    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);
    arg.val = 0;
    semctl(semid, 1, SETVAL, arg);

    V(semid, 2);

    for (int i = 2; i < 20; i += 2) {
        P(semid, 1);

        /* początek sekcji krytycznej */
        for (int j = 0; j < i; j++) {
            printf("%d ", j);
            fflush(stdout);
        }
        sleep(1);
        printf("\n");
        /* koniec sekcji krytycznej */

        V(semid, 0);
    }

    return 0;
}
