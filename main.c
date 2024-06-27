#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "public_gmv.h"

void set_up_alg(GmvControl *gmv, char algorithm, int k);

// Padrão de entrada: ./main <process_file_1> <process_file_2> <process_file3> <sub_alg> <alg_param>?
int main(int argc, char **argv)
{
    printf("aaa");
    GmvControl *gmv = get_gmv();
    printf("aaa");
    int expected_argc = PROCESS_N + 2;
    if (argc < expected_argc || (strcmp(argv[4], "WorkingSet") == 0 && argc != expected_argc + 1))
    {
        fprintf(stderr, "Erro! A entrada deve estar no formato:\n./main <process_file_1> <process_file_2> <process_file3> <sub_alg> <alg_param>?\n");
        return 1;
    }

    char algorithm = argv[PROCESS_N + 1][0];
    printf("aaa");
    int k_parameter = atoi(argv[PROCESS_N + 2]);
    printf("k: %d\n", k_parameter);
    set_up_alg(gmv, algorithm, k_parameter);

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
        printf("Selecione o algoritmo de substituição:\n");
        printf("Digite 'N' - NRU\n");
        printf("Digite 'L' - LRU\n");
        printf("Digite 'S' - Second Chance\n");
        printf("Digite 'W' - Working Set\n");
        scanf("%c", &algorithm);

        if (algorithm)
        {
            if (algorithm == 'W')
            {
                printf("Digite o valor de do parâmetro k para o algoritmo Working Set:\n");
                scanf("%d", &k_parameter);
            }
        }

        set_up_alg(gmv, algorithm, k_parameter);

        printf("Digite o número de acessos que deseja realizar:\n");
        scanf("%d", &running);

        while (running--)
        {
            FILE *file = process_files[gmv->current_process];
            char command[4];
            fread(command, 1, 4, file);
            printf("%c%c %c\n", command[0], command[1], command[2]);
            int page_number = (command[0] - '0') * 10 + command[1] - '0';
            char mode = command[2];
            int page_frame = get_page(gmv, page_number, mode);
            printf("Process: %d; Page_number: %d; Frame_number: %d\n", gmv->current_process, page_number, page_frame);
            gmv->current_process = (gmv->current_process + 1) % PROCESS_N;
        }

        printf("Deseja parar o programa? (s/n)\n");
        scanf("%c", &stop_command);

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
