#define PROCESS_N 3
#define RAM_SIZE 16
#define VIRTUAL_SIZE 16

typedef struct page_info {
    char frame;
    char m;
    char r;
} PageInfo;

typedef struct page_table {
    PageInfo tabela[VIRTUAL_SIZE];
} PageTable;

typedef struct gmv_control {
    PageTable process_tables[PROCESS_N];
    int frame_table[RAM_SIZE];
    int current_process;
} GmvControl;