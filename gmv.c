#include <stdlib.h>
#include <stdio.h>
#include "private_gmv.h"

int page_fault(GmvControl *gmv, int page);

GmvControl *get_gmv()
{
    static GmvControl *gmv;
    if (gmv == NULL)
    {
        gmv = (GmvControl *)malloc(sizeof(GmvControl));
        gmv->alg = DEFAULT_ALG;
        gmv->alg_param = -1;
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
    if (alg != WorkingSet)
    {
        fprintf(stderr, "Este algoritmo não necessita de parâmetro!");
        return 0;
    }
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

    tabela_atual->tabela[page].r = 1;
    tabela_atual->tabela[page].m = (mode == 'w') ? 1 : tabela_atual->tabela[page].m;
    printf("getting page\n");
    int frame;
    if (tabela_atual->tabela[page].frame != -1)
    {
        frame = tabela_atual->tabela[page].frame;
        printf("found frame %d\n", frame);
    }
    else
    {
        printf("calling page fault\n");
        frame = page_fault(gmv, page);
        tabela_atual->tabela[page].frame = frame;
        gmv->frame_table[frame] = 1;
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
        i++;
    }

    // Chamar algoritmo de substituição
    printf("frame is full, removing...\n");
    int empty_frame = remove_page(gmv);
    printf("returned frame: %d\n", empty_frame);
    return empty_frame;
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