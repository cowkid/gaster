#include "blib.h"
#include "gms_types.h"

/* This file is the disassembler for gm:s bytecode.

// We use a few different names (because it makes more sense)
// Conv is really a type-cast instruction, so we use cast instead.
// Second 'remainder' instruction is modulo.

// The Slt, etc instructions don't have the S prefix.

char* gm_string_get(int index);

#define CAST  0x03 // 2T
#define MUL   0x04 // 2T
#define DIV   0x05 // 2T
#define REMN  0x06 // 2T
#define MOD   0x07 // 2T
#define ADD   0x08 // 2T
#define SUB   0x09 // 2T
#define AND   0x0a // 2T
#define OR    0x0b // 2T
// Arithmetic Negation. As in, 0-(type)Stack. NOT YET DOC'D ON RAWR. Info obtained by messaging Mirrawrs.
#define NEG   0x0d // 2T??? (It's a one operand instruction.)
#define NOT   0x0e // 2T
#define LT    0x11 // 2T
#define LE    0x12 // 2T
#define EQ    0x13 // 2T
#define NE    0x14 // 2T
#define GE    0x15 // 2T
#define GT    0x16 // 2T
#define STOR  0x41 // CUST Pops stack to location
#define DUP   0x82 // 1T Duplicates stack top
#define RET   0x9d // 1T Returns, pop 1 off stack
#define EXIT  0x9e // 1T Returns, keeps stack
#define POP   0x9f // 1T Pops from stack, without storing.
#define BX    0xb7 // GT Branch to location.
#define BT    0xb8 // GT Branch if pop() == true.
#define BF    0xb9 // GT Branch if pop() == false.
#define PUSHA 0xbb // GT Push state.
#define POPA  0xbc // GT Pop all.
#define PUSH  0xc0 // CUST Push value to stack.
#define CALL  0xda // CUST Call.
#define BRK   0xff // Guard. Probably not meant to be executed.

#define VAL24(buf) ((((uint32_t*)buf)[0])&0x00ffffff)

#define Type(x, b) 	char* x = "?"; \
	switch(b) { \
		case 0x00: \
			x = "d"; \
			break; \
		case 0x01: \
			x = "f"; \
			break; \
		case 0x02: \
			x = "d"; \
			break; \
		case 0x03: \
			x = "q"; \
			break; \
		case 0x04: \
			x = "b"; \
			break; \
		case 0x05: \
			x = "v"; \
			break; \
		case 0x06: \
			x = "s"; \
			break; \
		case 0x07: \
			x = "i"; \
			break; \
		case 0x0f: \
			x = "w"; \
			break; \
	}

typedef enum {
	gm_Double   = 0,
	gm_Float    = 1,
	gm_Dword    = 2,
	gm_Qword    = 3,
	gm_Bool     = 4,
	gm_Var      = 5,
	gm_String   = 6,
	gm_Instance = 7,
	gm_Word     = 0x0f
} gm_data_type;

typedef enum {
	gm_StackGlob = 0,
	gm_Self      = -1,
	gm_Other     = -2,
	gm_All       = -3,
	gm_Noone     = -4,
	gm_Global    = -5
} gm_inst_type;

typedef enum {
	gm_Array     = 0,
	gm_StackTop  = 0x80,
	gm_Normal    = 0xa0
} gm_var_type;

struct opcode_info {
	// Points to raw opcode.
	uint8_t* ref;
	int len; // Length in BLOCKS, not bytes. 1block=4bytes.

	// Metadata for disassembly.
	int is_jump;
	uint32_t label_num;
	uint32_t jump_to;

	// What offset in the script this is at.
	uint32_t offset;

	// The opcode.
	uint8_t op;

	// Address the opcode specifies, if used.
	uint32_t addr;

	// Argument type 1, if used.
	uint8_t type1;

	// Argument type 2, if used.
	uint8_t type2;

	// Instance type if used.
	int16_t instance;

	// Reference location, if used.
	uint32_t ref_loc;

	// Reference type, if used.
	uint8_t  ref_type;

	// Reference location, if used.
	uint32_t var_loc;

	// Reference type, if used.
	uint8_t  var_type;

	// Push vars.
	double*  d;
	float*   f;
	int64_t* i8;
	int32_t* i4;
	int16_t* i2;

}__attribute__((packed));


#define DOUBLE_ARG \
	ops[ops_count].type1 = buf[2] & 0x0f; \
	ops[ops_count].type2 = (buf[2] >> 4) & 0x0f;

#define DOUBLE_ARG_AT(n) \
	ops[ops_count].type1 = buf[n] & 0x0f; \
	ops[ops_count].type2 = (buf[n] >> 4) & 0x0f;

#define SINGLE_ARG \
	ops[ops_count].type1   = buf[2];

#define GOTO_ARG \
	ops[ops_count].addr   = off + VAL24(buf) * 4;

#define REFERENCE_ARG \
	ops[ops_count].ref_type  = buf[4]; \
	ops[ops_count].ref_loc = VAL24((&buf[5]));


char*  argument_strs[16] = {
	"double", // 0
	"float", // 1
	"dword", // 2
	"qword", // 3
	"bool", // 4
	"var", // 5
	"str", // 6
	"inst", // 7
	"", // 8
	"", // 9
	"", // a
	"", // b
	"", // c
	"", // d
	"", // e
	"word", // f
};

#define DOUBLE_ARG_PRINT \
	fprintf(out, "%s, %s", argument_strs[ops[i].type1], argument_strs[ops[i].type2] );

//#define DOUBLE_ARG_PRINT \
//	fprintf(out, "%01hhx %01hhx", ops[i].type1, ops[i].type2 );

#define SINGLE_ARG_PRINT \
	fprintf(out, "%s", argument_strs[ops[i].type1] );

//#define SINGLE_ARG_PRINT \
//	fprintf(out, "%01hhx", ops[i].type1 );

#define OFFSET_PRINT \
	fprintf(out, "label%u", ops[i].jump_to );

void disassemble_pass2(FILE* out, struct opcode_info* ops, int ops_count) {
	printf(" > Checking jumps...\n");
	// Set the label property on destinations.
	for(int i = 0; i < ops_count; i++) {
		if (ops[i].op >= BX && ops[i].op <= POPA) {
			// Jumps have a destination, and that destination is a label.
			for(int j = 0; j < ops_count; j++) {
				if(ops[i].addr == ops[j].offset) {
					ops[j].is_jump = 1;
				}
			}
		}
	}

	printf(" > Computing labels...\n");
	// Index labels.
	uint32_t jump_num = 1;
	for(int i = 0; i < ops_count; i++) {
		if (ops[i].is_jump) {
			ops[i].label_num = jump_num;
			jump_num++;
		}
	}

	printf("   >> Total labels: %d\n", jump_num);

	printf(" > Indexing jumps...\n");
	int jump_count = 0;
	// Index jumps.
	for(int i = 0; i < ops_count; i++) {
		if (ops[i].op >= BX && ops[i].op <= POPA) {
			for(int j = 0; j < ops_count; j++) {
				if(ops[i].addr == ops[j].offset) {
					ops[i].jump_to = ops[j].label_num;
					jump_count++;
				}
			}
		}
	}
	printf("   >> Total jumps: %d\n", jump_count);

	printf(" > Disassembling...\n");
	for(int i=0; i < ops_count; i++) {
		if (ops[i].is_jump)
			fprintf(out, "label%d:\n", ops[i].label_num);

		fprintf(out, "\t");

		switch(ops[i].op) {
			case CAST:
				fprintf(out, "cast ");
				DOUBLE_ARG_PRINT
				break;
			case MUL:
				fprintf(out, "mul ");
				DOUBLE_ARG_PRINT
				break;
			case DIV:
				fprintf(out, "div ");
				DOUBLE_ARG_PRINT
				break;
			case REMN:
				fprintf(out, "remn ");
				DOUBLE_ARG_PRINT
				break;
			case MOD:
				fprintf(out, "mod ");
				DOUBLE_ARG_PRINT
				break;
			case ADD:
				fprintf(out, "add ");
				DOUBLE_ARG_PRINT
				break;
			case SUB:
				fprintf(out, "sub ");
				DOUBLE_ARG_PRINT
				break;
			case AND:
				fprintf(out, "and ");
				DOUBLE_ARG_PRINT
				break;
			case OR:
				fprintf(out, "or ");
				DOUBLE_ARG_PRINT
				break;
			case NEG:
				fprintf(out, "neg ");
				DOUBLE_ARG_PRINT
				break;
			case NOT:
				fprintf(out, "not ");
				DOUBLE_ARG_PRINT
				break;
			case LT:
				fprintf(out, "lt ");
				DOUBLE_ARG_PRINT
				break;
			case LE:
				fprintf(out, "le ");
				DOUBLE_ARG_PRINT
				break;
			case EQ:
				fprintf(out, "eq ");
				DOUBLE_ARG_PRINT
				break;
			case NE:
				fprintf(out, "ne ");
				DOUBLE_ARG_PRINT
				break;
			case GE:
				fprintf(out, "ge ");
				DOUBLE_ARG_PRINT
				break;
			case GT:
				fprintf(out, "gt ");
				DOUBLE_ARG_PRINT
				break;
			case STOR:
				fprintf(out, "stor ");
				switch(ops[i].instance) {
					case 0:
						fprintf(out, "stack, %s:%s", argument_strs[ops[i].type1], argument_strs[ops[i].type2]);
						break;
					case -1:
						fprintf(out, "self, %s:%s", argument_strs[ops[i].type1], argument_strs[ops[i].type2]);
						break;
					case -2:
						fprintf(out, "other, %s:%s", argument_strs[ops[i].type1], argument_strs[ops[i].type2]);
						break;
					case -3:
						fprintf(out, "all, %s:%s", argument_strs[ops[i].type1], argument_strs[ops[i].type2]);
						break;
					case -4:
						fprintf(out, "none, %s:%s", argument_strs[ops[i].type1], argument_strs[ops[i].type2]);
						break;
					case -5:
						fprintf(out, "global, %s:%s", argument_strs[ops[i].type1], argument_strs[ops[i].type2]);
						break;
					default:
						fprintf(out, "object:%04x, %s:%s", ops[i].var_loc, argument_strs[ops[i].type1], argument_strs[ops[i].type2]);
						break;
				}
				break;
			case DUP:
				fprintf(out, "dup ");
				SINGLE_ARG_PRINT
				break;
			case RET:
				fprintf(out, "ret ");
				SINGLE_ARG_PRINT
				break;
			case EXIT:
				fprintf(out, "exit ");
				SINGLE_ARG_PRINT
				break;
			case POP:
				fprintf(out, "pop ");
				SINGLE_ARG_PRINT
				break;
			case BX:
				fprintf(out, "bx ");
				OFFSET_PRINT
				break;
			case BT:
				fprintf(out, "bt ");
				OFFSET_PRINT
				break;
			case BF:
				fprintf(out, "bf ");
				OFFSET_PRINT
				break;
			case PUSHA:
				fprintf(out, "pusha ");
				OFFSET_PRINT
				break;
			case POPA:
				fprintf(out, "popa ");
				OFFSET_PRINT
				break;
			case PUSH:
				fprintf(out, "push ");
				SINGLE_ARG_PRINT
				switch(ops[i].type1) {
					case 0x0: // Double
						fprintf(out, ":%lf", ops[i].d[0]);
						break;
					case 0x1: // Float
						fprintf(out, ":%f", ops[i].f[0]);
						break;
					case 0x2: // Dword
						fprintf(out, ":%d", ops[i].i4[0]);
						break;
					case 0x3: // Qword
						fprintf(out, ":%ld", ops[i].i8[0]);
						break;
					case 0x4: // Dword
						fprintf(out, ":%d", ops[i].i4[0]);
						break;
					case 0x6: // String
						fprintf(out, ":%d->\"%s\"", ops[i].i4[0], gm_string_get(ops[i].i4[0]));
						break;
					case 0xf: // Word
						fprintf(out, ":%hd", ops[i].i2[0]);
						break;
				}
				break;
			case CALL:
				fprintf(out, "call ");
				break;
			case BRK:
				fprintf(out, "brk ");
				break;
			default:
				fprintf(out, "; unknown opcode: %02hhx @ offset %08x", ops[i].op, ops[i].offset);
				break;
		}

		fprintf(out, "\n");
		fflush(out);
	}

	free(ops);
}

int unknown_ops = 0;

void disassemble_new(uint8_t* buf, int32_t len, FILE* out, int off, struct opcode_info* ops, int ops_count) {
	if (ops_count == 0)
		unknown_ops = 0;

	if (len < 0) {
		printf(" > WARNING - Misaligned opcodes?\n");
		fprintf(out, "; Warning - output was not opcode-aligned.\n");
	}
	if (len <= 0) {
		printf(" > Total opcodes: %d\n", ops_count);
		printf("Disassembly -> Pass 2\n");
		// No opcodes left. Proceed to pass 2.
		disassemble_pass2(out, ops, ops_count);
		return;
	}

	// Pass 1 - Generate opcode array.
	ops = realloc(ops, (ops_count+1) * sizeof(struct opcode_info));

	memset(& ops[ops_count], 0, sizeof(struct opcode_info));

	// We use recursion here. Deal with it.
	ops[ops_count].op = buf[3];
	ops[ops_count].len = 1;
	ops[ops_count].offset = off;

	ops[ops_count].d  = &buf[4];
	ops[ops_count].f  = &buf[4];
	ops[ops_count].i8 = &buf[4];
	ops[ops_count].i4 = &buf[4];
	ops[ops_count].i2 = &buf[0];

	// Determine data extraction based on type.
	switch(ops[ops_count].op) {
		case CAST:
			DOUBLE_ARG
			break;
		case MUL:
			DOUBLE_ARG
			break;
		case DIV:
			DOUBLE_ARG
			break;
		case REMN:
			DOUBLE_ARG
			break;
		case MOD:
			DOUBLE_ARG
			break;
		case ADD:
			DOUBLE_ARG
			break;
		case SUB:
			DOUBLE_ARG
			break;
		case AND:
			DOUBLE_ARG
			break;
		case OR:
			DOUBLE_ARG
			break;
		case NEG:
			DOUBLE_ARG
			break;
		case NOT:
			DOUBLE_ARG
			break;
		case LT:
			DOUBLE_ARG
			break;
		case LE:
			DOUBLE_ARG
			break;
		case EQ:
			DOUBLE_ARG
			break;
		case NE:
			DOUBLE_ARG
			break;
		case GE:
			DOUBLE_ARG
			break;
		case GT:
			DOUBLE_ARG
			break;
		case STOR:
			DOUBLE_ARG
			ops[ops_count].instance    = ((uint16_t*)buf+4)[0];
			// One more block.
			ops[ops_count].len += 1;
			break;
		case DUP:
			SINGLE_ARG
			break;
		case RET:
			SINGLE_ARG
			break;
		case EXIT:
			SINGLE_ARG
			break;
		case POP:
			SINGLE_ARG
			break;
		case BX:
			GOTO_ARG
			break;
		case BT:
			GOTO_ARG
			break;
		case BF:
			GOTO_ARG
			break;
		case PUSHA:
			GOTO_ARG
			break;
		case POPA:
			GOTO_ARG
			break;
		case PUSH:
			SINGLE_ARG
			if (ops[ops_count].type1 != 0x0f) // Not a short? Add one.
				ops[ops_count].len += 1;
			if (ops[ops_count].type1 == 0x00 || ops[ops_count].type1 == 0x03) // Long type? Add one.
				ops[ops_count].len += 1;
			break;
		case CALL:
			ops[ops_count].len += 1;
			break;
		case BRK:
			break;
		default:
			if (unknown_ops == 0) {
				fprintf(out, "; Warning - Unknown opcodes in bytecode.\n");
				printf(" > Unknown opcodes in input.\n");
			}
			unknown_ops = 1;
			break;
	}

	disassemble_new(buf+(ops[ops_count].len * 4), len-(ops[ops_count].len * 4), out, off+(ops[ops_count].len * 4), ops, ops_count + 1);
}
