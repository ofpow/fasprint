#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "fasprint.h"

FILE *f;
fas_header h;
char *string_table;
fas_symbol *symbol_table;
char *preproc_src;
asm_dump *asm_dump_table;
section_name *section_name_table;
symbol_ref *symbol_ref_table;

bool verbose = false;
bool p_header = false;
bool p_symbol_table = false;
bool p_preproc_source = false;
bool p_string_table = false;

void read_file(char *file_path) {
    f = fopen(file_path, "rb");
    if (f == NULL) {
        fprintf(stderr, "couldnt open %s for reading\n", file_path);
        exit(1);
    }

    fread(&h, sizeof(fas_header), 1, f);
    if (h.signature != 0x1A736166) {fprintf(stderr, "tried to read non .fas file %s\n", file_path); exit(1);}

    string_table = calloc(sizeof(char), h.string_table_len);
    fread(string_table, sizeof(char), h.string_table_len, f);
   
    symbol_table = calloc(sizeof(fas_symbol), NUM_SYMBOLS);
    fread(symbol_table, sizeof(fas_symbol), NUM_SYMBOLS, f);

    preproc_src = calloc(sizeof(char), PREPROC_SRC_LEN);
    fread(preproc_src, sizeof(char), PREPROC_SRC_LEN, f);

    asm_dump_table = calloc(sizeof(asm_dump), NUM_ASM_DUMP);
    fread(asm_dump_table, sizeof(asm_dump), NUM_ASM_DUMP, f);

    section_name_table = calloc(sizeof(section_name), NUM_SECTION_NAMES);
    fread(section_name_table, sizeof(section_name), NUM_SECTION_NAMES, f);

    symbol_ref_table = calloc(sizeof(symbol_ref), NUM_SYMBOL_REFS);
    fread(symbol_ref_table, sizeof(symbol_ref), NUM_SYMBOL_REFS, f);
}

void print_header(void) {
    printf("Header:\n\n");
    printf("fasm major version: %d\n", h.maj_ver);
    printf("fasm minor version: %d\n", h.min_ver);
    printf("header length: %d\n", h.header_len);
    printf("input file: %s\n", string_table + h.input_file_offset);
    printf("output file: %s\n", string_table + h.output_file_offset);
    printf("string table offset: %d\n", h.string_table_offset);
    printf("string table len: %d\n", h.string_table_len);
    printf("symbol table offset: %d\n", h.symbol_table_offset);
    printf("symbol table len: %d\n", h.symbol_table_len);
    printf("preproc source offset: %d\n", h.preproc_src_offset);
    printf("preproc source len: %d\n", h.preproc_src_len);
    printf("asm dump offset: %d\n", h.asm_dump_offset);
    printf("asm dump len: %d\n", h.asm_dump_len);
    printf("section names offset: %d\n", h.section_names_offset);
    printf("section names len: %d\n", h.section_names_len);
    printf("symbol references offset: %d\n", h.symbol_ref_offset);
    printf("symbol references len: %d\n", h.symbol_ref_len);
}

void print_symbol_table(void) {
    printf("Symbol table:\n\n");
    for (int i = 0; i < NUM_SYMBOLS; i++) {
        fas_symbol sym = symbol_table[i];
        if (sym.symbol_name_offset & (1 << 31)) {
            printf("sym\n");
            printf("Symbol_name: %s\n", string_table + (sym.symbol_name_offset ^ (1 << 31)));
        } else {
            char *pstr = preproc_src + sym.symbol_name_offset;
            printf("Symbol name: %.*s\n", pstr[0], pstr + 1);
        }
        printf("Symbol value: %ld 0x%lx\n", sym.value, sym.value);

        if (verbose) {
            symbol_flags flags = sym.flags;
            printf("Symbol was defined: %d\n", flags.SYMBOL_DEFINED);
            printf("Symbol is an assembly-time variable: %d\n", flags.SYMBOL_ASM_TIME);
            printf("Symbol cannot be forward referenced: %d\n", flags.SYMBOL_CANT_FORWARD_REFERENCE);
            printf("Symbol was used: %d\n", flags.SYMBOL_USED);
            printf("Prediction needed when checking if symbol was used: %d\n", flags.SYMBOL_PRED_NEEDED_USED);
            printf("Result of last predicted check for being used: %d\n", flags.SYMBOL_LAST_CHECK_USED);
            printf("Prediction needed when checking if symbol was defined: %d\n", flags.SYMBOL_PRED_NEEDED_DEFINED);
            printf("Result of last predicted check for being defined: %d\n", flags.SYMBOL_LAST_PRED_DEFINED);
            printf("Symbol has optimization adjustment applied to its value: %d\n", flags.SYMBOL_OPT_ADJ); 
            printf("Symbol value is negative number in two's complement: %d\n", flags.SYMBOL_NEG_NUMBER);
            printf("Symbol has no value: %d\n", flags.SYMBOL_NO_VALUE);
            printf("Symbol type: %s\n", symbol_value_type_str(sym.value_type));
            printf("SIB: %d\n", sym.extended_sib);
            printf("Last defined in pass: %d\n", sym.pass_last_defined);
            printf("Last used in pass: %d\n", sym.pass_last_used);
            printf("reloc: %d\n", sym.relative_section_info);
            printf("Symbol preproc source offset: %d\n", sym.symbol_line_offset);
            printf("Symbol data size: %d\n", sym.data_size);
        }

        printf("\n");
    }
}

void print_preproc_source(void) {
    printf("Preprocessed source:\n\n");
    char *src = preproc_src;

    while (1) {
        if (src >= preproc_src + h.preproc_src_len) return;
        preproc_line *l = (preproc_line*)src;

        printf("%10d ", l->line_number);
        src += sizeof(preproc_line);
        u_int8_t *x = (u_int8_t*)src;
        while(1) {
            switch (*src) {
                case 26:
                    src++;
                    printf(" ");
                    printf("%.*s", *src, src + 1);
                    src += *src + 1;
                    x += *src + 1;
                    break;
                case 0:
                    printf("\n");
                    src++;
                    x++;
                    goto done;
                case 34:
                    printf(" \"");
                    src++;
                    u_int32_t len = *src;
                    src++;
                    len = (len << 8) | *src;
                    src++;
                    len = (len << 8) | *src;
                    src++;
                    len = (len << 8) | *src;
                    len = ((len>>24)&0xff) | 
                        ((len<<8)&0xff0000) | 
                        ((len>>8)&0xff00) | 
                        ((len<<24)&0xff000000);
                    printf("%.*s\"", len, ++src);
                    src += len;
                    break;
                case 7:
                    src++;
                default:
                    printf("%c", *src);
                    src++;
            }
        }
done:
    }
}

void print_string_table(void) {
    printf("String table:\n\n");
    char *s = string_table;
    while (s < string_table + h.string_table_len) {
        printf("%s\n", s);
        s += strlen(s) + 1;
    }
}

void free_mem(void) {
    free(symbol_ref_table);
    free(section_name_table);
    free(asm_dump_table);
    free(preproc_src);
    free(symbol_table);
    free(string_table);
    fclose(f);
}

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Usage: fasprint <args> [file]\n");
        printf("Arguments:\n");
        printf("  -v: enable verbose printing\n");
        printf("  -sym: print symbol table\n");
        printf("  -p: print preprocessed source\n");
        printf("  -str: print string table\n");
        printf("  -a: print everything\n");
        exit(1);
    }

    char *file_path;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) verbose = true;
        else if (!strcmp(argv[i], "-h")) p_header = true;
        else if (!strcmp(argv[i], "-sym")) p_symbol_table = true;
        else if (!strcmp(argv[i], "-p")) p_preproc_source = true;
        else if (!strcmp(argv[i], "-str")) p_string_table = true;
        else if (!strcmp(argv[i], "-a")) {p_header = true; p_symbol_table = true; p_preproc_source = true; p_string_table = true;}
        else file_path = argv[i];
    }
    read_file(file_path);

    if (p_header) {print_header(); printf("\n\n");}
    if (p_symbol_table) {print_symbol_table(); printf("\n");}
    if (p_preproc_source) {print_preproc_source(); printf("\n\n");}
    if (p_string_table) {print_string_table(); printf("\n");}
    
    free_mem();
    return 0;
}
