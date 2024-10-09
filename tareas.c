#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

// Función para leer líneas de un archivo y escribir líneas pares e impares en las tuberías
void tarea1(int pipe1[2], int pipe2[2], const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error abriendo archivo");
        exit(EXIT_FAILURE);
    }
    
    char buffer[BUFFER_SIZE];
    int line_num = 1;
    
    // Cerrar lectura de las tuberías en este proceso
    close(pipe1[0]);
    close(pipe2[0]);
    
    while (fgets(buffer, BUFFER_SIZE, file)) {
        if (line_num % 2 == 0) {
            write(pipe1[1], buffer, strlen(buffer));  // Líneas pares
        } else {
            write(pipe2[1], buffer, strlen(buffer));  // Líneas impares
        }
        line_num++;
    }
    
    fclose(file);
    close(pipe1[1]);
    close(pipe2[1]);
}

// Función para la tarea 2 (grep en líneas pares)
void tarea2(int pipe_in[2], int pipe_out[2], const char *word) {
    char buffer[BUFFER_SIZE];
    
    close(pipe_in[1]);  // Cerrar escritura de la tubería de entrada
    close(pipe_out[0]); // Cerrar lectura de la tubería de salida
    
    FILE *input = fdopen(pipe_in[0], "r");
    
    while (fgets(buffer, BUFFER_SIZE, input)) {
        if (strstr(buffer, word)) {
            write(pipe_out[1], buffer, strlen(buffer));
        }
    }
    
    fclose(input);
    close(pipe_out[1]);
}

// Función para la tarea 3 (grep en líneas impares)
void tarea3(int pipe_in[2], int pipe_out[2], const char *word) {
    tarea2(pipe_in, pipe_out, word); // Misma funcionalidad, pero con otra palabra
}

// Función para la tarea 4 (ordenar la salida con sort)
void tarea4(int pipe_in[2]) {
    char *sort_args[] = {"sort", NULL};
    close(pipe_in[1]); // Cerrar escritura de la tubería
    
    dup2(pipe_in[0], STDIN_FILENO);  // Redirigir entrada estándar a la tubería
    execvp("sort", sort_args);  // Ejecutar el comando sort
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <archivo> <palabra1> <palabra2>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    const char *filename = argv[1];
    const char *word1 = argv[2];
    const char *word2 = argv[3];
    
    int pipe1[2], pipe2[2], pipe3[2];  // Tuberías para las diferentes tareas
    
    // Crear las tuberías
    pipe(pipe1);
    pipe(pipe2);
    pipe(pipe3);
    
    pid_t pid1, pid2, pid3, pid4;
    
    // Primer proceso para la tarea 1
    if ((pid1 = fork()) == 0) {
        tarea1(pipe1, pipe2, filename);
        exit(EXIT_SUCCESS);
    }
    
    // Segundo proceso para la tarea 2 (grep en líneas pares)
    if ((pid2 = fork()) == 0) {
        tarea2(pipe1, pipe3, word1);
        exit(EXIT_SUCCESS);
    }
    
    // Tercer proceso para la tarea 3 (grep en líneas impares)
    if ((pid3 = fork()) == 0) {
        tarea3(pipe2, pipe3, word2);
        exit(EXIT_SUCCESS);
    }
    
    // Cuarto proceso para la tarea 4 (sort)
    if ((pid4 = fork()) == 0) {
        tarea4(pipe3);
        exit(EXIT_SUCCESS);
    }
    
    // Esperar a que terminen los hijos
    wait(NULL);
    wait(NULL);
    wait(NULL);
    wait(NULL);
    
    return 0;
}


