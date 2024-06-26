#include <stdlib.h>
#include <stdio.h>
#include "private_gmv.h"

int get_gmv()
{
    static GmvControl *gmv;
    if (gmv == NULL)
    {
        gmv = (GmvControl *)malloc(sizeof(GmvControl));
        gmv->alg = DEFAULT_ALG;
        gmv->alg_param = -1;
    }
    return gmv;
}

int set_alg(GmvControl *gmv, SUB_ALG alg)
{
    if (alg == WorkingSet)
    {
        fprintf(stderr, "Working Set necessita de parâmetro!");
        return 0;
    }
    gmv->alg = alg;
    return 1;
}

int set_alg(GmvControl *gmv, SUB_ALG alg, int alg_param)
{
    if (alg != WorkingSet)
    {
        fprintf(stderr, "Este algoritmo não necessita de parâmetro!");
        return 0;
    }
    gmv->alg = alg;
    gmv->alg_param = alg_param;
}

int update_guard(int virtual_time)
{
    if (virtual_time == 0)
        return 0;
    if (virtual_time % UPDATE_RATIO != 0)
        return 0;
}

int update_page_info(GmvControl *gmv)
{
    PageTable *current_table = gmv->process_tables + gmv->current_process;

    if (!update_guard(++current_table->virtual_time))
        return 0;

    int r_count = 0;
    for (int i = 0; i < VIRTUAL_SIZE; i++)
    {
        PageInfo *page = current_table->tabela + i;
        page->age = page->age >> 1;
        if (page->r)
        {
            page->age = page->age | 0x80;
            page->last_used = current_table->virtual_time;
            r_count++;
        }
    }

    return r_count;
}

int reset_r(GmvControl *gmv)
{
    PageTable *current_table = gmv->process_tables + gmv->current_process;

    if (!update_guard(current_table->virtual_time))
        return 0;

    for (int i = 0; i < VIRTUAL_SIZE; i++)
    {
        PageInfo *page = current_table->tabela + i;

        page->r = 0;
    }
    return 1;
}

// Essa função é chamada sempre que um processo faz um acesso/escrita à memória
int get_page(GmvControl *gmv, int page, char mode)
{
    update_page_info(gmv);
    PageTable *tabela_atual = gmv->process_tables + gmv->current_process;

    tabela_atual->tabela[page].m = (mode == 'w') ? 1 : tabela_atual->tabela[page].m;
    tabela_atual->tabela[page].r = (mode == 'r') ? 1 : tabela_atual->tabela[page].r;

    int frame;
    if (tabela_atual->tabela[page].frame != -1)
    {
        frame = tabela_atual->tabela[page].frame;
    }
    else
    {
        frame = page_fault(gmv, page);
        tabela_atual->tabela[page].frame = frame;
        gmv->frame_table[frame] = 0;
    }

    reset_r(gmv);

    return frame;
}

int page_fault(GmvControl *gmv, int page)
{
    PageTable current_table = gmv->process_tables[gmv->current_process];
    int i = 0;
    while (i < RAM_SIZE)
    {
        if (gmv->frame_table[i] == 0)
        {
            current_table.tabela[page].frame = i;
            return i;
        }
    }

    // Chamar algoritmo de substituição
    int empty_frame = remove_page(gmv);
    return empty_frame;
}
