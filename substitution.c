#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include "gmv.h"

int subs_NRU(GmvControl *gmv)
{
    PageTable current_table = gmv->process_tables[gmv->current_process];
    int category = 3;
    int page_to_replace = -1;
    for (int i = 0; i < VIRTUAL_SIZE; i++)
    {
        PageInfo page = current_table.tabela[i];
        if (page.frame == -1)
            continue;
        int r = page.r;
        int m = page.m;
        if (category >= 0 && !r && !m)
        {
            category = 0;
            page_to_replace = i;
            break;
        }
        else if (category >= 1 && !r && m)
        {
            category = 1;
            page_to_replace = i;
        }
        else if (category >= 2 && r && !m)
        {
            category = 2;
            page_to_replace = i;
        }
        else if (category == 3)
        {
            page_to_replace = i;
        }
    }
    return page_to_replace; // Caso não encontre uma página para substituir (deve ser tratado)
}

int subs_2nCh(GmvControl *gmv)
{
    static int pointer; // Mantém a posição atual na fila circular
    PageTable *current_table = gmv->process_tables + gmv->current_process;
    while (1)
    {
        PageInfo *page = current_table->tabela + pointer;
        if (page->frame == -1)
        {
            pointer = (pointer + 1) % VIRTUAL_SIZE;
            continue;
        }
        if (page->r)
        {
            page->r = 0; // Dá uma segunda chance
            pointer = (pointer + 1) % VIRTUAL_SIZE;
        }
        else
        {
            return pointer;
        }
    }
    return -1;
}

int subs_LRU(GmvControl *gmv)
{
    PageTable *current_table = gmv->process_tables + gmv->current_process;
    int oldest_page_id = -1;
    unsigned char oldest = -1;
    for (int i = 0; i < VIRTUAL_SIZE; i++)
    {
        if (current_table->tabela[i].frame == -1)
            continue;
        if (current_table->tabela[i].age < oldest)
        {
            oldest_page_id = i;
            oldest = current_table->tabela[i].age;
        }
    }
    if (oldest_page_id < 0)
    {
        fprintf(stderr, "[LRU] Erro ao achar página mais velha");
        return -1;
    }
    return oldest_page_id; // Caso não encontre uma página para substituir
}

int subs_WS(GmvControl *gmv, int k)
{
    PageTable *current_table = gmv->process_tables + gmv->current_process;
    int current_time = current_table->virtual_time;
    int oldest_page_id;
    int oldest_age = MAX_INT;
    while (oldest_age == MAX_INT)
    {
        for (int i = 0; i < VIRTUAL_SIZE; i++)
        {
            PageInfo *page = current_table->tabela + i;
            if (page->frame == -1)
                continue;
            int age = current_time - page->last_used;
            if (page->r)
            {
                page->last_used = current_table->virtual_time;
                page->r = 0;
                continue;
            }
            if (!page->r && age > (k))
            {
                return i;
            }
            if (!page->r && age <= (k) && age < oldest_age)
            {
                oldest_page_id = i;
                oldest_age = age;
            }
        }
    }
    return oldest_page_id; // Caso não encontre uma página para substituir
}

PageLog *remove_page(GmvControl *gmv)
{
    int page_to_replace;
    switch (gmv->alg)
    {
    case NRU:
        page_to_replace = subs_NRU(gmv);
        break;
    case SecondChance:
        page_to_replace = subs_2nCh(gmv);
        break;
    case LRU:
        page_to_replace = subs_LRU(gmv);
        break;
    case WorkingSet:
        page_to_replace = subs_WS(gmv, gmv->alg_param);
        break;
    default:
        fprintf(stderr, "Erro ao escolher algoritmo de substituição!\n");
        return NULL;
    }
    PageTable *current_table = gmv->process_tables + gmv->current_process;
    int frame_to_reuse = current_table->tabela[page_to_replace].frame;

    int need_to_save = current_table->tabela[page_to_replace].m;
    // Marca a página antiga como não estando mais em um frame
    current_table->tabela[page_to_replace].frame = -1;
    current_table->tabela[page_to_replace].r = 0;
    current_table->tabela[page_to_replace].m = 0;

    // Atualiza a tabela de frames para indicar que o frame está agora livre
    gmv->frame_table[frame_to_reuse] = 0;

    PageLog *log = (PageLog *)malloc(sizeof(PageLog));
    log->frame = frame_to_reuse;
    log->old_page = page_to_replace;
    log->saved = need_to_save;
    log->process = gmv->current_process;
    return log;
}