#include <stdlib.h>
#include <stdio.h>
#include "private_gmv.h"

PageLog *page_fault(GmvControl *gmv, int page);

GmvControl *get_gmv()
{
    static GmvControl *gmv;
    if (gmv == NULL)
    {
        gmv = (GmvControl *)malloc(sizeof(GmvControl));
        for (int i = 0; i < PROCESS_N; i++)
        {
            gmv->process_tables[i].virtual_time = 0;
            for (int j = 0; j < VIRTUAL_SIZE; j++)
            {
                gmv->process_tables[i].tabela[j].frame = -1;
                gmv->process_tables[i].tabela[j].age = 0;
                gmv->process_tables[i].tabela[j].last_used = 0;
                gmv->process_tables[i].tabela[j].m = 0;
                gmv->process_tables[i].tabela[j].r = 0;
            }
        }
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

int set_param_alg(GmvControl *gmv, SUB_ALG alg, int alg_param)
{
    gmv->alg = alg;
    gmv->alg_param = alg_param;
    return 1;
}

int update_guard(int virtual_time)
{
    if (virtual_time == 0)
        return 0;
    if (virtual_time % UPDATE_RATIO != 0)
        return 0;

    return 1;
}

int delete_guard(int virtual_time)
{
    if (virtual_time == 0)
        return 0;
    if (virtual_time % DELETE_RATIO != 0)
        return 0;

    return 1;
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

    if (!delete_guard(current_table->virtual_time))
        return 0;

    for (int i = 0; i < VIRTUAL_SIZE; i++)
    {
        PageInfo *page = current_table->tabela + i;

        page->r = 0;
    }
    return 1;
}

// Essa função é chamada sempre que um processo faz um acesso/escrita à memória
PageLog *get_page(GmvControl *gmv, int page, char mode)
{
    PageTable *tabela_atual = gmv->process_tables + gmv->current_process;
    tabela_atual->tabela[page].r = 1;
    tabela_atual->tabela[page].m = (mode == 'W') ? 1 : tabela_atual->tabela[page].m;
    update_page_info(gmv);

    PageLog *log = NULL;
    if (tabela_atual->tabela[page].frame == -1)
    {
        log = page_fault(gmv, page);
        tabela_atual->tabela[page].frame = log->frame;
        gmv->frame_table[log->frame] = 1;
    }

    reset_r(gmv);

    tabela_atual->tabela[page].r = 1;
    return log;
}

PageLog *page_fault(GmvControl *gmv, int page)
{
    PageTable current_table = gmv->process_tables[gmv->current_process];
    PageLog *log;
    int i = 0;
    while (i < RAM_SIZE)
    {
        if (gmv->frame_table[i] == 0)
        {
            current_table.tabela[page].frame = i;
            log = (PageLog *)malloc(sizeof(PageLog));
            log->frame = i;
            log->new_page = page;
            log->old_page = -1;
            log->process = gmv->current_process;
            return log;
        }
        i++;
    }

    // Chamar algoritmo de substituição
    log = remove_page(gmv);
    log->new_page = page;
    current_table.tabela[page].frame = log->frame;
    return log;
}

int verify_integrity(GmvControl *gmv)
{
    int used_frames[RAM_SIZE];
    for (int i = 0; i < RAM_SIZE; i++)
    {
        used_frames[i] = 0;
    }
    for (int i = 0; i < PROCESS_N; i++)
    {
        for (int j = 0; j < VIRTUAL_SIZE; j++)
        {
            int f = gmv->process_tables[i].tabela[j].frame;
            if (f == -1)
                continue;
            if (used_frames[f])
            {
                fprintf(stderr, "Erro no gerenciamento de memória, 2 entradas apontam para o mesmo frame!\n");
                return 0;
            }

            used_frames[f] = 1;
        }
    }
    return 1;
}