#pragma once

#define NUM_SYMBOLS (h.symbol_table_len / sizeof(fas_symbol))
#define PREPROC_SRC_LEN (h.preproc_src_len)
#define NUM_ASM_DUMP (h.asm_dump_len / sizeof(asm_dump))
#define NUM_SECTION_NAMES (h.section_names_len / sizeof(section_name))
#define NUM_SYMBOL_REFS (h.symbol_ref_len / sizeof(symbol_ref))

typedef struct fas_header {
    u_int32_t signature;
    u_int8_t maj_ver;
    u_int8_t min_ver;
    u_int16_t header_len;
    u_int32_t input_file_offset;
    u_int32_t output_file_offset;
    u_int32_t string_table_offset;
    u_int32_t string_table_len;
    u_int32_t symbol_table_offset;
    u_int32_t symbol_table_len; 
    u_int32_t preproc_src_offset;
    u_int32_t preproc_src_len;
    u_int32_t asm_dump_offset;
    u_int32_t asm_dump_len;
    u_int32_t section_names_offset;
    u_int32_t section_names_len;
    u_int32_t symbol_ref_offset;
    u_int32_t symbol_ref_len;
} fas_header;

typedef struct symbol_flags {
    bool SYMBOL_DEFINED: 1;
    bool SYMBOL_ASM_TIME: 1;
    bool SYMBOL_CANT_FORWARD_REFERENCE: 1;
    bool SYMBOL_USED: 1;
    bool SYMBOL_PRED_NEEDED_USED: 1;
    bool SYMBOL_LAST_CHECK_USED: 1;
    bool SYMBOL_PRED_NEEDED_DEFINED: 1;
    bool SYMBOL_LAST_PRED_DEFINED: 1;
    bool SYMBOL_OPT_ADJ: 1;
    bool SYMBOL_NEG_NUMBER: 1;
    bool SYMBOL_NO_VALUE: 1;
} symbol_flags;

typedef struct fas_symbol {
    u_int64_t value;
    symbol_flags flags;
    u_int8_t data_size;
    u_int8_t value_type;
    u_int32_t extended_sib;
    u_int16_t pass_last_defined;
    u_int16_t pass_last_used;
    u_int32_t relative_section_info;
    u_int32_t symbol_name_offset;
    u_int32_t symbol_line_offset;
} fas_symbol;

typedef enum {
    ABS_VALUE,
    RELOC_SEGMENT,
    RELOC_32,
    RELOC_32_RELATIVE,
    RELOC_64,
    GOT_32_RELATIVE,
    PLT_ENTRY_32,
    PLT_ENTRY_32_RELATIVE
} symbol_value_type;

char *symbol_value_type_str(symbol_value_type type) {
    switch (type) {
    case ABS_VALUE: return "ABS_VALUE";
    case RELOC_SEGMENT: return "RELOC_SEGMENT";
    case RELOC_32: return "RELOC_32";
    case RELOC_32_RELATIVE: return "RELOC_32_RELATIVE";
    case RELOC_64: return "RELOC_64";
    case GOT_32_RELATIVE: return "GOT_32_RELATIVE";
    case PLT_ENTRY_32: return "PLT_ENTRY_32";
    case PLT_ENTRY_32_RELATIVE: return "PLT_ENTRY_32_RELATIVE";
    }
    return "unreachable";
}

typedef enum {
    BX = 0x23,
    BP = 0x25,
    SI = 0x26,
    DI = 0x27,
    EAX = 0x40,
    ECX = 0x41,
    EDX = 0x42,
    EBX = 0x43,
    ESP = 0x44,
    EBP = 0x45,
    ESI = 0x46,
    EDI = 0x47,
    R8D = 0x48,
    R9D = 0x49,
    R10D = 0x4a,
    R11D = 0x4b,
    R12D = 0x4c,
    R13D = 0x4d,
    R14D = 0x4e,
    R15D = 0x4f,
    RAX = 0x80,
    RCX = 0x81,
    RDX = 0x82,
    RBX = 0x83,
    RSP = 0x84,
    RBP = 0x85,
    RSI = 0x86,
    RDI = 0x87,
    R8 = 0x88,
    R9 = 0x89,
    R10 = 0x8a,
    R11 = 0x8b,
    R12 = 0x8c,
    R13 = 0x8d,
    R14 = 0x8e,
    R15 = 0x8f,
    EIP = 0x94,
    RIP = 0x98,
} reg_code;

typedef struct preproc_line {
    u_int32_t line_info_offset;
    u_int32_t line_number;
    u_int32_t line_offset;
    u_int32_t macro_line_offset;
} preproc_line;

typedef struct asm_dump {
    u_int32_t offset_in_file;
    u_int32_t offset_in_preproc_src;
    u_int64_t $_addr_value;
    u_int32_t $_extended_sib;
    u_int32_t $_reloc_info;
    u_int8_t $_addr_type;
    u_int8_t code_type;
    u_int8_t x;
    u_int8_t higher_bits_value_$_addr;
} asm_dump;

typedef struct section_name {
    u_int32_t offset;
} section_name;

typedef struct symbol_ref {
    u_int32_t symbol_offset;
    u_int32_t asm_dump_offset;
} symbol_ref;
