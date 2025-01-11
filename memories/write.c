//Code by Katarzyna Ludwa

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

int errno;

void P(int semid, int sem) {
    struct sembuf sb = {sem, -1, 0};
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

char *get_line() {
    char *str = NULL;
    size_t len = 0;
    int ptr = getline(&str, &len, stdin);
    str[ptr - 1] = '\0';
    return str;
}

int read_input(int *x2, char *s2) {
    while (1) {
        char *sx = get_line();

        if (!strcmp(sx, "koniec")) {
            free(sx);
            return 0;
        }

        char *endptr;
        errno = 0;
        long result = strtol(sx, &endptr, 10);
        if (sx[0] == '\0') {
            printf("Wpisz liczbe: \n");
        } else if (errno == ERANGE || result > INT_MAX || result < INT_MIN) {
            printf("Za duza lub za mala liczba\n");
        } else if (*endptr != '\0' && *endptr != '\n') {
            printf("To nie jest liczba\n");
        } else {
            *x2 = (int)(result);
            free(sx);
            break;
        }
        free(sx);
    }

    while (1) {
        char *sx = get_line();

        if (strlen(sx) > 19) {
            printf("Za dlugi input\n");
        } else {
            strncpy(s2, sx, 19);
            free(sx);
            break;
        }
        free(sx);
    }

    return 1;
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
        perror("Blad utworzenia pamieci dzielonej");
        return 1;
    }

    printf("Identyfikator pamieci dzielonej: %d\n", shmID);
    sleep(1);

    dane_ptr = (struct Dane *) shmat(shmID, 0, 0);

    if (((long)dane_ptr) == -1) {
        perror("Blad dolaczania");
        return 2;
    }

    printf("Dolaczono strukture danych do obszaru pamieci dzielonej\n");
    sleep(1);

    key_t klucz2 = ftok(".", 'S');
    int semID = semget(klucz2, 3, 0640 | IPC_CREAT);
    if (semID == -1) {
        perror("Blad przy dolaczaniu/tworzeniu zbioru semaforow");
        return 3;
    }

    printf("Identyfikator zbioru semaforow: %d\n", semID);

    sleep(1);

    P(semID, 2);

    printf("Wpisz w osobnych liniach liczbe i stringa.\n");

    int x2 = 0;
    char s2[20];

    while (read_input(&x2, s2)) {
        P(semID, 0);

        dane_ptr->x = x2;
        strcpy(dane_ptr->s, s2);

        V(semID, 1);
    }

    V(semID, 2);
    V(semID, 1);

    if (shmdt((struct Dane *) dane_ptr) == -1) {
        perror("Blad przy odlaczaniu pamieci dzielonej");
        return 4;
    }

    printf("Pamiec dzielona odlaczona\n");

    return 0;
}