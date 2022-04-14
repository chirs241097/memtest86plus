#ifndef DEBUG_H
#define DEBUG_H
#include "print.h"
#include "keyboard.h"

#define STRIFY(x) #x
#define STR(x) STRIFY(x)
#define DEBUGP \
    prints(0, 0, "@" __FILE__ ":" STR(__LINE__) " Press any key to continue"); \
    while (get_key() == 0);

#endif //DEBUG_H
