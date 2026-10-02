/*
 * assembler.c
 *
 * A two-pass assembler for the 16-bit single-cycle CPU described in ISA.md.
 *
 * Usage:
 *   ./assembler input.asm output.hex
 *
 * Output is a Logisim-Evolution "v2.0 raw" hex image, loadable directly
 * into the ROM component (right-click ROM -> Load Image...).
 *
 * Build:
 *   gcc -std=c99 -Wall -Wextra -o assembler assembler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32) || defined(_MSC_VER)
  #include <string.h>
  #define strcasecmp _stricmp
#else
  #include <strings.h> /* strcasecmp on POSIX (Linux/macOS) */
#endif

#define MAX_LINE      256
#define MAX_LABELS    256
#define MAX_WORDS     65536
#define MAX_LABEL_LEN 32

/* ---------- label table ---------- */

typedef struct {
    char name[MAX_LABEL_LEN];
    int  address;
} Label;

static Label labels[MAX_LABELS];
static int   label_count = 0;

static void add_label(const char *name, int address) {
    int i;
    for (i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            fprintf(stderr, "error: duplicate label '%s'\n", name);
            exit(1);
        }
    }
    if (label_count >= MAX_LABELS) {
        fprintf(stderr, "error: too many labels\n");
        exit(1);
    }
    strncpy(labels[label_count].name, name, MAX_LABEL_LEN - 1);
    labels[label_count].name[MAX_LABEL_LEN - 1] = '\0';
    labels[label_count].address = address;
    label_count++;
}

static int find_label(const char *name) {
    int i;
    for (i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) return labels[i].address;
    }
    fprintf(stderr, "error: undefined label '%s'\n", name);
    exit(1);
}

/* ---------- output image ---------- */

static unsigned short memory[MAX_WORDS];
static int word_count = 0;

static void emit(unsigned short w) {
    if (word_count >= MAX_WORDS) {
        fprintf(stderr, "error: program exceeds 64K words\n");
        exit(1);
    }
    memory[word_count++] = w;
}

/* ---------- small string helpers ---------- */

static char *trim(char *s) {
    char *end;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

static void strip_comment(char *s) {
    char *p = strchr(s, ';');
    if (!p) p = strchr(s, '#');
    if (p) *p = '\0';
}

static int is_register(const char *tok, int *out) {
    int n;
    if ((tok[0] != 'r' && tok[0] != 'R') || tok[1] == '\0') return 0;
    for (n = 1; tok[n]; n++) if (!isdigit((unsigned char)tok[n])) return 0;
    n = atoi(tok + 1);
    if (n < 0 || n > 15) {
        fprintf(stderr, "error: register out of range: %s\n", tok);
        exit(1);
    }
    *out = n;
    return 1;
}

static int parse_int(const char *tok) {
    return (int)strtol(tok, NULL, 0); /* supports decimal, 0x hex, leading - */
}

/* splits "MOV r1, r2" style operand lists on commas/whitespace */
static int split_operands(char *s, char *out[], int max_out) {
    int n = 0;
    char *tok = strtok(s, ", \t");
    while (tok && n < max_out) {
        out[n++] = tok;
        tok = strtok(NULL, ", \t");
    }
    return n;
}

/* ---------- native instruction encoders ---------- */

static unsigned short enc_R(int op, int rd, int rs1, int rs2) {
    return (unsigned short)((op << 12) | (rd << 8) | (rs1 << 4) | rs2);
}
static unsigned short enc_I(int op, int rd, int rs1, int imm4) {
    return (unsigned short)((op << 12) | (rd << 8) | (rs1 << 4) | (imm4 & 0xF));
}
static unsigned short enc_LOAD(int op, int rd, int rs1) {
    return (unsigned short)((op << 12) | (rd << 8) | (rs1 << 4));
}
static unsigned short enc_STORE(int op, int rs1, int rs2) {
    return (unsigned short)((op << 12) | (rs1 << 4) | rs2);
}
static unsigned short enc_BRANCH(int op, int offset4) {
    return (unsigned short)((op << 12) | (offset4 & 0xF));
}
static unsigned short enc_JUMP(int op, int target12) {
    return (unsigned short)((op << 12) | (target12 & 0xFFF));
}

static void check_imm(int val, int lo, int hi, const char *what) {
    if (val < lo || val > hi) {
        fprintf(stderr, "error: %s value %d out of range [%d, %d]\n", what, val, lo, hi);
        exit(1);
    }
}

/* ---------- LI chunk count (needed by both passes) ---------- */

static int li_chunk_count(int imm) {
    int mag = imm < 0 ? -imm : imm;
    int step = 7; /* largest magnitude a single ADDI step can add */
    int chunks = (mag + step - 1) / step;
    return chunks; /* 0 if imm == 0 */
}

/* ---------- word-count of one source line (pass 1) ---------- */

static int words_for_line(const char *mnem, char *operand_str) {
    char ops_copy[MAX_LINE];
    char *ops[4];
    int n;

    strncpy(ops_copy, operand_str, MAX_LINE - 1);
    ops_copy[MAX_LINE - 1] = '\0';
    n = split_operands(ops_copy, ops, 4);

    if (!strcasecmp(mnem, "ADD") || !strcasecmp(mnem, "SUB") ||
        !strcasecmp(mnem, "AND") || !strcasecmp(mnem, "XOR") ||
        !strcasecmp(mnem, "OR")  || !strcasecmp(mnem, "SLL") ||
        !strcasecmp(mnem, "SRL") || !strcasecmp(mnem, "SRA") ||
        !strcasecmp(mnem, "ADDI")) return 1;
    if (!strcasecmp(mnem, "LW") || !strcasecmp(mnem, "SW")) return 1;
    if (!strcasecmp(mnem, "BR") || !strcasecmp(mnem, "JMP")) return 1;
    if (!strcasecmp(mnem, "NOP") || !strcasecmp(mnem, "HALT")) return 1;
    if (!strcasecmp(mnem, "MOV") || !strcasecmp(mnem, "CLR") ||
        !strcasecmp(mnem, "INC") || !strcasecmp(mnem, "DEC")) return 1;
    if (!strcasecmp(mnem, "NEG")) return 2;
    if (!strcasecmp(mnem, "LI")) {
        if (n < 2) { fprintf(stderr, "error: LI needs 2 operands\n"); exit(1); }
        return 1 + li_chunk_count(parse_int(ops[1])); /* CLR + chunks */
    }

    fprintf(stderr, "error: unknown mnemonic '%s'\n", mnem);
    exit(1);
}

/* ---------- encode one source line (pass 2), appends words via emit() ---------- */

static void encode_line(const char *mnem, char *operand_str, int this_addr) {
    char ops_copy[MAX_LINE];
    char *ops[4];
    int n, rd, rs1, rs2, imm, target;

    strncpy(ops_copy, operand_str, MAX_LINE - 1);
    ops_copy[MAX_LINE - 1] = '\0';
    n = split_operands(ops_copy, ops, 4);

    /* ---- real R-type ALU ops ---- */
    if (!strcasecmp(mnem, "ADD") || !strcasecmp(mnem, "SUB") ||
        !strcasecmp(mnem, "AND") || !strcasecmp(mnem, "XOR") ||
        !strcasecmp(mnem, "OR")  || !strcasecmp(mnem, "SLL") ||
        !strcasecmp(mnem, "SRL") || !strcasecmp(mnem, "SRA")) {
        int op =  !strcasecmp(mnem, "ADD") ? 0 :
                  !strcasecmp(mnem, "SUB") ? 1 :
                  !strcasecmp(mnem, "AND") ? 2 :
                  !strcasecmp(mnem, "XOR") ? 3 :
                  !strcasecmp(mnem, "OR")  ? 4 :
                  !strcasecmp(mnem, "SLL") ? 5 :
                  !strcasecmp(mnem, "SRL") ? 6 : 7;
        if (n != 3 || !is_register(ops[0], &rd) || !is_register(ops[1], &rs1) ||
            !is_register(ops[2], &rs2)) {
            fprintf(stderr, "error: '%s' needs 3 registers\n", mnem); exit(1);
        }
        emit(enc_R(op, rd, rs1, rs2));
        return;
    }

    if (!strcasecmp(mnem, "ADDI")) {
        if (n != 3 || !is_register(ops[0], &rd) || !is_register(ops[1], &rs1)) {
            fprintf(stderr, "error: ADDI needs rd, rs1, imm\n"); exit(1);
        }
        imm = parse_int(ops[2]);
        check_imm(imm, -8, 7, "ADDI immediate");
        emit(enc_I(8, rd, rs1, imm));
        return;
    }

    if (!strcasecmp(mnem, "LW")) {
        if (n != 2 || !is_register(ops[0], &rd) || !is_register(ops[1], &rs1)) {
            fprintf(stderr, "error: LW needs rd, rs1\n"); exit(1);
        }
        emit(enc_LOAD(9, rd, rs1));
        return;
    }

    if (!strcasecmp(mnem, "SW")) {
        if (n != 2 || !is_register(ops[0], &rs1) || !is_register(ops[1], &rs2)) {
            fprintf(stderr, "error: SW needs rs1, rs2\n"); exit(1);
        }
        emit(enc_STORE(10, rs1, rs2));
        return;
    }

    if (!strcasecmp(mnem, "BR")) {
        int target_addr, offset;
        if (n != 1) { fprintf(stderr, "error: BR needs 1 operand\n"); exit(1); }
        if (isdigit((unsigned char)ops[0][0]) || ops[0][0] == '-') {
            offset = parse_int(ops[0]);
        } else {
            target_addr = find_label(ops[0]);
            offset = target_addr - this_addr;
        }
        check_imm(offset, -8, 7, "BR offset");
        emit(enc_BRANCH(11, offset));
        return;
    }

    if (!strcasecmp(mnem, "JMP")) {
        if (n != 1) { fprintf(stderr, "error: JMP needs 1 operand\n"); exit(1); }
        if (isdigit((unsigned char)ops[0][0])) {
            target = parse_int(ops[0]);
        } else {
            target = find_label(ops[0]);
        }
        check_imm(target, 0, 4095, "JMP target");
        emit(enc_JUMP(12, target));
        return;
    }

    if (!strcasecmp(mnem, "NOP")) { emit(0xF000); return; }
    if (!strcasecmp(mnem, "HALT")) { emit(enc_BRANCH(11, -1)); return; }

    if (!strcasecmp(mnem, "MOV")) {
        if (n != 2 || !is_register(ops[0], &rd) || !is_register(ops[1], &rs1)) {
            fprintf(stderr, "error: MOV needs rd, rs\n"); exit(1);
        }
        emit(enc_I(8, rd, rs1, 0));
        return;
    }

    if (!strcasecmp(mnem, "CLR")) {
        if (n != 1 || !is_register(ops[0], &rd)) {
            fprintf(stderr, "error: CLR needs rd\n"); exit(1);
        }
        emit(enc_R(1, rd, rd, rd)); /* SUB rd, rd, rd */
        return;
    }

    if (!strcasecmp(mnem, "INC")) {
        if (n != 1 || !is_register(ops[0], &rd)) {
            fprintf(stderr, "error: INC needs rd\n"); exit(1);
        }
        emit(enc_I(8, rd, rd, 1));
        return;
    }

    if (!strcasecmp(mnem, "DEC")) {
        if (n != 1 || !is_register(ops[0], &rd)) {
            fprintf(stderr, "error: DEC needs rd\n"); exit(1);
        }
        emit(enc_I(8, rd, rd, -1));
        return;
    }

    if (!strcasecmp(mnem, "NEG")) {
        if (n != 2 || !is_register(ops[0], &rd) || !is_register(ops[1], &rs1)) {
            fprintf(stderr, "error: NEG needs rd, rs\n"); exit(1);
        }
        emit(enc_R(1, rd, rd, rd));   /* CLR rd */
        emit(enc_R(1, rd, rd, rs1));  /* SUB rd, rd, rs1 */
        return;
    }

    if (!strcasecmp(mnem, "LI")) {
        int remaining, step, chunk;
        if (n != 2 || !is_register(ops[0], &rd)) {
            fprintf(stderr, "error: LI needs rd, imm\n"); exit(1);
        }
        imm = parse_int(ops[1]);
        emit(enc_R(1, rd, rd, rd)); /* CLR rd */
        remaining = imm;
        while (remaining != 0) {
            if (remaining > 0) { step = remaining > 7 ? 7 : remaining; }
            else               { step = remaining < -8 ? -8 : remaining; }
            chunk = step;
            emit(enc_I(8, rd, rd, chunk));
            remaining -= chunk;
        }
        return;
    }

    fprintf(stderr, "error: unknown mnemonic '%s'\n", mnem);
    exit(1);
}

/* ---------- line splitting: optional "label:" then optional "MNEM operands" ---------- */

static void split_label_mnem(char *line, char **label_out, char **mnem_out, char *operand_buf) {
    char *colon = strchr(line, ':');
    char *rest = line;

    *label_out = NULL;
    *mnem_out  = NULL;
    operand_buf[0] = '\0';

    if (colon) {
        *colon = '\0';
        *label_out = trim(line);
        rest = colon + 1;
    }

    rest = trim(rest);
    if (*rest == '\0') return;

    {
        char *sp = rest;
        while (*sp && !isspace((unsigned char)*sp)) sp++;
        if (*sp) {
            *sp = '\0';
            strncpy(operand_buf, trim(sp + 1), MAX_LINE - 1);
            operand_buf[MAX_LINE - 1] = '\0';
        }
        *mnem_out = rest;
    }
}

/* ---------- main two-pass driver ---------- */

int main(int argc, char **argv) {
    FILE *in, *out;
    char line[MAX_LINE];
    int addr;
    int i;

    if (argc != 3) {
        fprintf(stderr, "usage: %s input.asm output.hex\n", argv[0]);
        return 1;
    }

    /* ---- pass 1: collect labels, compute addresses ---- */
    in = fopen(argv[1], "r");
    if (!in) { perror("fopen"); return 1; }

    addr = 0;
    while (fgets(line, sizeof(line), in)) {
        char *label, *mnem, operand_buf[MAX_LINE];
        char working[MAX_LINE];
        strncpy(working, line, MAX_LINE - 1);
        working[MAX_LINE - 1] = '\0';
        strip_comment(working);
        if (trim(working)[0] == '\0') continue;

        split_label_mnem(working, &label, &mnem, operand_buf);
        if (label) add_label(label, addr);
        if (mnem && *mnem) addr += words_for_line(mnem, operand_buf);
    }
    fclose(in);

    /* ---- pass 2: encode ---- */
    in = fopen(argv[1], "r");
    if (!in) { perror("fopen"); return 1; }

    addr = 0;
    while (fgets(line, sizeof(line), in)) {
        char *label, *mnem, operand_buf[MAX_LINE];
        char working[MAX_LINE];
        strncpy(working, line, MAX_LINE - 1);
        working[MAX_LINE - 1] = '\0';
        strip_comment(working);
        if (trim(working)[0] == '\0') continue;

        split_label_mnem(working, &label, &mnem, operand_buf);
        if (mnem && *mnem) {
            int words_before = word_count;
            encode_line(mnem, operand_buf, addr);
            addr += (word_count - words_before);
        }
    }
    fclose(in);

    /* ---- write Logisim v2.0 raw hex image ---- */
    out = fopen(argv[2], "w");
    if (!out) { perror("fopen"); return 1; }
    fprintf(out, "v2.0 raw\n");
    for (i = 0; i < word_count; i++) {
        fprintf(out, "%04x%c", memory[i], ((i + 1) % 8 == 0) ? '\n' : ' ');
    }
    fprintf(out, "\n");
    fclose(out);

    fprintf(stderr, "assembled %d words -> %s\n", word_count, argv[2]);
    return 0;
}
