#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

#include "vm.h"
#include "garbage_collector.hpp"

/****************** Error handling ******************/
void fatal(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    
    fprintf(stderr, "Fatal error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    
    va_end(ap);
    exit(1);
}

void print_help() {
    printf("Usage: ./vm bytecode_file\n\n"
           "This program takes as input the name of a file\n"
           "containing the bytecode to be run from the vm.\n\n");
}


/****************** Data manipulation *****************/

int *stack;
int stack_size = 0;
int top = 0;

void stack_init(int size) {
    stack = (int *) malloc(sizeof(int) * size);
    if (stack == NULL)
        fatal("could not allocate memory for stack");
    stack_size = size;
}

void push_item(int a) {
    if (stack_size == top) {
        stack = (int *) realloc(stack, 2 * stack_size * sizeof(int));
        if (stack == NULL)
            fatal("could not allocate memory for stack");
        stack_size *= 2;
    }
    stack[top++] = a;
}

int pop_item(int *a) {
    if (stack_size == 0)
        return -1;
    *a = stack[--top];
    return 0;
}

int get_item(int *a, int i) {
    if (i >= top)
        return -1;
    *a = stack[top -1 - i];
    return 0;
}


int get_unsigned1(char *pc) {
    return (int) ((unsigned char) pc[1]);
}

int get_unsigned2(char *pc) {
    return (int) ((unsigned char) pc[2] << 8) | ((unsigned char) pc[1]);
}

int get_signed1(char *pc) {
    return (int) ((signed char) pc[1]);
}

int get_signed2(char *pc) {
    return (int) ((signed char) pc[2] << 8) | ((unsigned char) pc[1]);
}

int get_signed4(char *pc) {
    return (get_signed2(pc+2) << 16) | get_unsigned2(pc);
}


/******************* Main function ***********************/

int main(int argc, char **argv) {
    double start_time = (double) clock() / CLOCKS_PER_SEC;
    /* deal with parameters first */
    if (argc != 2)
        print_help();

    /* check if input file exists */
    if (access(argv[1], F_OK) != 0)
        fatal("File %s not found", argv[1]);
    
    /* read input bytecode */
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) 
        fatal("Could not open %s", argv[1]);
    
    char *byte_program = (char *) malloc(sizeof(char) * 65537);
    if (byte_program == NULL) 
        fatal("could not allocate memory for input");
    
    int byte_program_size = 0;
    while (1) {
        int res = read(fd, byte_program + byte_program_size, 65536);
        if (res < 0) 
            fatal("error while reading input");
        if (res == 0)
            break;
        byte_program_size += res;
    }
    byte_program[byte_program_size] = HALT;

    
    stack_init(1);
    
    /************** The following code is based on the slides *************/

    static void *label_table[] = {
        &&halt_label,
        &&jumb_label,
        &&jnz_label,
        &&dup_label,
        &&drop_label,
        &&push4_label,
        &&push2_label,
        &&push1_label,
        &&add_label,
        &&sub_label,
        &&mul_label,
        &&div_label,
        &&mod_label,
        &&eq_label,
        &&ne_label,
        &&lt_label,
        &&gt_label,
        &&le_label,
        &&ge_label,
        &&not_label,
        &&and_label,
        &&or_label,
        &&input_label,
        &&output_label,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        &&clock_label,
        &&cons_label,
        &&hd_label,
        &&tl_label
    };
    
    
    char *pc = byte_program;
    char opcode = pc[0];
    int a,b;
    
    while (1) {

        switch (opcode) {
            case HALT:
            halt_label:
                return 0;
                break;

            case JUMP: {
            jumb_label:
                pc = byte_program + get_unsigned2(pc);
                NEXT_INSTRUCTION;
                break;
            }

            case JNZ: {
            jnz_label:
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - byte_program);
                if (a != 0)
                    pc = byte_program + get_unsigned2(pc);
                else
                    pc += JNZ_SIZE;
                NEXT_INSTRUCTION;
                break;
            }

            case DUP: {
            dup_label:
                pc += DUP_SIZE; // prefetching
                int n;
                int i = get_unsigned1(pc - DUP_SIZE);
                if (get_item(&n,i) < 0)
                    fatal("index out of bounds at %d", pc - DUP_SIZE - byte_program);
                push_item(n);
                NEXT_INSTRUCTION;
                break;
            }

            case DROP: {
            drop_label:
                pc += DROP_SIZE; // prefetching;
                pop_item(&a); // we don't care if stack is empty
                NEXT_INSTRUCTION;
                break;
            }

            case PUSH4:{
            push4_label:
                push_item(get_signed4(pc));
                pc += PUSH4_SIZE;
                NEXT_INSTRUCTION;
                break;
            }
            
            case PUSH2:{
            push2_label:
                push_item(get_signed2(pc));
                pc += PUSH2_SIZE;
                NEXT_INSTRUCTION;
                break;
            }

            case PUSH1:{
            push1_label:
                push_item(get_signed1(pc));
                pc += PUSH1_SIZE;
                NEXT_INSTRUCTION;
                break;
            }

            case ADD: {
            add_label:
                pc += ADD_SIZE; //prefetching
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - ADD_SIZE - byte_program);
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - ADD_SIZE - byte_program);
                push_item(a + b);
                NEXT_INSTRUCTION;
                break;
            }

            case SUB: {
            sub_label:
                pc += SUB_SIZE; //prefetching
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - SUB_SIZE - byte_program);
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - SUB_SIZE - byte_program);
                push_item(a - b);
                NEXT_INSTRUCTION;
                break;
            }

            case MUL: {
            mul_label:
                pc += MUL_SIZE; //prefetching
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - MUL_SIZE - byte_program);
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - MUL_SIZE - byte_program);
                push_item(a * b);
                NEXT_INSTRUCTION;
                break;
            }

            case DIV: {
            div_label:
                pc += DIV_SIZE; //prefetching
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - DIV_SIZE - byte_program);
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - DIV_SIZE - byte_program);
                if (b == 0) {
                    fatal("devision with 0 at %d", pc - DIV_SIZE - byte_program);
                }
                push_item(a/b);
                NEXT_INSTRUCTION;
                break;
            }
            

            case MOD: {
            mod_label:
                pc += MOD_SIZE; //prefetching
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - MOD_SIZE - byte_program);
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - MOD_SIZE - byte_program);
                push_item(a % b);
                NEXT_INSTRUCTION;
                break;
            }

            case EQ: {
            eq_label:
                pc += EQ_SIZE; //prefetching
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - EQ_SIZE - byte_program);
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - EQ_SIZE - byte_program);
                push_item(a == b ? 1 : 0);
                NEXT_INSTRUCTION;
                break;
            }

            case NE: {
            ne_label:
                pc += NE_SIZE; //prefetching
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - NE_SIZE - byte_program);
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - NE_SIZE - byte_program);
                push_item(a != b ? 1 : 0);
                NEXT_INSTRUCTION;
                break;
            }
            
            case LT: {
            lt_label:
                pc += LT_SIZE; //prefetching
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - LT_SIZE - byte_program);
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - LT_SIZE - byte_program);
                push_item(a < b ? 1 : 0);
                NEXT_INSTRUCTION;
                break;
            }

            case GT: {
            gt_label:
                pc += GT_SIZE; //prefetching
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - GT_SIZE - byte_program);
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - GT_SIZE - byte_program);
                push_item(a > b ? 1 : 0);
                NEXT_INSTRUCTION;
                break;
            }

            case LE: {
            le_label:
                pc += LE_SIZE; //prefetching
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - LE_SIZE - byte_program);
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - LE_SIZE - byte_program);
                push_item(a <= b ? 1 : 0);
                NEXT_INSTRUCTION;
                break;
            }

            case GE: {
            ge_label:
                pc += GE_SIZE; //prefetching
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - GE_SIZE - byte_program);
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - GE_SIZE - byte_program);
                push_item(a >= b ? 1 : 0);
                NEXT_INSTRUCTION;
                break;
            }


            case NOT: {
            not_label:
                pc += NOT_SIZE; //prefetching
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - NOT_SIZE - byte_program);
                push_item(a == 0 ? 1 : 0);
                NEXT_INSTRUCTION;
                break;
            }
            
            case AND: {
            and_label:
                pc += AND_SIZE; //prefetching
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - AND_SIZE - byte_program);
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - AND_SIZE - byte_program);
                push_item((a != 0 && b != 0) ? 1 : 0);
                NEXT_INSTRUCTION;
                break;
            }

            case OR: {
            or_label:
                pc += OR_SIZE; //prefetching
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - OR_SIZE - byte_program);
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - OR_SIZE - byte_program);
                push_item((a != 0 || b != 0) ? 1 : 0);
                NEXT_INSTRUCTION;
                break;
            }

            case INPUT: {
            input_label:
                pc += OUTPUT_SIZE; // prefetching
                scanf("%c", &a);
                push_item(a);
                NEXT_INSTRUCTION;
                break;
            }

            case OUTPUT: {
            output_label:
                pc += OUTPUT_SIZE; // prefetching
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - OUTPUT_SIZE - byte_program);
                printf("%c", a);
                NEXT_INSTRUCTION;
                break;
            }

            case CLOCK: {
            clock_label:
                pc += CLOCK_SIZE; // prefetching
                double end_time = (double) clock() /CLOCKS_PER_SEC;
                printf("%0.6lf\n", end_time - start_time);
                NEXT_INSTRUCTION;
                break;
            }
            case CONS: {
            cons_label:
                pc += CONS_SIZE; // prefetching
                int idx = new_cons();
                cons n = get_cons(idx);
                if (pop_item(&b) < 0)
                    fatal("pop from empty stack at %d", pc - CONS_SIZE - byte_program);
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - CONS_SIZE - byte_program);
                n->hd = a;
                n->tl = b;
                push_item(idx);
                NEXT_INSTRUCTION;
                break;
            }
            case HD: {
            hd_label:
                pc += HD_SIZE; // prefetching
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - HD_SIZE - byte_program);
                cons n = get_cons(a);
                push_item(n->hd);
                NEXT_INSTRUCTION;
                break;
            }
            case TL: {
            tl_label:
                pc += TL_SIZE; // prefetching
                if (pop_item(&a) < 0)
                    fatal("pop from empty stack at %d", pc - TL_SIZE - byte_program);
                cons n = get_cons(a);
                push_item(n->tl);
                NEXT_INSTRUCTION;
                break;
            }

            default:
                break;
        }

    }


    return 0;
}