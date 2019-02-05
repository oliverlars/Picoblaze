#include <stdio.h>
#include <stdlib.h>

enum KeywordType: int{
    KEYWORD_JUMP = 0,
    KEYWORD_CALL,
    KEYWORD_RETURN,
    KEYWORD_ADD,
    KEYWORD_ADDC,
    KEYWORD_SUB,
    KEYWORD_SUBC,
    KEYWORD_COMPARE,
    KEYWORD_LOAD,
    KEYWORD_AND,
    KEYWORD_OR,
    KEYWORD_XOR,
    KEYWORD_SR0,
    KEYWORD_SR1,
    KEYWORD_SRX,
    KEYWORD_SRA,
    KEYWORD_RR,
    KEYWORD_SL0,
    KEYWORD_SL1,
    KEYWORD_SLX,
    KEYWORD_SLA,
    KEYWORD_RL,
    KEYWORD_STORE,
    KEYWORD_FETCH,
    KEYWORD_INPUT,
    KEYWORD_OUTPUT,
    
    KEYWORD_Z,
    KEYWORD_NZ,
    KEYWORD_C,
    KEYWORD_NC,
    
    KEYWORD_LABEL,
    
    KEYWORD_INVALID
    
};

enum TokenType: int{
    TOKEN_INVALID,
    
    TOKEN_FULLSTOP,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_CONDITION,
    TOKEN_INSTRUCTION,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER_LITERAL,
    
    TOKEN_END,
    
};


enum Opcode: unsigned int{
    OPCODE_JUMP_U = 0x22000,
    OPCODE_JUMP_Z = 0x32000,
    OPCODE_JUMP_NZ = 0x36000,
    OPCODE_JUMP_C = 0x3A000,
    OPCODE_JUMP_NC = 0x3E000,
    
    OPCODE_CALL_U = 0x20000,
    OPCODE_CALL_Z = 0x30000,
    OPCODE_CALL_NZ = 0x34000,
    OPCODE_CALL_C = 0x38000,
    OPCODE_CALL_NC = 0x3C000,
    
    OPCODE_ADD_REGISTER = 0x10000,
    OPCODE_ADD_CONSTANT = 0x11000,
    
    OPCODE_ADDC_REGISTER = 0x12000,
    OPCODE_ADDC_CONSTANT = 0x13000,
    
    OPCODE_SUB_REGISTER = 0x18000,
    OPCODE_SUB_CONSTANT = 0x19000,
    
    OPCODE_SUBC_REGISTER = 0x1A000,
    OPCODE_SUBC_CONSTANT = 0x1B000,
    
    OPCODE_COMPARE_REGISTER = 0x1C000,
    OPCODE_COMPARE_CONSTANT = 0x1D000,
    
    OPCODE_COMPAREC_REGISTER = 0x1E000,
    OPCODE_COMPAREC_CONSTANT = 0x1F000,
    
    OPCODE_LOAD_REGISTER = 0x00000,
    OPCODE_LOAD_CONSTANT= 0x01000,
    
    OPCODE_AND_REGISTER = 0x02000,
    OPCODE_AND_CONSTANT = 0x03000,
    
    OPCODE_OR_REGISTER = 0x04000,
    OPCODE_OR_CONSTANT = 0x05000,
    
    OPCODE_XOR_REGISTER = 0x06000,
    OPCODE_XOR_CONSTANT = 0x07000,
    
    OPCODE_SL0 = 0x14006,
    OPCODE_SL1 = 0x14007,
    OPCODE_SLX = 0x14004,
    OPCODE_SLA = 0x14000,
    OPCODE_RL = 0x14002,
    OPCODE_SR0 = 0x1400E,
    OPCODE_SR1 = 0x1400F,
    OPCODE_SRX = 0x1400A,
    OPCODE_SRA = 0x14008,
    OPCODE_RR = 0x1400C,
    
    OPCODE_STORE_REGISTER = 0x2E000,
    OPCODE_STORE_CONSTANT = 0x2F000,
    
    OPCODE_FETCH_REGISTER = 0x0A000,
    OPCODE_FETCH_CONSTANT = 0x0B000,
};

union Instruction{
    struct{
        unsigned int address: 12;
    };
    struct{
        unsigned int constant: 8;
    };
    struct{
        unsigned int _dummy: 4;
        unsigned int regy: 4;
        unsigned int regx: 4;
    };
    unsigned int opcode: 20;
    unsigned int val: 20;
    
};

struct String{
    char* text;
    int len;
};


struct Token{
    String str;
    TokenType type;
};

struct Hash_Node{
    Hash_Node* next = nullptr;
    int value;
};
#define HASH_SIZE 211

struct Map{
    Hash_Node* nodes[HASH_SIZE];
    int size;
};

struct Lexer{
    char* pos;
    bool error;
    int instruction_count = 0;
    Map label_map;
};

static char* instructions[]{
    "jump","call", "return", "add", "addc", "sub", "subc", "compare",
    "load", "and", "or", "xor", "sr0", "sr1", "srx", "sra", "rr",
    "sl0", "sl1", "slx", "sla", "rl", "store", "fetch", "input", "output",
};

static char* conditions[]{
    "nz", "n", "nc", "c"
};


#define array_count(x) sizeof(x)/sizeof(x[0])