#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "public_gmv.h"

// Padrão de entrada: ./main <process_file_1> <process_file_2> <process_file3> <sub_alg> <alg_param>?
int main(int argc, char **argv)
{
    GmvControl *gmv = get_gmv();
    int expected_argc = PROCESS_N + 2;
    if (argc < expected_argc || (strcmp(argv[4], "WorkingSet") == 0) && argc != expected_argc + 1)
    {
        fprintf(stderr, "Erro! A entrada deve estar no formato:\n./main <process_file_1> <process_file_2> <process_file3> <sub_alg> <alg_param>?\n");
        return 1;
    }

    set_up_alg(gmv, argv);

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

    int running = 1;
    while (running)
    {
        FILE *file = process_files[gmv->current_process];
        char command[4];
        fread(command, 1, 4, file);
        int page_number = atoi(command[0]) * 10 + atoi(command[1]);
        char mode = command[2];
        int page_frame = get_page(gmv, page_number, mode);
        gmv->current_process = (gmv->current_process + 1) % PROCESS_N;
    }
}

void set_up_alg(GmvControl *gmv, char **arguments)
{
    switch (arguments[PROCESS_N + 1][0])
    {
    case 'N':
        setAlg(gmv, NRU);
        break;
    case 'L':
        setAlg(gmv, LRU);
        break;
    case 'S':
        setAlg(gmv, SecondChance);
        break;
    case 'W':
        int alg_param = atoi(arguments[PROCESS_N + 2]);
        setAlg(gmv, WorkingSet, alg_param);
        break;
    }
}
