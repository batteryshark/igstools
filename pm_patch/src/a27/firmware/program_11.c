#include <string.h>

#include "../emulator.h"

typedef struct _A27_SEL_SCREEN_STATE{
    unsigned short subcmd;
    unsigned short val;
}A27SelScreenState,*PA27SelScreenState;

// Program 11: Title(Coin) Screen State
void A27_Program_11(PA27WriteMessage req, PA27ReadMessage res){
    res->header.system_mode = A27_MODE_SCREEN_COIN;
    res->header.data_size = sizeof(A27SelScreenState);
    PA27SelScreenState req_ss = (PA27SelScreenState)req->data;
    PA27SelScreenState res_ss = (PA27SelScreenState)res->data;
    res_ss->val = req_ss->val;
    switch(req_ss->subcmd){
        case 0:
            res_ss->subcmd = 1;
            break;
        case 1:
            res_ss->subcmd = 2;
            break;
        case 2:
        case 3:
            res_ss->subcmd = 3;
            break;
        default:
            break;
    }
}

