#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>  // Para strcmp

typedef struct {
    int period;
    int execution_time;
    int deadline;
    int remaining_time;
    int next_deadline;
} Task;

void read_tasks(const char* filename, Task** tasks, int* n, int* end_time) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        exit(1);
    }

    fscanf(file, "%*s %*s %*s");  // Ignora a primeira linha (P C D)

    // Inicialmente aloca memória para 10 tarefas
    int capacity = 10;
    *tasks = (Task*)malloc(capacity * sizeof(Task));
    if (*tasks == NULL) {
        printf("Erro de alocação de memória.\n");
        exit(1);
    }

    int i = 0;
    int end_time_aux = 0;
    while (fscanf(file, "%d %d %d", &(*tasks)[i].period, &(*tasks)[i].execution_time, &(*tasks)[i].deadline) != EOF) {
        if (i >= capacity) {
            // Se necessário, aumenta a capacidade
            capacity *= 2;
            *tasks = (Task*)realloc(*tasks, capacity * sizeof(Task));
            if (*tasks == NULL) {
                printf("Erro de alocação de memória.\n");
                exit(1);
            }
        }
        (*tasks)[i].remaining_time = (*tasks)[i].execution_time;
        (*tasks)[i].next_deadline = (*tasks)[i].deadline;
        end_time_aux < (*tasks)[i].deadline ? end_time_aux = (*tasks)[i].deadline : end_time_aux;
        i++;
    }
    *n = i;
    *end_time = end_time_aux;
    fclose(file);
}

int rate_monotonic(Task tasks[], int n, int time) {
    int min_period = 99999;
    int chosen_task = -1;
    for (int i = 0; i < n; i++) {
        if (tasks[i].remaining_time > 0 && tasks[i].period < min_period) {
            min_period = tasks[i].period;
            chosen_task = i;
        }
    }
    return chosen_task;
}

int earliest_deadline_first(Task tasks[], int n, int time) {
    int min_deadline = 99999;
    int chosen_task = -1;
    for (int i = 0; i < n; i++) {
        if (tasks[i].remaining_time > 0 && tasks[i].next_deadline <= min_deadline) {
            min_deadline = tasks[i].next_deadline;
            chosen_task = i;
        }
    }
    return chosen_task;
}

void print_schedulability_result(const char* algorithm, float utilization, int n) {
    if (strcmp(algorithm, "RM") == 0) {
        float threshold = n * (pow(2, 1.0 / n) - 1);
        printf("\nUtilização calculada: %.2f\n", utilization);
        printf("Utilização máxima permitida para RM: %.2f\n", threshold);
        if (utilization <= threshold) {
            printf("O conjunto de tarefas é escalonável no RM.\n");
        } else {
            printf("O conjunto de tarefas NÃO é recomendável escalar com RM.\n");
        }
    } else if (strcmp(algorithm, "EDF") == 0) {
        printf("\nUtilização calculada: %.2f\n", utilization);
        printf("Utilização máxima permitida para EDF: 1.00\n");
        if (utilization <= 1) {
            printf("O conjunto de tarefas é escalonável no EDF.\n");
        } else {
            printf("O conjunto de tarefas NÃO é recomendável escalar com EDF.\n");
        }
    }
}

int is_schedulable(Task tasks[], int n, const char* algorithm) {
    float utilization = 0;
    for (int i = 0; i < n; i++) {
        utilization += (float)tasks[i].execution_time / tasks[i].period;
    }
    print_schedulability_result(algorithm, utilization, n);
    return 0;
}

void execute_schedule(Task tasks[], int n, int end_time, int (*scheduler)(Task[], int, int)) {
    // Criar a matriz de escalonamento
    char** schedule = (char**)malloc(end_time * sizeof(char*));
    if (schedule == NULL) {
        printf("Erro de alocação de memória.\n");
        exit(1);
    }
    for (int i = 0; i < end_time; i++) {
        schedule[i] = (char*)malloc(n * sizeof(char));
        if (schedule[i] == NULL) {
            printf("Erro de alocação de memória.\n");
            exit(1);
        }
        for (int j = 0; j < n; j++) {
            schedule[i][j] = '-';  // Inicialmente, nenhum tempo está ocupado
        }
    }

    for (int time = 0; time < end_time; time++) {
        for (int i = 0; i < n; i++) {
            if (time % tasks[i].period == 0) {
                tasks[i].remaining_time = tasks[i].execution_time;
                tasks[i].next_deadline = time + tasks[i].deadline;
            }
        }
        
        int task_id = scheduler(tasks, n, time);
        
        if (task_id != -1) {
            tasks[task_id].remaining_time--;
            schedule[time][task_id] = 'O';  // Marcar o tempo como ocupado pela tarefa
        }
    }

    // Determinar o número máximo de caracteres a imprimir (considerando alinhamento)
    int max_width = 2;  // Para os caracteres 'O' e '-'
    int num_digits = snprintf(NULL, 0, "%d", end_time - 1);  // Número de dígitos do último índice
    if (num_digits > max_width) {
        max_width = num_digits;
    }
    
    // Imprimir a matriz de escalonamento
    for (int i = 0; i < n; i++) {
        printf("t%d ", i + 1);
        for (int j = 0; j < end_time; j++) {
            printf("%*c ", max_width, schedule[j][i]);  // Usa largura ajustada
        }
        printf("\n");
    }

    // Imprimir os tempos
    printf("   ");
    for (int j = 0; j < end_time; j++) {
        printf("%*d ", max_width, j);  // Usa largura ajustada
    }
    printf("\n");

    // Liberar memória
    for (int i = 0; i < end_time; i++) {
        free(schedule[i]);
    }
    free(schedule);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_de_entrada>\n", argv[0]);
        return 1;
    }

    Task* tasks;
    int n;
    int end_time;
    char* file = argv[1];
    read_tasks(file, &tasks, &n, &end_time);

    is_schedulable(tasks, n, "RM");
    printf("\nEscalonamento Rate Monotonic:\n");
    execute_schedule(tasks, n, end_time, rate_monotonic);

    is_schedulable(tasks, n, "EDF");
    printf("\nEscalonamento Earliest Deadline First:\n");
    execute_schedule(tasks, n, end_time, earliest_deadline_first);

    // Liberar memória
    free(tasks);

    return 0;
}
