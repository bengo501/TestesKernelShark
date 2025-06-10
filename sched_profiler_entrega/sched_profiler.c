#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>

// Definindo políticas se não estiverem definidas
#ifndef SCHED_IDLE
#define SCHED_IDLE 5
#endif

#ifndef SCHED_LOW_IDLE
#define SCHED_LOW_IDLE 7
#endif

// Variáveis globais
char *global_buffer;
int buffer_size;
int buffer_index = 0;
int num_threads;
int *thread_schedule_count;
sem_t mutex;

// Função para simular trabalho computacional mais intensivo
void simulate_work() {
    volatile int dummy = 0;
    // Aumentar significativamente o trabalho computacional
    for (int i = 0; i < 100000; i++) {
        dummy += i * i;
        // Adicionar mais operações para consumir mais tempo de CPU
        if (i % 1000 == 0) {
            dummy = dummy * 2 + 1;
        }
    }
}

void *thread_function(void *arg) {
    int thread_id = *((int *)arg);
    char thread_char = 'A' + thread_id;
    int writes_per_quantum = 10;  // Número de escritas por quantum de tempo

    while (1) {
        // Simular trabalho computacional intensivo
        simulate_work();
        
        sem_wait(&mutex);

        // Verificar se ainda há espaço no buffer
        if (buffer_index >= buffer_size) {
            sem_post(&mutex);
            break;
        }

        // Escrever múltiplas vezes para criar períodos mais longos
        int writes_this_turn = 0;
        while (buffer_index < buffer_size && writes_this_turn < writes_per_quantum) {
            global_buffer[buffer_index++] = thread_char;
            thread_schedule_count[thread_id]++;
            writes_this_turn++;
        }

        sem_post(&mutex);

        // Mais trabalho computacional após escrever
        simulate_work();
        
        // REMOVIDO: sched_yield() para permitir execução mais longa
        // Pequena pausa para permitir interrupção natural do scheduler
        usleep(1000);  // 1ms de pausa
    }

    return NULL;
}

void post_process_buffer() {
    printf("Saída sem pós-processamento:\n");
    for (int i = 0; i < buffer_size; i++) {
        printf("%c", global_buffer[i]);
        if ((i + 1) % 50 == 0) printf("\n");
    }
    if (buffer_size % 50 != 0) printf("\n");

    printf("\nSaída após pós-processamento:\n");

    if (buffer_size == 0) {
        printf("(buffer vazio)\n");
        return;
    }

    char current_char = global_buffer[0];
    printf("%c", current_char);
    int post_count[26] = {0};
    post_count[current_char - 'A'] = 1;

    for (int i = 1; i < buffer_size; i++) {
        if (global_buffer[i] != current_char) {
            current_char = global_buffer[i];
            printf("%c", current_char);
            post_count[current_char - 'A']++;
        }
    }
    printf("\n");

    printf("\nContagem de períodos de execução:\n");
    for (int i = 0; i < num_threads; i++) {
        printf("%c = %d\n", 'A' + i, post_count[i]);
    }
}

void print_statistics() {
    printf("\nEstatísticas de escalonamento (total de escritas):\n");
    for (int i = 0; i < num_threads; i++) {
        printf("%c = %d\n", 'A' + i, thread_schedule_count[i]);
    }
    
    printf("\nDistribuição percentual:\n");
    for (int i = 0; i < num_threads; i++) {
        float percentage = (float)thread_schedule_count[i] * 100.0 / buffer_size;
        printf("%c = %.1f%%\n", 'A' + i, percentage);
    }
}

const char* get_policy_name(int policy) {
    switch(policy) {
        case SCHED_OTHER: return "SCHED_OTHER";
        case SCHED_FIFO: return "SCHED_FIFO";
        case SCHED_RR: return "SCHED_RR";
        case SCHED_IDLE: return "SCHED_IDLE";
        case SCHED_LOW_IDLE: return "SCHED_LOW_IDLE";
        default: return "UNKNOWN";
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <tamanho_buffer> <num_threads> <policy>\n", argv[0]);
        printf("Políticas disponíveis:\n");
        printf("  0 = SCHED_OTHER (padrão)\n");
        printf("  1 = SCHED_FIFO (tempo real)\n");
        printf("  2 = SCHED_RR (round-robin)\n");
        printf("  5 = SCHED_IDLE (baixa prioridade)\n");
        printf("  7 = SCHED_LOW_IDLE (prioridade mais baixa que IDLE)\n");
        printf("\nExemplo: %s 1000 4 0\n", argv[0]);
        return 1;
    }

    buffer_size = atoi(argv[1]);
    num_threads = atoi(argv[2]);
    int policy = atoi(argv[3]);

    if (buffer_size <= 0 || num_threads <= 0 || num_threads > 26) {
        printf("Erro: Parâmetros inválidos\n");
        printf("Buffer size deve ser > 0, threads deve estar entre 1-26\n");
        return 1;
    }

    // Validar política de escalonamento
    if (policy != SCHED_OTHER && policy != SCHED_FIFO && policy != SCHED_RR && 
        policy != SCHED_IDLE && policy != SCHED_LOW_IDLE) {
        printf("Erro: Política de escalonamento inválida: %d\n", policy);
        return 1;
    }

    global_buffer = malloc(buffer_size * sizeof(char));
    thread_schedule_count = calloc(num_threads, sizeof(int));
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    int *thread_ids = malloc(num_threads * sizeof(int));
    
    if (!global_buffer || !thread_schedule_count || !threads || !thread_ids) {
        printf("Erro: Falha na alocação de memória\n");
        return 1;
    }
    
    sem_init(&mutex, 0, 1);

    printf("=== SCHED_PROFILER ===\n");
    printf("Política: %s (%d)\n", get_policy_name(policy), policy);
    printf("Threads: %d\n", num_threads);
    printf("Buffer size: %d\n", buffer_size);
    printf("Iniciando execução...\n\n");

    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;

        pthread_attr_t attr;
        pthread_attr_init(&attr);

        struct sched_param sched;
        // Configurar prioridade baseada na política
        if (policy == SCHED_FIFO || policy == SCHED_RR) {
            sched.sched_priority = 10;  // Prioridade moderada para RT
        } else {
            sched.sched_priority = 0;   // Prioridade 0 para outras políticas
        }

        if (pthread_attr_setschedpolicy(&attr, policy) != 0) {
            perror("Aviso: Erro ao definir política (pode necessitar privilégios root)");
        }

        if (pthread_attr_setschedparam(&attr, &sched) != 0) {
            perror("Aviso: Erro ao definir prioridade");
        }

        if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0) {
            perror("Aviso: Erro ao definir herança de escalonamento");
        }

        int res = pthread_create(&threads[i], &attr, thread_function, &thread_ids[i]);
        if (res != 0) {
            fprintf(stderr, "Erro ao criar thread %d: %s\n", i, strerror(res));
        }

        pthread_attr_destroy(&attr);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Execução concluída!\n\n");
    post_process_buffer();
    print_statistics();

    // Cleanup
    free(global_buffer);
    free(thread_schedule_count);
    free(threads);
    free(thread_ids);
    sem_destroy(&mutex);

    return 0;
}
