#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    int period;
    int execution_time;
    int deadline;
    int remaining_time;
    int next_deadline;
} Task;

int GreatestCommonDivisor(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int LeastCommonMultiple(int a, int b) {
    return (a * b) / GreatestCommonDivisor(a, b);
}

void read_tasks(const char* filename, Task tasks[], int* n, int* end_time) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        exit(1);
    }

    fscanf(file, "%*s %*s %*s");  // Ignora a primeira linha (P C D)
    int i = 0;
    int end_time_aux = 1;
    while (fscanf(file, "%d %d %d", &tasks[i].period, &tasks[i].execution_time, &tasks[i].deadline) != EOF) {
        tasks[i].remaining_time = tasks[i].execution_time;
        tasks[i].next_deadline = tasks[i].deadline;
        end_time_aux = LeastCommonMultiple(end_time_aux, tasks[i].period);
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
        if (tasks[i].remaining_time > 0 && tasks[i].next_deadline < min_deadline) {
            min_deadline = tasks[i].next_deadline;
            chosen_task = i;
        }
    }
    return chosen_task;
}

int is_schedulable(Task tasks[], int n, char* algorithm) {
    if (algorithm == "RM") {
        float utilization = 0;
        for (int i = 0; i < n; i++) {
            utilization += (float)tasks[i].execution_time / tasks[i].period;
        }
        float threshold = n * (pow(2, 1.0 / n) - 1);
        return utilization <= threshold;
    }else if (algorithm == "EDF") {
        float utilization = 0;
        for (int i = 0; i < n; i++) {
            utilization += (float)tasks[i].execution_time / tasks[i].period;
        }
        return utilization <= 1;
    }
    
}

void execute_schedule(Task tasks[], int n, int end_time, int (*scheduler)(Task[], int, int)) {
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
            printf("Time %d: Task %d\n", time, task_id + 1);
        } else {
            printf("Time %d: Nenhum\n", time);
        }
    }
}

int main() {
    Task tasks[10];
    int n;
    int end_time;
    read_tasks("tasks.txt", tasks, &n, &end_time);

    if (is_schedulable(tasks, n, "RM")) {
        printf("O conjunto de tarefas é escalonável no RM.\n");
    } else {
        printf("O conjunto de tarefas NÃO é escalonável no RM.\n");
    }

    printf("\nEscalonamento Rate Monotonic:\n");
    execute_schedule(tasks, n, end_time, rate_monotonic);


    if (is_schedulable(tasks, n, "EDF")) {
        printf("\nO conjunto de tarefas é escalonável no EDF.\n");
    } else {
        printf("\nO conjunto de tarefas NÃO é escalonável no EDF.\n");
    }

    printf("\nEscalonamento Earliest Deadline First:\n");
    execute_schedule(tasks, n, end_time, earliest_deadline_first);

    return 0;
}
