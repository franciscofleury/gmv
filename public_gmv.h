#include "gmv.h"

GmvControl *get_gmv();

int get_page(GmvControl *gmv, int page, char mode);

int set_alg(GmvControl *gmv, SUB_ALG alg);

int set_param_alg(GmvControl *gmv, SUB_ALG alg, int alg_param);