#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include "types.h"
#include "gmv.h"

int get_current_time();

int replace_page(GmvControl *gmv, int page_to_replace) {
    PageTable *current_table = &gmv->process_tables[gmv->current_process];
    int frame_to_reuse = current_table->tabela[page_to_replace].frame;

    // Marca a página antiga como não estando mais em um frame
    current_table->tabela[page_to_replace].frame = -1;
    current_table->tabela[page_to_replace].r = 0;
    current_table->tabela[page_to_replace].m = 0;

    // Encontra um frame livre ou reutilizável
    for (int i = 0; i < RAM_SIZE; i++) {
        if (gmv->frame_table[i] == 0) { // Verifica se o frame está livre
            frame_to_reuse = i;
            break;
        }
    }

    // Atualiza a tabela de frames para indicar que o frame está agora ocupado
    gmv->frame_table[frame_to_reuse] = 1;

    // Atualiza a tabela de páginas para o novo mapeamento
    current_table->tabela[page_to_replace].frame = frame_to_reuse;

    return frame_to_reuse;
}

int subs_NRU(GmvControl *gmv) {
    PageTable current_table = gmv->process_tables[gmv->current_process];
    for (int category = 0; category < 4; category++) {
        for (int i = 0; i < VIRTUAL_SIZE; i++) {
            PageInfo *page = &current_table.tabela[i];
            int r = page->r;
            int m = page->m;
            if ((category == 0 && !r && !m) || (category == 1 && !r && m) || (category == 2 && r && !m) || (category == 3 && r && m)) {
                return replace_page(gmv, i);
            }
        }
    }
    return -1; // Caso não encontre uma página para substituir (deve ser tratado)
}

int subs_2nCh(GmvControl *gmv) {
    static int pointer = 0; // Mantém a posição atual na fila circular
    PageTable current_table = gmv->process_tables[gmv->current_process];
    while (1) {
        PageInfo *page = &current_table.tabela[pointer];
        if (page->r) {
            page->r = 0; // Dá uma segunda chance
            pointer = (pointer + 1) % VIRTUAL_SIZE;
        }
        else {
            return replace_page(gmv, pointer);
        }
    }
    return -1; // Caso não encontre uma página para substituir (deve ser tratado)
}

int subs_LRU(GmvControl *gmv) {
    PageTable current_table = gmv->process_tables[gmv->current_process];
    int min_age = INT_MAX;
    int page_to_replace = -1;
    for (int i = 0; i < VIRTUAL_SIZE; i++) {
        PageInfo *page = &current_table.tabela[i];
        page->age = (page->age >> 1) | (page->r << 7);
        page->r = 0;  // Reseta o bit de referência após o uso
        if (page->age < min_age) {
            min_age = page->age;
            page_to_replace = i;
        }
    }
    if (page_to_replace != -1) {
        return replace_page(gmv, page_to_replace);
    }
    return -1; // Caso não encontre uma página para substituir
}

int subs_WS(GmvControl *gmv, int k) {
    PageTable current_table = gmv->process_tables[gmv->current_process];
    int current_time = get_current_time();
    for (int i = 0; i < VIRTUAL_SIZE; i++) {
        PageInfo *page = &current_table.tabela[i];
        if (current_time - page->last_used > k) {
            return replace_page(gmv, i);
        }
    }
    return -1; // Caso não encontre uma página para substituir
}

int get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}