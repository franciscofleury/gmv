#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAX_LOGS 300
#define SHARE_TIME 3
#include "public_gmv.h"

void set_up_alg(GmvControl *gmv, char algorithm, int k);

// Padrão de entrada: ./main <process_file_1> <process_file_2> <process_file3> <sub_alg> <alg_param>?
int main(int argc, char **argv)
{
    GmvControl *gmv = get_gmv();
    int expected_argc = PROCESS_N + 2;
    int k_parameter;
    if (argc > PROCESS_N + 1)
    {

        k_parameter = atoi(argv[PROCESS_N + 1]);
    }

    FILE *process_files[PROCESS_N];
    for (int i = 0; i < PROCESS_N; i++)
    {
        process_files[i] = fopen(argv[i + 1], "r");
        if (process_files[i] == NULL)
        {
            fprintf(stderr, "Erro ao ler arquivo %s\n", argv[i + 1]);
            return 2;
        }
    }
    int running;
    char stop_command = 's';
    do
    {
        char thrash;
        char algorithm;
        printf("Deseja parar o programa? (s/n)\n");
        scanf("%c", &stop_command);
        if (stop_command == 's')
        {
            break;
        }
        printf("Selecione o algoritmo de substituição:\n");
        printf("Digite 'N' - NRU\n");
        printf("Digite 'L' - LRU\n");
        printf("Digite 'S' - Second Chance\n");
        printf("Digite 'W' - Working Set\n");
        scanf("%c", &thrash);
        scanf("%c", &algorithm);

        set_up_alg(gmv, algorithm, k_parameter);

        printf("Digite o número de acessos que deseja realizar:\n");
        int rounds;
        scanf("%d", &rounds);
        int page_fault_count = 0;
        PageLog *log_vec[MAX_LOGS];
        running = rounds;
        int process_time = 0;
        while (running--)
        {
            FILE *file = process_files[gmv->current_process];
            char command[4];
            fread(command, 1, 4, file);
            int page_number = (command[0] - '0') * 10 + command[1] - '0';
            char mode = command[2];
            PageLog *log = get_page(gmv, page_number, mode);
            if (log != NULL)
            {
                log_vec[page_fault_count++] = log;
            }
            if (++process_time >= SHARE_TIME)
            {
                gmv->current_process = (gmv->current_process + 1) % PROCESS_N;
                process_time = 0;
            }
        }

        printf("Relatório:\n");
        switch (gmv->alg)
        {
        case NRU:
            printf("NRU\n");
            break;
        case SecondChance:
            printf("Second Chance\n");
            break;
        case LRU:
            printf("LRU\n");
            break;
        case WorkingSet:
            printf("WorkingSet %d\n", k_parameter);
            break;
        }
        printf("Rodadas executadas: %d\n", rounds);
        printf("Page Fault gerados: %d\n", page_fault_count);
        for (int i = 0; i < page_fault_count; i++)
        {
            PageLog *log = log_vec[i];
            printf("PROCESS=%d ", log->process);
            printf("New_Page=%d", log->new_page);
            if (log->old_page != -1)
            {
                printf(" Old_Page=%d", log->old_page);
            }
            printf(" Frame=%d", log->frame);
            if (log->saved)
            {
                printf(" SAVED");
            }
            printf("\n");
        }
        for (int i = 0; i < PROCESS_N; i++)
        {
            PageTable *table = gmv->process_tables + i;
            printf("TABELA PROCESSO %d:\n\n", i);
            for (int j = 0; j < VIRTUAL_SIZE; j++)
            {
                printf("Page = %d; R = %d; M = %d; Frame = %d; WS_age = %d;\n", j, table->tabela[j].r, table->tabela[j].m, table->tabela[j].frame, table->tabela[j].last_used);
            }
            printf("\n");
        }
        stop_command = 'n';
    } while (stop_command != 'n');

    for (int i = 0; i < PROCESS_N; i++)
    {
        fclose(process_files[i]);
    }

    free(gmv);
}

void set_up_alg(GmvControl *gmv, char algorithm, int k)
{
    switch (algorithm)
    {
    case 'N':
        set_alg(gmv, NRU);
        break;
    case 'L':
        set_alg(gmv, LRU);
        break;
    case 'S':
        set_alg(gmv, SecondChance);
        break;
    case 'W':
        set_param_alg(gmv, WorkingSet, k);
        break;
    }
}
