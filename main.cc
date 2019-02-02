#include <stdio.h>
#include <stdlib.h>

#define PANIC(a, b) printf("PANIC: %s on line %d", a, b); exit(0)

static char*
open_source(char* filename){
    char* result = 0;
    FILE* file_pointer = fopen(filename, "r");
    
    if(file_pointer){
        fseek(file_pointer, 0, SEEK_END);
        size_t size = ftell(file_pointer);
        fseek(file_pointer, 0, SEEK_SET);
        
        result = (char*)malloc(size+1);
        fread(result, size, 1, file_pointer);
        result[size] = 0;
        fclose(file_pointer);
    }else {
        PANIC("Unable to open file", 0);
    }
    return result;
}

enum TokenType{
    LABEL=-2,
    INVALID=-1,
    JUMP=0,
    CALL,
    RETURN,
    ADD,
    ADDC,
    SUB,
    SUBC,
    COMPARE,
    LOAD,
    AND,
    OR,
    XOR,
    TEST,
    SR0,
    SR1,
    SRX,
    SRA,
    RR,
    SL0,
    SL1,
    SLX,
    SLA,
    RL,
    STORE,
    FETCH,
    INPUT,
    OUTPUT,
    
    
};

enum ConditionType{
    Z = OUTPUT+1,
    NZ,
    C,
    NC,
    
    NONE=-1
};


char* tokens[] =
{
    "jump", "call", "return", "add", "addc", "sub", "subc", "compare",
    "load", "and", "or", "xor", "test", "sr0", "sr1", "srx", "sra", "rr",
    "sl0", "sl1", "slx", "sla", "rl", "store", "fetch", "input", "output",
    
    "z", "nz", "c", "nc"
};

enum  AddressMode{
    ABSOLUTE,
    INDIRECT,
    RELATIVE,
    
};

static int
extract_opcode(TokenType type, AddressMode mode){
    switch(type){
        case LOAD:{
            if(mode == ABSOLUTE) return 0x01000;
            else if(mode == INDIRECT) return 0x00000;
        }break;
    }
}

static int
extract_opcode(TokenType type, ConditionType condition){
    switch(type){
        case JUMP:{
            switch(condition){
                case NZ:{
                    return 0x36000;
                }break;
                case Z:{
                    return 0x32000;
                }break;
                case NC:{
                    return 0x3E000;
                }break;
                case C:{
                    return 0x3A000;
                }break;
                case NONE:{
                    return 0x22000;
                }break;
            }break;
        }
    }
}

static void
add_regx_to_opcode(int* opcode, int regx){
    int x = 0x00100;
    int val_x = x*regx;
    
    *opcode |= val_x;
}


static void
add_regy_to_opcode(int* opcode, int regy){
    int y = 0x00010;
    int val_y = y*regy;
    
    *opcode |= val_y;
}

struct Token{
    TokenType type = INVALID;
    char* data;
    int size;
};

struct Stack{
    char reg[16];
    int top = -1;
};

static void
push_stack(Stack* stack, char val){
    if(stack->top >= 16) PANIC("Stack overflow", 0); 
    stack->reg[stack->top++] = val;
}

static char
pop_stack(Stack* stack){
    if(stack->top < 0) PANIC("Stack underflow", 0);
    return stack->reg[stack->top--];
}

static inline bool 
token_match(Token token, TokenType type){
    char* pos = tokens[type];
    for(int i =  0; i < token.size; i++, pos++){
        if((*pos == 0)||(token.data[i] != *pos)){
            return false;
        }
    }
    bool result = (*pos == 0);
    return result;
}

static inline bool 
is_whitespace(char c){
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

static inline bool
is_alpha(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline bool
is_number(char c){
    return (c <= '9') && (c >= '0');
}

static int 
token_to_number(Token token){
    
    auto pow = [](int base, int exponent){
        int result = 1;
        for(int i = 1; i < exponent; i++){
            result *= base;
        }
        return result;
    };
    
    int result = 0;
    for(int i = 0; i < token.size; i++){
        result += (token.data[i] - '0') * pow(10, (token.size - i));
    }
    return result;
}

struct Lexer{
    char* pos;
};

static Token
parse_keyword(Lexer* l){
    Token keyword = {};
    
    while(is_whitespace(*l->pos)) l->pos++;
    char* start_pos = l->pos;
    while(is_alpha(*l->pos)) l->pos++;
    keyword.size = l->pos - start_pos;
    keyword.data = start_pos;
    keyword.type = INVALID;
    for(int type =JUMP; type <= NC; type++){
        if(token_match(keyword, (TokenType)type)){
            keyword.type = (TokenType)type;
            break;
        }
    }
    if(keyword.data[keyword.size] == ':'){
        keyword.type = LABEL;
    }
    return keyword;
}

static char
peek(Lexer l){
    while(is_whitespace(*l.pos)) l.pos++;
    return *l.pos;
}

static char
peek_and_advance(Lexer* l){
    while(is_whitespace(*l->pos)) l->pos++;
    return *l->pos;
}

static int 
parse_number(Lexer* l){
    Token number = {};
    
    while(is_whitespace(*l->pos)) l->pos++;
    char* start_pos = l->pos;
    while(is_number(*l->pos)) l->pos++;
    number.size = l->pos - start_pos;
    number.data = start_pos;
    
    return token_to_number(number);
}

static void
num_to_8bit_bin(int num, char* result){
    int no_bits = 7;
    for(int i = no_bits; i >= 0; i--){
        int k = num >> i;
        if(k & 1) result[no_bits-i] = '1';
        else result[no_bits-i] = '0';
    }
}
static void
eat_whitespace(Lexer* l){
    while(is_whitespace(*l->pos)){
        l->pos++;
    }
}
static void 
emit_binary(int num, TokenType type){
    char operand[] = "00000000";
    num_to_8bit_bin(num, operand);
    
    printf("%.*s\")\n", 8,operand); 
}

#define MEM_SIZE   256
#define STACK_SIZE 16
#define RAM_SIZE   64

void render(char* mem, char* stack){
    printf("STACK----------\n");
    for(int i = 0; i < STACK_SIZE;){
        for(int j = 0; j < 16; j++){
            printf("%d ", stack[i++]);
        }
        printf("\n");
    }
    printf("RAM------------\n");
    for(int i = 0; i < MEM_SIZE;){
        for(int j = 0; j < 16; j++){
            printf("%d ", mem[i++]);
        }
        printf("\n");
    }
}

static inline int
extract_regx(int instruction){
    return (instruction & 0x00F00) >> 8;
}

static inline int
extract_regy(int instruction){
    return (instruction & 0x000F0) >> 4;
}

static inline int
extract_constant(int instruction){
    return (instruction & 0x000FF);
}

static inline int
extract_address(int instruction){
    return (instruction & 0x00FFF);
}


int main(int argc, char** args){
    char* source = open_source(args[1]);
    Lexer lex = {source};
    char* pos = source;
    bool parsing = true;
    int line_no = 1;
    
    char mem[MEM_SIZE] = {};
    Stack stack = {};
    int ram[RAM_SIZE] = {};
    int pc = 0;
    while(parsing){
        if(*lex.pos){
            if(is_alpha(*lex.pos)){
                Token keyword = parse_keyword(&lex);
                if(keyword.type == LABEL){
                    printf("%.*s", keyword.size, keyword.data);
                }
            }
        }else{
            parsing = false;
        }
        lex.pos++;
    }
    parsing = true;
    while(parsing){
        if(*lex.pos){
            if(is_alpha(*lex.pos))
            {
                Token current = parse_keyword(&lex);
                switch(current.type)
                {
                    case LOAD:
                    {
                        if(peek_and_advance(&lex) == 's')
                        {
                            lex.pos++;
                            int regx = parse_number(&lex);
                            if(peek_and_advance(&lex) == ',')
                            {
                                lex.pos++;
                                int opcode = 0x00000;
                                add_regx_to_opcode(&opcode, regx);
                                if(peek_and_advance(&lex) == 's')
                                {
                                    lex.pos++;
                                    int regy = parse_number(&lex);
                                    opcode |= extract_opcode(LOAD, INDIRECT);
                                    add_regy_to_opcode(&opcode, regy);
                                }
                                else
                                {
                                    int constant = parse_number(&lex);
                                    opcode |= extract_opcode(LOAD, ABSOLUTE);
                                    opcode += constant;
                                }
                                ram[pc] = opcode;
                            }
                        }
                        pc++;
                    }break;
                    case JUMP:{
                        Token condition;
                        int address;
                        if(is_alpha(peek(lex))){
                            condition = parse_keyword(&lex);
                        }
                        else if(is_number(peek(lex))){
                            address = parse_number(&lex);
                        }
                        int opcode = 0x00000;
                        switch(condition.type){
                            case NZ:{
                                opcode |= extract_opcode(JUMP, NZ);
                            }break;
                            case Z:{
                            }break;
                            case NC:{
                            }break;
                            case C:{
                            }break;
                            case INVALID:{ //assume it's conditionless
                                opcode |= extract_opcode(JUMP, NONE);
                            }break;
                        }
                        if(peek(lex) == ','){
                            lex.pos++;
                            eat_whitespace(&lex);
                            if(is_number(peek(lex))){
                                address = parse_number(&lex);
                            }
                            else if(is_alpha(peek(lex))){
                            }
                        }
                        opcode += address;
                        ram[pc] = opcode;
                        pc++;
                    }break;
                }
            }
        }else{
            parsing = false;
        }
        lex.pos++;
    }
    pc = 0; 
    
    while(pc < 3){
        int opcode = ram[pc] & 0xFF000;
        printf("%05x\n", opcode);
        switch(opcode){
            case 0x00000:{
                int regx = extract_regx(ram[pc]);
                int regy = extract_regy(ram[pc]);
                stack.reg[regx] = stack.reg[regy];
            }break;
            case 0x01000:{
                int regx = extract_regx(ram[pc]);  
                int constant = extract_constant(ram[pc]);
                stack.reg[regx] = constant;
            }break;
            case 0x22000:{
                int address = extract_address(ram[pc]);
                pc = address-1;
            }break;
            case 0x32000:{
            }break;
            case 0x36000:{
            }break;
            case 0x3A000:{
            }break;
            case 0x3E000:{
            }break;
        }
        pc++;
    }
    
    fflush(stdout);
    render(mem, stack.reg);
    return 0;
}