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
    LOAD=0,
    ADD_K,
    ADD_P,
    INPUT,
    OUTPUT,
    JUMP_U,
    JUMP_Z,
    JUMP_C,
    JUMP_NZ,
    JUMP_NC,
    
    //Not used as an index for instructions
    ADD,
    JUMP,
    INVALID,
};

char* instructions[] =
{
    "load", "k", "p", "input", "output", "u", "z", "c", "nz", "nc",
    "add", "jump" 
};

char* instruction_opcodes[] =
{
    "00000000",
    "01000000",
    "11000000",
    "10100000",
    "11100000",
    "10000000",
    "10010000",
    "10011000",
    "10010100",
    "10011100"
};

struct Token{
    TokenType type;
    char* data;
    int size;
};

static inline bool 
token_match(Token token, TokenType type){
    char* pos = instructions[type];
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
    for(int type =LOAD; type <= JUMP; type++){
        if(token_match(keyword, (TokenType)type)){
            keyword.type = (TokenType)type;
            break;
        }
    }
    if(keyword.type == INVALID) {
        PANIC("Unknown keyword", 1);
    }
    return keyword;
}

static char
peek_error(Lexer l){
    while(is_whitespace(*l.pos)) l.pos++;
    return *l.pos;
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
emit_binary(int num, TokenType type){
    char operand[] = "00000000";
    num_to_8bit_bin(num, operand);
    
    printf("DATA_RAM_WORD'(\"%.*s", 8,instruction_opcodes[type]);
    printf("%.*s\")\n", 8,operand); 
}

int main(int argc, char** args){
    char* source = open_source(args[1]);
    Lexer lex = {source};
    char* pos = source;
    bool parsing = true;
    int line_no = 1;
    while(parsing){
        if(*lex.pos){
            if(is_alpha(*lex.pos)){
                Token current = parse_keyword(&lex);
                switch(current.type){
                    case LOAD:{
                        if(!is_number(peek_error(lex))){
                            PANIC("Expected number, got letter",line_no);
                        }
                        int constant = parse_number(&lex);
                        emit_binary(constant, LOAD);
                    }break;
                    case INPUT:{
                        if(!is_number(peek_error(lex))){
                            PANIC("Expected number, got letter", line_no);
                        }
                        int address = parse_number(&lex);
                        emit_binary(address, INPUT);
                    }break;
                    case OUTPUT:{
                        if(!is_number(peek_error(lex))){
                            PANIC("Expected number, got letter", line_no);
                        }
                        int address = parse_number(&lex);
                        emit_binary(address, OUTPUT);
                    }break;
                    case ADD:{
                        Token mode = parse_keyword(&lex);
                        switch(mode.type){
                            case ADD_K:{
                                if(!is_number(peek_error(lex))){
                                    PANIC("Expected number, got letter", line_no);
                                }
                                int constant = parse_number(&lex);
                                emit_binary(constant, ADD_K);
                            }break;
                            case ADD_P:{
                                if(!is_number(peek_error(lex))){
                                    PANIC("Expected number, got letter", line_no);
                                }
                                int address = parse_number(&lex);
                                emit_binary(address, ADD_P);
                            }break;
                        }
                    }break;
                    case JUMP:{
                        Token condition = parse_keyword(&lex);
                        switch(condition.type){
                            case JUMP_Z:{
                                if(!is_number(peek_error(lex))){
                                    PANIC("Expected number, got letter", line_no);
                                }
                                int address = parse_number(&lex);
                                emit_binary(address, JUMP_Z);
                            }break;
                            case JUMP_C:{
                                if(!is_number(peek_error(lex))){
                                    PANIC("Expected number, got letter", line_no);
                                }
                                int address = parse_number(&lex);
                                emit_binary(address, JUMP_C);
                            }break;
                            case JUMP_NZ:{
                                if(!is_number(peek_error(lex))){
                                    PANIC("Expected number, got letter", line_no);
                                }
                                int address = parse_number(&lex);
                                emit_binary(address, JUMP_NZ);
                            }break;
                            case JUMP_NC:{
                                if(!is_number(peek_error(lex))){
                                    PANIC("Expected number, got letter", line_no);
                                }
                                int address = parse_number(&lex);
                                emit_binary(address, JUMP_NC);
                            }break;
                            case JUMP_U:{
                                if(!is_number(peek_error(lex))){
                                    PANIC("Expected number, got letter", line_no);
                                }
                                int address = parse_number(&lex);
                                emit_binary(address, JUMP_U);
                            }break;
                        }break;
                    }
                }
            }
            lex.pos++;
        }else{
            parsing = false;
        }
    }
    
    return 0;
}