#include "gmv.h"

// Essa função é chamada sempre que um processo faz um acesso/escrita à memória
int get_page(GmvControl *gmv, int page, char mode) {
    PageTable tabela_atual = gmv->process_tables[gmv->current_process];
    int frame;

    if (tabela_atual.tabela[page].frame != -1) {
        frame = tabela_atual.tabela[page].frame;
    } else {
        frame = page_fault(gmv, page);
    }

    tabela_atual.tabela[page].m = (mode == 'w')? 1: tabela_atual.tabela[page].m;
    tabela_atual.tabela[page].r = (mode == 'r')? 1: tabela_atual.tabela[page].r;

    return frame;
}

int page_fault(GmvControl *gmv, int page) {
    PageTable current_table = gmv->process_tables[gmv->current_process];
    int i=0;
    while (i<RAM_SIZE) {
        if (gmv->frame_table[i] == 0) {
            current_table.tabela[page].frame = i;
            return i;
        }
    }
    
    // Chamar algoritmo de substituição
}