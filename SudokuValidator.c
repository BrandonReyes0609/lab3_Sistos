// SudokuValidator.c

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>

#define SIZE 9

int sudoku[SIZE][SIZE];

void cargar_sudoku(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error abriendo el archivo");
        exit(1);
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Error al obtener tamaño del archivo");
        close(fd);
        exit(1);
    }

    char *data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        exit(1);
    }

    
    int k = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (k >= st.st_size) {
                fprintf(stderr, "Archivo no contiene suficientes datos\n");
                munmap(data, st.st_size);
                close(fd);
                exit(1);
            }
            sudoku[i][j] = data[k++] - '0';
        }
    }

    munmap(data, st.st_size);
    close(fd);
}

void imprimir_sudoku() {
    printf("Sudoku cargado:\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d ", sudoku[i][j]);
        }
        printf("\n");
    }
}

int main() {
    cargar_sudoku("sudoku");
    imprimir_sudoku();
    return 0;
}
