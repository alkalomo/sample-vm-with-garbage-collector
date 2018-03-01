#include <stdlib.h>

#include <vector>

#include "garbage_collector.hpp"
#include "vm.h"

using namespace std;

typedef struct _list_node{
    int idx;
    struct _list_node * next;
} list_node;

vector<int> dfs_stack;
vector<cons> free_mem;

list_node *free_list = NULL;

char flag = 0;
int total_cons = 0;

list_node *new_list_node(int idx) {
    list_node *n = (list_node *) malloc(sizeof(*n));
    if (n == NULL)
        fatal("out of memory");
    n->idx = idx;
    n->next = NULL;
    return n;
}

int is_free_list_empty() {
    return (free_list == NULL);
}

void add_to_free_list(int idx) {
    list_node *n = new_list_node(idx);
    if (is_free_list_empty()) {
        free_list = n;
    }
    else {
        n->next = free_list;
        free_list = n;
    }
}

int get_next_free_idx() {
    if (is_free_list_empty())
        return -1;
    list_node *tmp = free_list;
    free_list = tmp->next;
    int res = tmp->idx;
    free(tmp);
    return res;
}

int add_to_free_mem(cons n) {
    if (is_free_list_empty()) {
        free_mem.push_back(n);
        return free_mem.size() - 1;
    }
    else {
        int idx = get_next_free_idx();
        free_mem[idx] = n;
        return idx;
    }
}


int new_cons(){
    if (total_cons == 4194304) 
        mark_and_sweep();
    if (total_cons == 4194304) 
        fatal("out of memory");
    cons n = (cons) malloc(sizeof(*n));
    if (n == NULL)
        fatal("out of memory");
    n->flag = flag;
    total_cons++;
    return add_to_free_mem(n);
}

cons get_cons(int idx) {
    return free_mem[idx];
}

int is_cons_in_range(int a) {
    return ( a < free_mem.size() && a >= 0 && free_mem[a] != NULL);
}

void mark_and_sweep() {
    flag = (flag + 1) % 2;
    // begin mark phase
    mark();
    // disp_stack();
    // disp();
    sweep();
    // disp();
}

void mark() {
    int i;
    for (i = 0; i< top; i++)
        dfs(stack[i]);
}

void dfs(int n) {
    //dfs_stack.clear();
    while(1){
        if (is_cons_in_range(n)) {
            cons tmp = get_cons(n);
            if (tmp->flag != flag) {
                tmp->flag = flag;
                dfs_stack.push_back(tmp->tl);
                dfs_stack.push_back(tmp->hd);
            }
        }
        if (dfs_stack.size() == 0)
            break;
        n = dfs_stack.back();
        dfs_stack.pop_back();
    }  
}

void sweep() {
    int removed_cons = 0;
    for (int i = 0; i < free_mem.size(); i++) {
        if (free_mem[i] == NULL)
            continue;
        if (free_mem[i]->flag != flag) {
            removed_cons++;
            add_to_free_list(i);
            cons tmp = free_mem[i];
            free_mem[i] = NULL;
            free(tmp);
        }
    }
    total_cons -= removed_cons;
}
