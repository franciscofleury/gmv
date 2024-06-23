#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "types.h"
#include "gmv.h"
#define DEFAULT_ALG NRU

// Algortimo de substituição rodando no momento
SUB_ALG alg = DEFAULT_ALG;
// Parâmetro adicional do algoritmo (só necessário para WorkingSet)
int alg_param = -1; // -1 indica que não existe parâmetro para o algoritmo selecionado

// Padrão de entrada: ./main <process_file_1> <process_file_2> <process_file3> <sub_alg> <alg_param>?
int main(int argc, char** argv) {
    if (argc < 5 || (strcmp(argv[4], "WorkingSet") == 0) && argc != 6) {
        fprintf(stderr, "Erro! A entrada deve estar no formato:\n./main <process_file_1> <process_file_2> <process_file3> <sub_alg> <alg_param>?\n");
        return 1;
    }

    setAlg(argv);


}

void setAlg(char **arguments) {
    switch (arguments[0][0]) {
        case 'N':
            alg = NRU;
            break;
        case 'L':
            alg = LRU;
            break;
        case 'S':
            alg = SecondChance;
            break;
        case 'W':
            alg = WorkingSet;
            alg_param = atoi(arguments[1]);
            break;
    }
}

