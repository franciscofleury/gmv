#define PROCESS_N 3
#define RAM_SIZE 16
#define VIRTUAL_SIZE 16

typedef struct page_info
{
    char frame;
    char m;
    char r;
    char age;      // usado para lru
    int last_used; // usado para ws
} PageInfo;

typedef struct page_table
{
    PageInfo tabela[VIRTUAL_SIZE];
    int virtual_time;
} PageTable;

typedef struct gmv_control
{
    PageTable process_tables[PROCESS_N];
    PageInfo *frame_table[RAM_SIZE];
    int current_process;
} GmvControl;