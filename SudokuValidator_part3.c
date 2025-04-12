#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <omp.h>

#define SIZE 9

int sudoku[SIZE][SIZE];

// ------------------------- Cargar Sudoku -------------------------
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
            sudoku[i][j] = data[k++] - '0';
        }
    }

    munmap(data, st.st_size);
    close(fd);
}

// ------------------------- Imprimir -------------------------
void imprimir_sudoku() {
    printf("Sudoku cargado:\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d ", sudoku[i][j]);
        }
        printf("\n");
    }
}

// ------------------------- Verificar Filas -------------------------
int verificar_filas() {
    for (int i = 0; i < SIZE; i++) {
        int visto[SIZE + 1] = {0};
        for (int j = 0; j < SIZE; j++) {
            int num = sudoku[i][j];
            if (num < 1 || num > 9 || visto[num]) {
                printf("❌ Fila %d inválida\n", i + 1);
                return 0;
            }
            visto[num] = 1;
        }
    }
    printf("✔ Todas las filas son válidas\n");
    return 1;
}

// ------------------------- Verificar Columnas -------------------------
void* verificar_columnas(void* arg) {
    for (int j = 0; j < SIZE; j++) {
        int visto[SIZE + 1] = {0};
        for (int i = 0; i < SIZE; i++) {
            int num = sudoku[i][j];
            if (num < 1 || num > 9 || visto[num]) {
                printf("❌ Columna %d inválida\n", j + 1);
                pthread_exit((void*)0);
            }
            visto[num] = 1;
        }
    }
    printf("✔ Todas las columnas son válidas\n");
    pthread_exit((void*)1);
}

// ------------------------- Verificar Subcuadro 3x3 -------------------------
int verificar_subcuadro(int fila_inicio, int col_inicio) {
    int visto[SIZE + 1] = {0};
    for (int i = fila_inicio; i < fila_inicio + 3; i++) {
        for (int j = col_inicio; j < col_inicio + 3; j++) {
            int num = sudoku[i][j];
            if (num < 1 || num > 9 || visto[num]) {
                printf("❌ Subcuadro (%d,%d) inválido\n", fila_inicio / 3 + 1, col_inicio / 3 + 1);
                return 0;
            }
            visto[num] = 1;
        }
    }
    return 1;
}

// ------------------------- Verificar Todos los Subcuadros -------------------------
int verificar_todos_los_subcuadros() {
    int valido = 1;

    #pragma omp parallel for collapse(2) shared(valido)
    for (int i = 0; i < SIZE; i += 3) {
        for (int j = 0; j < SIZE; j += 3) {
            if (!verificar_subcuadro(i, j)) {
                #pragma omp critical
                valido = 0;
            }
        }
    }

    if (valido) {
        printf("✔ Todos los subcuadros son válidos\n");
    }
    return valido;
}

// ------------------------- Main -------------------------
int main() {
    cargar_sudoku("sudoku");
    imprimir_sudoku();

    verificar_filas();

    pthread_t thread;
    void* resultado;

    if (pthread_create(&thread, NULL, verificar_columnas, NULL) != 0) {
        perror("Error al crear hilo");
        exit(1);
    }

    pthread_join(thread, &resultado);
    if ((int)(long)resultado == 0) {
        printf("❌ Error en la validación de columnas\n");
    }

    // Validar subcuadros con OpenMP
    verificar_todos_los_subcuadros();

    return 0;
}
