#include "gmv.h"

GmvControl *get_gmv();

PageLog *get_page(GmvControl *gmv, int page, char mode);

int set_alg(GmvControl *gmv, SUB_ALG alg);

int set_param_alg(GmvControl *gmv, SUB_ALG alg, int alg_param);

int verify_integrity(GmvControl *gmv);