/*
* Nombre del equipo: S.O. AGREVAL
* Fecha: 09/10/2024
* Versión: 1.2.1
* Este programa en C simula la gestión de memoria de un sistema operativo, donde se almacenan 
* elementos en una memoria RAM simulada de tamaño fijo. Si la memoria está llena, el programa 
* realiza una operación de swap para mover el primer elemento a un archivo en disco y luego 
* agregar el nuevo elemento en su lugar. Se utilizan semáforos para la sincronización y un pipe 
* para la comunicación entre procesos padre e hijo. El proceso hijo agrega elementos a la RAM 
* y el proceso padre monitorea el progreso. El contenido final de la RAM se muestra en pantalla.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>

#define MAX_ITEMS 10       // Capacidad máxima de la memoria RAM simulada
#define SWAP_THRESHOLD 8   // Límite para activar el swap (no se usa en el código actual)

sem_t sem_ram;  // Semáforo para controlar el acceso a la RAM

// Estructura que representa un elemento de datos con un identificador y un campo de datos
typedef struct {
    int id;
    char data[20];
} DataItem;

// Simulación de la memoria RAM
DataItem ram[MAX_ITEMS];

// Función para imprimir los elementos en RAM
void printRAM() {
    printf("Contenido en RAM:\n");
    for (int i = 0; i < MAX_ITEMS; ++i) {
        if (ram[i].id != -1) {  // Verifica si la posición está ocupada
            printf("ID: %d, Data: %s\n", ram[i].id, ram[i].data);
        }
    }
}

// Simulación de la operación de swap, que guarda un elemento en un archivo en disco
void swapToDisk(const DataItem item) {
    char filename[255];
    sprintf(filename, "swap_%d.txt", item.id);  // Crea un nombre de archivo basado en el ID
    FILE *swap_file = fopen(filename, "w+");  // Abre o crea el archivo en modo escritura
    if (swap_file == NULL) {
        perror("Error al abrir archivo de swap");
        return;
    }
    fwrite(&item, sizeof(item), 1, swap_file);  // Escribe el elemento en el archivo
    fclose(swap_file);  // Cierra el archivo
    printf("** Proceso hijo: Escrito en disco el ID: %d (swap)\n", item.id);
}

// Función que el proceso hijo usará para agregar elementos a la RAM
void addToRAM(int pipe_fd[2], DataItem *item) {
    int added = 0;  // Indica si el elemento fue agregado a la RAM

    printf("** Proceso hijo: Intentando agregar ID: %d a la RAM\n", item->id);

    sem_wait(&sem_ram);  // Bloquea el acceso a la RAM usando el semáforo
    for (int i = 0; i < MAX_ITEMS; ++i) {
        if (ram[i].id == -1) {  // Encuentra una posición vacía en la RAM
            ram[i] = *item;  // Agrega el elemento a la RAM
            added = 1;
            break;
        }
    }
    sem_post(&sem_ram);  // Libera el acceso a la RAM

    if (added) {
        printf("** Proceso hijo: ID: %d agregado correctamente a la RAM\n", item->id);
    }

    if (!added) {  // Si la RAM estaba llena, realiza un swap del primer elemento
        swapToDisk(ram[0]);  // Simula el intercambio del primer elemento al disco
        sem_wait(&sem_ram);  // Bloquea el acceso a la RAM
        ram[0] = *item;      // Reemplaza el primer elemento con el nuevo
        sem_post(&sem_ram);  // Libera el acceso a la RAM
        printf("** Proceso hijo: ID: %d agregado tras swap a la RAM\n", item->id);
    }

    // Envía el ID del elemento agregado al proceso padre a través del pipe
    write(pipe_fd[1], &item->id, sizeof(item->id));
    free(item);  // Libera la memoria asignada para el elemento
}

int main() {
    int pipe_fd[2];  // Pipe para la comunicación entre el proceso padre e hijo
    pipe(pipe_fd);   // Crea la tubería

    // Inicializa la RAM con valores de ID -1 (vacío)
    for (int i = 0; i < MAX_ITEMS; ++i) {
        ram[i].id = -1;
    }

    sem_init(&sem_ram, 0, 1);  // Inicializa el semáforo con valor 1 para el acceso exclusivo

    // Crea el proceso hijo
    pid_t pid = fork();

    if (pid == 0) {
        // Código para el proceso hijo: agrega elementos a la RAM
        for (int i = 1; i <= 64; ++i) {
            DataItem *item = malloc(sizeof(DataItem));  // Asigna memoria para un nuevo elemento
            item->id = i;  // Asigna un ID al elemento
            sprintf(item->data, "Data %d", i);  // Asigna una cadena de datos
            addToRAM(pipe_fd, item);  // Intenta agregar el elemento a la RAM
        }
        close(pipe_fd[1]);  // Cierra el pipe de escritura
        exit(0);  // Finaliza el proceso hijo
    } else {
        // Código para el proceso padre: monitorea los elementos agregados a la RAM
        close(pipe_fd[1]);  // Cierra el pipe de escritura

        int id_received;
        for (int i = 1; i <= 64; ++i) {
            read(pipe_fd[0], &id_received, sizeof(id_received));  // Lee el ID del proceso hijo
            printf("Proceso padre: ID %d agregado a RAM.\n", id_received);
        }

        wait(NULL);  // Espera a que el proceso hijo termine

        // Imprime el contenido final de la RAM
        printRAM();
        printf("Finalización exitosa.\n");
    }

    sem_destroy(&sem_ram);  // Destruye el semáforo
    return 0;
}
