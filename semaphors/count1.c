//Code by Katarzyna Ludwa

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <errno.h>

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

    printf("Tu c1, mój identyfikator zbioru semaforów: %d\n", semid);

    P(semid, 2);

    for (int i = 1; i < 20; i += 2) {
        P(semid, 0);

        /* początek sekcji krytycznej */
        for (int j = 0; j < i; j++) {
            printf("%d ", j);
            fflush(stdout);
        }
        sleep(1);
        printf("\n");
        /* koniec sekcji krytycznej */

        V(semid, 1);
    }

    if (semctl(semid, IPC_RMID, 0) == -1) {
        perror("Błąd przy usuwaniu zbioru semaforów");
        return 4;
    }
    printf("Usunięto zbiór semaforów\n");

    return 0;
}