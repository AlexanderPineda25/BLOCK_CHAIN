#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdint.h>
#include "hash.h"

#define SERVER_PORT 8888
#define BUFFER_SIZE 1024
#define MAX_TEXT 256
#define TIME_LIMIT_SECONDS 600

int g_should_stop = 0;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void* listen_server(void* param);
void find_solution(int sock,
                   const char* base_text,
                   const char* pad_text,
                   int zero_count,
                   uint64_t range_start,
                   uint64_t range_end);

int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char server_ip[50] = "127.0.0.1";
    int nread;

    if (argc > 1) {
        strcpy(server_ip, argv[1]);
    }

    printf("=== WORKER - CLIENTE DE MINERIA ===\n");
    printf("Conectando al servidor %s:%d...\n", server_ip, SERVER_PORT);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        printf("Direccion invalida / no soportada\n");
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar con el servidor");
        printf("Asegurese de que el servidor este ejecutandose.\n");
        return 1;
    }

    printf("Conectado al servidor!\n");

    nread = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (nread <= 0) {
        printf("Error al recibir configuracion\n");
        close(sock);
        return 1;
    }
    buffer[nread] = '\0';

    char base_text[MAX_TEXT];
    char pad_text[MAX_TEXT];
    int zero_count;
    uint64_t range_start, range_end;

    char* tok = strtok(buffer, "|");
    if (tok == NULL || strcmp(tok, "CONFIG") != 0) {
        printf("Error: Mensaje de configuracion invalido\n");
        close(sock);
        return 1;
    }

    tok = strtok(NULL, "|");
    strcpy(base_text, tok ? tok : "");

    tok = strtok(NULL, "|");
    strcpy(pad_text, tok ? tok : "");

    tok = strtok(NULL, "|");
    zero_count = tok ? atoi(tok) : 0;

    tok = strtok(NULL, "|");
    range_start = tok ? strtoull(tok, NULL, 10) : 0ULL;

    tok = strtok(NULL, "|");
    range_end = tok ? strtoull(tok, NULL, 10) : 0ULL;

    printf("\n=== CONFIGURACION RECIBIDA ===\n");
    printf("Texto base: %s\n", base_text);
    printf("Relleno: %s\n", pad_text);
    printf("Ceros requeridos: %d\n", zero_count);
    printf("Rango asignado: [%llu - %llu]\n",
           (unsigned long long)range_start,
           (unsigned long long)range_end);
    printf("Total de combinaciones a probar: %llu\n\n",
           (unsigned long long)(range_end - range_start + 1));

    pthread_t th;
    pthread_create(&th, NULL, listen_server, (void*)(intptr_t)sock);
    pthread_detach(th);

    printf("Iniciando busqueda...\n\n");
    find_solution(sock, base_text, pad_text, zero_count, range_start, range_end);

    sleep(2);

    close(sock);
    pthread_mutex_destroy(&g_mutex);

    printf("\nWorker finalizado.\n");
    return 0;
}

void* listen_server(void* param) {
    int sock = (int)(intptr_t)param;
    char buffer[BUFFER_SIZE];
    int nread;

    while ((nread = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[nread] = '\0';
        if (strcmp(buffer, "DETENER") == 0) {
            pthread_mutex_lock(&g_mutex);
            g_should_stop = 1;
            pthread_mutex_unlock(&g_mutex);
            printf("\n[SERVIDOR] Orden de detencion recibida (otro worker encontro la solucion)\n");
            break;
        }
    }
    return NULL;
}

void find_solution(int sock,
                   const char* base_text,
                   const char* pad_text,
                   int zero_count,
                   uint64_t range_start,
                   uint64_t range_end) {
    char full_text[BUFFER_SIZE];
    char variant[MAX_TEXT];
    char hash_hex[BUFFER_SIZE];
    char msg[BUFFER_SIZE];
    int pad_len = (int)strlen(pad_text);
    uint64_t attempts = 0;
    uint64_t last_progress = 0;
    time_t t_start = time(NULL);
    time_t t_now;
    int found = 0;

    for (uint64_t nonce = range_start; nonce <= range_end; nonce++) {
        pthread_mutex_lock(&g_mutex);
        int stop = g_should_stop;
        pthread_mutex_unlock(&g_mutex);
        if (stop) {
            printf("Deteniendo busqueda...\n");
            break;
        }

        t_now = time(NULL);
        if (difftime(t_now, t_start) > TIME_LIMIT_SECONDS) {
            printf("\nTiempo limite alcanzado (%d segundos)\n", TIME_LIMIT_SECONDS);
            break;
        }

        index_to_string_variant(nonce, pad_len, variant);
        sprintf(full_text, "%s%s", base_text, variant);

        compute_basic_hash(full_text, hash_hex);

        if (hash_has_trailing_zeros(hash_hex, zero_count)) {
            printf("\n*** SOLUCION ENCONTRADA! ***\n");
            printf("Nonce: %llu\n", (unsigned long long)nonce);
            printf("Combinacion: %s\n", variant);
            printf("Texto completo: %s\n", full_text);
            printf("Hash: %s\n", hash_hex);

            sprintf(msg, "SOLUCION|%s|%llu", hash_hex, (unsigned long long)nonce);
            send(sock, msg, strlen(msg), 0);
            found = 1;
            break;
        }

        attempts++;
        if (attempts - last_progress >= 10000) {
            double pct = ((double)(nonce - range_start) / (double)(range_end - range_start + 1)) * 100.0;
            printf("Progreso: %.2f%% - Intentos: %llu - Ultimo hash: %s\n",
                   pct, (unsigned long long)attempts, hash_hex);
            last_progress = attempts;
        }
    }

    if (!found) {
        pthread_mutex_lock(&g_mutex);
        int stop = g_should_stop;
        pthread_mutex_unlock(&g_mutex);

        if (!stop) {
            printf("\nBusqueda completada. No se encontro solucion en el rango asignado.\n");
            printf("Total de combinaciones probadas: %llu\n", (unsigned long long)attempts);
            sprintf(msg, "NO_ENCONTRADO");
            send(sock, msg, strlen(msg), 0);
        }
    }

    t_now = time(NULL);
    printf("Tiempo de ejecucion: %.0f segundos\n", difftime(t_now, t_start));
}
