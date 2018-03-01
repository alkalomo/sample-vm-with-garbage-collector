#ifndef _GARBAGE_COLLECTOR_
#define _GARBAGE_COLLECTOR_

#include "vm.h"

using namespace std;

extern int *stack;
extern int top;

int new_cons();
cons get_cons(int idx);
void mark_and_sweep();
void mark();
void dfs(int n);
void sweep();

#endif // _GARBAGE_COLLECTOR_