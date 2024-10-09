/**
 * Nombre del equipo: S.O. AGREVAL
 * Versión: 1.1.1
 * Fecha: 06/10/2024
 * 
 * Simulación del paso de vehículos por un puente.
 * 
 * Descripción:
 * Este programa simula un puente estrecho que solo permite el paso de vehículos en un único sentido. 
 * Se generan 100 vehículos (50 en cada sentido), y mediante hilos y semáforos se controla el acceso 
 * al puente, respetando la prioridad de los vehículos que esperan en el mismo sentido.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_VEHICULOS 100
#define VEHICULOS_SENTIDO 50

sem_t semaforo;           // Controla el acceso al puente
int esperando_izq = 0;     // Vehículos esperando en el sentido izquierdo
int esperando_der = 0;     // Vehículos esperando en el sentido derecho
int en_puente = 0;         // Vehículos actualmente en el puente
int sentido_actual = 0;    // 0: sin sentido, 1: izquierda, 2: derecha

void* vehiculo_izq(void* arg) {
    int id = *(int*)arg;
    sleep(rand() % 20);  // Espera aleatoria antes de intentar cruzar
    esperando_izq++;

    while (1) {
        sem_wait(&semaforo);

        if ((sentido_actual == 0 || sentido_actual == 1) && esperando_der == 0) {
            // Si el puente está libre o ya están pasando vehículos en el mismo sentido
            sentido_actual = 1;
            en_puente++;
            esperando_izq--;
            sem_post(&semaforo);

            printf("Vehículo %d (izquierda) entra al puente.\n", id);
            sleep(1);  // Cruza el puente durante 1 segundo
            printf("Vehículo %d (izquierda) sale del puente.\n", id);

            sem_wait(&semaforo);
            en_puente--;
            if (en_puente == 0) sentido_actual = 0;
            sem_post(&semaforo);
            break;
        } else {
            sem_post(&semaforo);
            usleep(100);  // Espera antes de intentar de nuevo
        }
    }
    return NULL;
}

void* vehiculo_der(void* arg) {
    int id = *(int*)arg;
    sleep(rand() % 20);  // Espera aleatoria antes de intentar cruzar
    esperando_der++;

    while (1) {
        sem_wait(&semaforo);

        if ((sentido_actual == 0 || sentido_actual == 2) && esperando_izq == 0) {
            // Si el puente está libre o ya están pasando vehículos en el mismo sentido
            sentido_actual = 2;
            en_puente++;
            esperando_der--;
            sem_post(&semaforo);

            printf("Vehículo %d (derecha) entra al puente.\n", id);
            sleep(1);  // Cruza el puente durante 1 segundo
            printf("Vehículo %d (derecha) sale del puente.\n", id);

            sem_wait(&semaforo);
            en_puente--;
            if (en_puente == 0) sentido_actual = 0;
            sem_post(&semaforo);
            break;
        } else {
            sem_post(&semaforo);
            usleep(100);  // Espera antes de intentar de nuevo
        }
    }
    return NULL;
}

int main() {
    pthread_t vehiculos[NUM_VEHICULOS];
    int ids[NUM_VEHICULOS];

    sem_init(&semaforo, 0, 1);

    // Crear hilos para vehículos que van en ambos sentidos
    for (int i = 0; i < VEHICULOS_SENTIDO; i++) {
        ids[i] = i;
        pthread_create(&vehiculos[i], NULL, vehiculo_izq, &ids[i]);
    }
    for (int i = VEHICULOS_SENTIDO; i < NUM_VEHICULOS; i++) {
        ids[i] = i;
        pthread_create(&vehiculos[i], NULL, vehiculo_der, &ids[i]);
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < NUM_VEHICULOS; i++) {
        pthread_join(vehiculos[i], NULL);
    }

    sem_destroy(&semaforo);
    return 0;
}
