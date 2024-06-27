#define PROCESS_N 3
#define RAM_SIZE 16
#define VIRTUAL_SIZE 16
#define DEFAULT_ALG NRU
#define MAX_INT 50000
#define UPDATE_RATIO 1
#define DELETE_RATIO 4

typedef enum SUB_ALG
{
    NRU,
    LRU,
    SecondChance,
    WorkingSet
} SUB_ALG;

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
    int frame_table[RAM_SIZE];
    int current_process;
    SUB_ALG alg;
    int alg_param;
} GmvControl;

typedef struct page_log
{
    int old_page;
    int new_page;
    int frame;
    int saved;
    int process;
} PageLog;