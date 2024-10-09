/*
 * Nombre del equipo: S.O. AGREVAL
 * Fecha: 04/10/2024
 * Versión: 1.2.1
 * Programa que simula el funcionamiento básico de un banco con operaciones de depósito y retiro,
 * utilizando hilos y mutex para asegurar la exclusión mutua.
 *
 * Descripción:
 * El programa utiliza dos hilos, uno para realizar depósitos y otro para realizar retiros de 
 * una cuenta bancaria simulada. Una variable global representa el saldo de la cuenta, y se
 * utiliza un mutex para proteger el acceso a esta variable compartida, garantizando que 
 * las operaciones de depósito y retiro sean atómicas. Las operaciones de los hilos se ejecutan
 * en paralelo.
 * 
 * El saldo inicial, el monto a depositar y el monto a retirar se solicitan al usuario. 
 * Las funciones para depositar y retirar validan si las operaciones se pueden realizar 
 * correctamente, como el caso de un saldo insuficiente para un retiro.
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// Variable global que representa el saldo de la cuenta bancaria
int saldo;

// Mutex para proteger la sección crítica que modifica el saldo
pthread_mutex_t mutex;

/*
 * Función: Depositar
 * Simula una operación de depósito en la cuenta bancaria.
 * Bloquea el mutex antes de modificar el saldo para asegurar que solo
 * un hilo acceda a la variable saldo a la vez.
 */
void* depositar(void* arg) {
    int monto = *((int*)arg);
    
    // Intentar bloquear el mutex
    if (pthread_mutex_lock(&mutex) != 0) {
        printf("Error al bloquear el mutex en la función depositar.\n");
        return NULL;
    }

    // Sección crítica: modificar el saldo
    saldo += monto;
    printf("Depósito realizado: +%d, Saldo actual: %d\n", monto, saldo);

    // Desbloquear el mutex
    if (pthread_mutex_unlock(&mutex) != 0) {
        printf("Error al desbloquear el mutex en la función depositar.\n");
    }

    return NULL;
}

/*
 * Función: Retirar
 * Simula una operación de retiro de la cuenta bancaria.
 * Verifica que el saldo sea suficiente antes de realizar el retiro.
 * Bloquea el mutex antes de modificar el saldo para asegurar que solo
 * un hilo acceda a la variable saldo a la vez.
 */
void* retirar(void* arg) {
    int monto = *((int*)arg);

    // Intentar bloquear el mutex
    if (pthread_mutex_lock(&mutex) != 0) {
        printf("Error al bloquear el mutex en la función retirar.\n");
        return NULL;
    }

    // Sección crítica: verificar y modificar el saldo
    if (saldo >= monto) {
        saldo -= monto;
        printf("Retiro realizado: -%d, Saldo actual: %d\n", monto, saldo);
    } else {
        printf("Retiro fallido: -%d, Saldo insuficiente: %d\n", monto, saldo);
    }

    // Desbloquear el mutex
    if (pthread_mutex_unlock(&mutex) != 0) {
        printf("Error al desbloquear el mutex en la función retirar.\n");
    }

    return NULL;
}

/*
 * Función: obtenerMontoPositivo
 * Solicita un monto positivo al usuario, validando que no sea negativo.
 */
int obtenerMontoPositivo(const char* mensaje) {
    int monto;
    do {
        printf("%s", mensaje);
        scanf("%d", &monto);
        if (monto < 0) {
            printf("Error: El monto no puede ser negativo. Inténtelo de nuevo.\n");
        }
    } while (monto < 0);
    return monto;
}

int main() {
    // Inicializar el mutex
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("Error al inicializar el mutex.\n");
        return 1;
    }

    // Crear los hilos para depositar y retirar
    pthread_t hilo_deposito, hilo_retiro;
    int monto_deposito, monto_retiro;

    // Pedir al usuario que ingrese el saldo inicial, el monto de depósito y el monto de retiro
    saldo = obtenerMontoPositivo("Ingrese el saldo inicial: ");
    monto_deposito = obtenerMontoPositivo("Ingrese el monto a depositar: ");
    monto_retiro = obtenerMontoPositivo("Ingrese el monto a retirar: ");

    // Crear hilo de depósito
    if (pthread_create(&hilo_deposito, NULL, depositar, &monto_deposito) != 0) {
        printf("Error al crear el hilo para depositar.\n");
        pthread_mutex_destroy(&mutex);
        return 1;
    }

    // Crear hilo de retiro
    if (pthread_create(&hilo_retiro, NULL, retirar, &monto_retiro) != 0) {
        printf("Error al crear el hilo para retirar.\n");
        pthread_mutex_destroy(&mutex);
        return 1;
    }

    // Esperar a que los hilos terminen
    pthread_join(hilo_deposito, NULL);
    pthread_join(hilo_retiro, NULL);

    // Destruir el mutex
    pthread_mutex_destroy(&mutex);

    printf("Operaciones completadas. Saldo final: %d\n", saldo);

    return 0;
}

