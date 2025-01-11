//Code by Katarzyna Ludwa

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int errno;

int P(int semid, int sem, int flag) {
    struct sembuf sb = {sem, -1, flag};
    if (flag)
        return semop(semid, &sb, 1);
    if (semop(semid, &sb, 1) == -1) {
        perror("Błąd przy opuszczaniu semafora");
        exit(7);
    }
}

void V(int semid, int sem) {
    struct sembuf sb = {sem, 1, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("Błąd przy podnoszeniu semafora");
        exit(8);
    }
}

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
} arg;

int main() {

    struct Dane {
        int x;
        char s[20];
    } *dane_ptr;

    key_t klucz1 = ftok(".", 'M');
    int shmID = shmget(klucz1, sizeof(struct Dane), IPC_CREAT | SHM_R | SHM_W | SHM_R >> 3);

    if (shmID == -1) {
        perror("Błąd utworzenia pamięci dzielonej");
        return 1;
    }

    printf("Identyfikator pamięci dzielonej: %d\n", shmID);
    sleep(1);

    dane_ptr = (struct Dane *) shmat(shmID, 0, 0);

    if (((long)dane_ptr) == -1) {
        perror("Błąd dołączania");
        return 2;
    }

    printf("Dołączono strukturę danych do obszaru pamięci dzielonej\n");
    sleep(1);

    key_t klucz2 = ftok(".", 'S');
    int semID = semget(klucz2, 3, 0640 | IPC_CREAT);
    if (semID == -1) {
        perror("Błąd przy dołączaniu/tworzeniu zbioru semaforów");
        return 3;
    }

    printf("Identyfikator zbioru semaforów: %d\n", semID);
    sleep(1);

    arg.val = 1;
    semctl(semID, 0, SETVAL, arg);
    arg.val = 0;
    semctl(semID, 1, SETVAL, arg);

    V(semID, 2);

    while (1) {
        P(semID, 1, 0);

        if (!P(semID, 2, IPC_NOWAIT))
            break;

        printf("\nZAWARTOŚĆ PAMIĘCI DZIELONEJ:\nLiczba: %d %s\n", dane_ptr->x, &dane_ptr->s);

        V(semID, 0);
    }

    if (shmdt((struct Dane *) dane_ptr) == -1) {
        perror("Błąd przy odłączaniu pamięci dzielonej");
        return 4;
    }
    printf("Pamięć dzielona odłączona\n");
    sleep(1);

    if (shmctl(shmID, IPC_RMID, 0) == -1) {
        perror("Błąd przy usuwaniu pamięci dzielonej");
        return 5;
    }
    printf("Pamięć dzielona skasowana\n");
    sleep(1);

    if (semctl(semID, IPC_RMID, 0) == -1) {
        perror("Błąd przy usuwaniu zbioru semaforów");
        return 6;
    }

    printf("Zbiór semaforów skasowany\n");

    return 0;
}