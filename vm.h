#ifndef _VM_DEFS_
#define _VM_DEFS_

#define NEXT_INSTRUCTION \
        goto *(void *)(label_table[*pc])

typedef struct _cons_node{
        int hd;
        int tl;
        char flag;
} *cons;

/* OPCODE Definitions */
#define HALT    0x00
#define JUMP    0X01
#define JNZ     0x02
#define DUP     0x03
#define DROP    0x04
#define PUSH4   0x05
#define PUSH2   0x06
#define PUSH1   0x07
#define ADD     0x08
#define SUB     0x09
#define MUL     0x0a
#define DIV     0x0b
#define MOD     0x0c
#define EQ      0x0d
#define NE      0x0e
#define LT      0x0f
#define GT      0x10
#define LE      0x11
#define GE      0x12
#define NOT     0x13
#define AND     0x14
#define OR      0x15
#define INPUT   0x16
#define OUTPUT  0x17
#define CLOCK   0x2a
#define CONS    0x2b
#define HD      0x2c
#define TL      0x2d


/* OPCODE sizes */
#define HALT_SIZE       1
#define JUMP_SIZE       3
#define JNZ_SIZE        3
#define DUP_SIZE        2
#define DROP_SIZE       1
#define PUSH4_SIZE      5
#define PUSH2_SIZE      3
#define PUSH1_SIZE      2
#define ADD_SIZE        1
#define SUB_SIZE        1
#define MUL_SIZE        1
#define DIV_SIZE        1
#define MOD_SIZE        1
#define EQ_SIZE         1
#define NE_SIZE         1
#define LT_SIZE         1
#define GT_SIZE         1
#define LE_SIZE         1
#define GE_SIZE         1
#define NOT_SIZE        1
#define AND_SIZE        1
#define OR_SIZE         1
#define INPUT_SIZE      1
#define OUTPUT_SIZE     1
#define CLOCK_SIZE      1
#define CONS_SIZE       1
#define HD_SIZE         1
#define TL_SIZE         1

void fatal(const char *fmt, ...);

#endif // _VM_DEFS_