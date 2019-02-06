#include "picoblaze_compiler.h"
#include "picoblaze_hash.h"

#define panic(a) printf("PANIC: %s", a); exit(0)


static char*
open_source(char* filename){
    char* result = 0;
    FILE* file_pointer = fopen(filename, "r");
    
    if(file_pointer){
        fseek(file_pointer, 0, SEEK_END);
        size_t size = ftell(file_pointer);
        fseek(file_pointer, 0, SEEK_SET);
        
        result = (char*)calloc(size+1, sizeof(char));
        result[size] = 0;
        fread(result, size, 1, file_pointer);
        fclose(file_pointer);
    }else {
        panic("unable to open file");
    }
    return result;
}


#define is_newline(x) ((x) == '\n' || (x) == '\r')
#define is_space(x) ((x) == ' ' || (x) == '\t' || (x) == '\v' || (x) == '\f' || is_newline(x))
#define is_digit(x) ((x) <= '9' && (x) >= '0')
#define is_upper_hex(x) (is_digit(x) || ((x) <= 'F' && (x) >= 'A'))
#define is_lower_hex(x) (is_digit(x) || ((x) <= 'f' && (x) >= 'a'))
#define is_hex(x) (is_upper(x)  || is_lower_hex(x))
#define is_lower_alpha(x) ((x) <= 'z' && (x) >= 'a')
#define is_upper_alpha(x) ((x) <= 'Z' && (x) >= 'A')
#define is_alpha(x) (is_lower_alpha(x) || is_upper_alpha(x))
#define print_string(x) printf("%.*s\n", x.len, x.text)

static void
eat_whitespace(Lexer* l){
    while(is_space(*l->pos)){
        l->pos++;
    }
}

static bool
match_token(Token token, char* string){
    char* pos = string;
    
    for(int i = 0; i < token.str.len; i++, pos++){
        if((*pos == 0) || (token.str.text[i] != *pos)){
            return false;
        }
        
    }
    return *pos == 0;
}

static bool
is_instruction(Token token){
    for(int i = 0; i < array_count(instructions); i++){
        if((match_token(token, instructions[i]))){
            return true;
        }
    }
    return false;
}

static bool
is_condition(Token token){
    for(int i = 0; i < array_count(conditions); i++){
        if(!(match_token(token, conditions[i]))){
            return false;
        }
    }
    return true;
}


static Token
get_token(Lexer* l){
    eat_whitespace(l);
    
    char c = l->pos[0];
    Token token = {};
    token.str.text = l->pos;
    token.str.len = 1;
    ++l->pos;
    switch(c){
        case '\0':{token.type = TOKEN_END;}break;
        case '.':{token.type = TOKEN_FULLSTOP;}break;
        case ':':{token.type = TOKEN_COLON; } break;
        case ',':{token.type = TOKEN_COMMA;} break;
        case '(':{token.type = TOKEN_OPEN_BRACE;}break;
        case ')':{token.type = TOKEN_CLOSE_BRACE;}break;
        default:{
            token.type = TOKEN_IDENTIFIER;
            if(is_lower_alpha(c)){
                while(is_lower_alpha(*l->pos)){++l->pos;}
                token.str.len = l->pos - token.str.text;
            }else if(is_digit(c)){
                token.type = TOKEN_NUMBER_LITERAL;
                while(*l->pos == 'x' || is_upper_hex(*l->pos)){l->pos++;}
                token.str.len = l->pos - token.str.text;
            }
            else if(is_upper_hex(c)){
                token.type = TOKEN_NUMBER_LITERAL;
                while(is_upper_hex(*l->pos)){l->pos++;}
                token.str.len = l->pos - token.str.text;
            }
        }break;
    }
    print_string(token.str);
    return token;
}

static Token
peek_token(Lexer* l){
    Lexer lex = *l;
    return get_token(&lex);
}

static Token
require_token(Lexer* l, TokenType type){
    
    Token token = get_token(l);
    if(token.type != type){
        panic("Unexpected token");
    }
    return token;
}

static Token
require_keyword(Lexer* l, char* keyword){
    Token token = peek_token(l);
    require_token(l, TOKEN_IDENTIFIER);
    if(!match_token(token, keyword)){
        panic("Unexpected keyword");
    }
    return token;
}

static bool
expect_token(Lexer* l, TokenType type){
    Token token = peek_token(l);
    return token.type == type;
}

static bool
peek_keyword(Lexer* l, char* keyword){
    Token token = peek_token(l);
    if(!expect_token(l, TOKEN_IDENTIFIER)){
        return false;
    }
    return match_token(token, keyword);
}

static inline void
skip_token(Lexer* l){
    get_token(l);
}

static inline int
string_to_int(String str){
    
    auto hex = [=](char c){
        int byte = c;
        if(is_digit(byte)){
            byte = byte - '0';
        }else if(is_lower_alpha(byte)  && byte <= 'f'){
            byte = byte - 'a' + 10;
        }else if(is_upper_alpha(byte) && byte <= 'F'){
            byte = byte - 'A' + 10;
        }
        return (byte & 0xF);
    };
    
    int value = 0;
    
    if(str.text[0] == '0' && str.text[1] == 'x'){
        for(int i = 2; i < str.len; i++){
            int byte = hex(str.text[i]);
            value = (value << 4) | byte;
        }
    }else{
        for(int i = 0; i < str.len; i++){
            value = value*10 + str.text[i] - '0';
        }
    }
    return value;
}

static void
write_instruction(int opcode, int regx = 0, int regy = 0, 
                  int constant = 0, int address = 0){
    int instruction = 0x00000;
    instruction |= opcode;
    instruction |= regx << 8;
    instruction |= regy << 4;
    instruction |= constant;
    instruction |= address;
    printf("instruction: %05x\n", instruction);
}

static int 
parse_register(Lexer* l){
    require_keyword(l, "s");
    Token reg = require_token(l, TOKEN_NUMBER_LITERAL);
    return string_to_int(reg.str);
}
void
parse_identifier(Lexer* l, Token token){
    if(match_token(token, "jump")){
        Token condition = get_token(l);
        int address = 0;
        int opcode = 0x00000;
        switch(condition.type){
            case TOKEN_IDENTIFIER:{
                if(match_token(condition, "nz")){
                    require_token(l, TOKEN_COMMA);
                    Token next = get_token(l);
                    if(next.type == TOKEN_IDENTIFIER){
                        address = map_get(l->label_map, next.str); 
                    }else if(next.type == TOKEN_NUMBER_LITERAL){
                        address = string_to_int(next.str); 
                    }
                    opcode = OPCODE_JUMP_NZ;
                }else if(match_token(condition, "z")){
                    opcode = OPCODE_JUMP_Z;
                }else if(match_token(condition, "nc")){
                    opcode = OPCODE_JUMP_NC;
                }else if(match_token(condition, "c")){
                    opcode=  OPCODE_JUMP_C;
                }else{
                    opcode = OPCODE_JUMP_U;
                    address = map_get(l->label_map, condition.str);
                }
            }break;
            case TOKEN_NUMBER_LITERAL:{
                opcode = OPCODE_JUMP_U;
                address = string_to_int(condition.str);
            }break;
        }
        write_instruction(opcode, address = address);
    }else if(match_token(token, "call")){
        Token condition = get_token(l);
        int address = 0;
        int opcode = 0x00000;
        switch(condition.type){
            case TOKEN_IDENTIFIER:{
                if(match_token(condition, "nz")){
                    require_token(l, TOKEN_COMMA);
                    Token next = get_token(l);
                    if(next.type == TOKEN_IDENTIFIER){
                        address = map_get(l->label_map, next.str); 
                    }else if(next.type == TOKEN_NUMBER_LITERAL){
                        address = string_to_int(next.str); 
                    }
                    opcode = OPCODE_CALL_NZ;
                }else if(match_token(condition, "z")){
                    opcode = OPCODE_CALL_Z;
                }else if(match_token(condition, "nc")){
                    opcode = OPCODE_CALL_NC;
                }else if(match_token(condition, "c")){
                    opcode=  OPCODE_CALL_C;
                }else{
                    opcode = OPCODE_CALL_U;
                    address = map_get(l->label_map, condition.str);
                }
            }break;
            case TOKEN_NUMBER_LITERAL:{
                opcode = OPCODE_CALL_U;
                address = string_to_int(condition.str);
            }break;
        }
        write_instruction(opcode, address = address);
    }else if(match_token(token, "return")){
        Token condition = get_token(l);
        int opcode = 0x00000;
        switch(condition.type){
            case TOKEN_IDENTIFIER:{
                if(match_token(condition, "nz")){
                    opcode = OPCODE_RETURN_NZ;
                }else if(match_token(condition, "z")){
                    opcode = OPCODE_RETURN_Z;
                }else if(match_token(condition, "nc")){
                    opcode = OPCODE_RETURN_NC;
                }else if(match_token(condition, "c")){
                    opcode=  OPCODE_RETURN_C;
                }else{
                    opcode = OPCODE_RETURN_U;
                }
            }break;
            case TOKEN_NUMBER_LITERAL:{
                opcode = OPCODE_RETURN_U;
            }break;
        }
        write_instruction(opcode);
    }else if(match_token(token, "add")){
        int opcode = 0x00000;
        int regx = parse_register(l);
        require_token(l, TOKEN_COMMA);
        if(peek_keyword(l, "s")){
            int regy = parse_register(l);
            opcode = OPCODE_ADD_REGISTER;
            write_instruction(opcode, regx, regy);
        }else{
            Token number = require_token(l, TOKEN_NUMBER_LITERAL);
            int value = string_to_int(number.str);
            opcode = OPCODE_ADD_CONSTANT;
            write_instruction(opcode, regx,0,  value);
        }
    }else if(match_token(token, "addc")){
        int opcode = 0x00000;
        int regx = parse_register(l);
        require_token(l, TOKEN_COMMA);
        if(peek_keyword(l, "s")){
            int regy = parse_register(l);
            opcode = OPCODE_ADDC_REGISTER;
            write_instruction(opcode, regx, regy);
        }else{
            Token number = require_token(l, TOKEN_NUMBER_LITERAL);
            int value = string_to_int(number.str);
            opcode = OPCODE_ADDC_CONSTANT;
            write_instruction(opcode, regx,0,  value);
        }
    }else if(match_token(token, "sub")){
        int opcode = 0x00000;
        int regx = parse_register(l);
        require_token(l, TOKEN_COMMA);
        if(peek_keyword(l, "s")){
            int regy = parse_register(l);
            opcode = OPCODE_SUB_REGISTER;
            write_instruction(opcode, regx, regy);
        }else{
            Token number = require_token(l, TOKEN_NUMBER_LITERAL);
            int value = string_to_int(number.str);
            opcode = OPCODE_SUB_CONSTANT;
            write_instruction(opcode, regx,0,  value);
        }
    }else if(match_token(token, "subc")){
        int opcode = 0x00000;
        int regx = parse_register(l);
        require_token(l, TOKEN_COMMA);
        if(peek_keyword(l, "s")){
            int regy = parse_register(l);
            opcode = OPCODE_SUBC_REGISTER;
            write_instruction(opcode, regx, regy);
        }else{
            Token number = require_token(l, TOKEN_NUMBER_LITERAL);
            int value = string_to_int(number.str);
            opcode = OPCODE_SUBC_CONSTANT;
            write_instruction(opcode, regx,0,  value);
        }
    }else if(match_token(token, "compare")){
        int opcode = 0x00000;
        int regx = parse_register(l);
        require_token(l, TOKEN_COMMA);
        if(peek_keyword(l, "s")){
            int regy = parse_register(l);
            opcode = OPCODE_COMPARE_REGISTER;
            write_instruction(opcode, regx, regy);
        }else{
            Token number = require_token(l, TOKEN_NUMBER_LITERAL);
            int value = string_to_int(number.str);
            opcode = OPCODE_COMPARE_CONSTANT;
            write_instruction(opcode, regx,0,  value);
        }
    }else if(match_token(token, "load")){
        int opcode = 0x00000;
        int regx = parse_register(l);
        require_token(l, TOKEN_COMMA);
        if(peek_keyword(l, "s")){
            int regy = parse_register(l);
            opcode = OPCODE_LOAD_REGISTER;
            write_instruction(opcode, regx, regy);
        }else{
            Token number = require_token(l, TOKEN_NUMBER_LITERAL);
            int value = string_to_int(number.str);
            opcode = OPCODE_LOAD_CONSTANT;
            write_instruction(opcode, regx, 0,  value);
        }
    }else if(match_token(token, "and")){
        int opcode = 0x00000;
        int regx = parse_register(l);
        require_token(l, TOKEN_COMMA);
        if(peek_keyword(l, "s")){
            int regy = parse_register(l);
            opcode = OPCODE_AND_REGISTER;
            write_instruction(opcode, regx, regy);
        }else{
            Token number = require_token(l, TOKEN_NUMBER_LITERAL);
            int value = string_to_int(number.str);
            opcode = OPCODE_AND_CONSTANT;
            write_instruction(opcode, regx,0,  value);
        }
    }else if(match_token(token, "or")){
        int opcode = 0x00000;
        int regx = parse_register(l);
        require_token(l, TOKEN_COMMA);
        if(peek_keyword(l, "s")){
            int regy = parse_register(l);
            opcode = OPCODE_OR_REGISTER;
            write_instruction(opcode, regx, regy);
        }else{
            Token number = require_token(l, TOKEN_NUMBER_LITERAL);
            int value = string_to_int(number.str);
            opcode = OPCODE_OR_CONSTANT;
            write_instruction(opcode, regx,0,  value);
        }
    }else if(match_token(token, "xor")){
        int opcode = 0x00000;
        int regx = parse_register(l);
        require_token(l, TOKEN_COMMA);
        if(peek_keyword(l, "s")){
            int regy = parse_register(l);
            opcode = OPCODE_XOR_REGISTER;
            write_instruction(opcode, regx, regy);
        }else{
            Token number = require_token(l, TOKEN_NUMBER_LITERAL);
            int value = string_to_int(number.str);
            opcode = OPCODE_XOR_CONSTANT;
            write_instruction(opcode, regx,0,  value);
        }
    }else if(match_token(token, "sr0")){
        int opcode = OPCODE_SR0;
        int regx = parse_register(l);
        write_instruction(opcode, regx);
    }else if(match_token(token, "sr1")){
        int opcode = OPCODE_SR1;
        int regx = parse_register(l);
        write_instruction(opcode, regx);
    }else if(match_token(token, "srx")){
        int opcode = OPCODE_SRX;
        int regx = parse_register(l);
        write_instruction(opcode, regx);
    }else if(match_token(token, "sra")){
        int opcode = OPCODE_SRA;
        int regx = parse_register(l);
        write_instruction(opcode, regx);
    }else if(match_token(token, "rr")){
        int opcode = OPCODE_RR;
        int regx = parse_register(l);
        write_instruction(opcode, regx);
    }else if(match_token(token, "sl0")){
        int opcode = OPCODE_SL0;
        int regx = parse_register(l);
        write_instruction(opcode, regx);
    }else if(match_token(token, "sl1")){
        int opcode = OPCODE_SL1;
        int regx = parse_register(l);
        write_instruction(opcode, regx);
    }else if(match_token(token, "slx")){
        int opcode = OPCODE_SLX;
        int regx = parse_register(l);
        write_instruction(opcode, regx);
    }else if(match_token(token, "sla")){
        
        int opcode = OPCODE_SLA;
        int regx = parse_register(l);
        write_instruction(opcode, regx);
    }else if(match_token(token, "rl")){
        
        int opcode = OPCODE_RL;
        int regx = parse_register(l);
        write_instruction(opcode, regx);
    }else if(match_token(token, "store")){
        
        int opcode = OPCODE_SR1;
        int regx = parse_register(l);
        write_instruction(opcode, regx);
    }else if(match_token(token, "fetch")){
        
    }else if(match_token(token, "input")){
        
        int opcode = 0x00000;
        int regx = parse_register(l);
        require_token(l, TOKEN_COMMA);
        require_token(l, TOKEN_OPEN_BRACE);
        int regy = parse_register(l);
        require_token(l, TOKEN_CLOSE_BRACE);
        opcode = OPCODE_INPUT_REGISTER; 
        write_instruction(opcode, regx, regy);
    }else if(match_token(token, "output")){
        int opcode = 0x00000;
        require_keyword(l, "s");
        Token reg = get_token(l);
        int regx = string_to_int(reg.str);
        require_token(l, TOKEN_COMMA);
        require_token(l, TOKEN_OPEN_BRACE);
        require_keyword(l, "s");
        reg = get_token(l);
        int regy = string_to_int(reg.str);
        require_token(l, TOKEN_CLOSE_BRACE);
        opcode = OPCODE_OUTPUT_REGISTER; 
        write_instruction(opcode, regx, regy);
    }else{
    }
}

static void 
parse_labels(Lexer* l, Token token){
    if(expect_token(l, TOKEN_COLON)){
        map_insert(l->label_map, token.str, l->instruction_count);
    }else if(is_instruction(token)){
        l->instruction_count++;
    }
    //print_string(token.str);
}

int main(int argc, char** args){
    char* source = open_source(args[1]);
    
    //printf("%s", source);
    
    Lexer l = {};
    l.pos = source;
    Token token = {};
    bool parsing = true;
    
    while(parsing){
        token = get_token(&l);
        switch(token.type){
            case TOKEN_END:{parsing = false;}break;
            case TOKEN_IDENTIFIER:{
                parse_labels(&l, token);
            }break;
        }
    }
    l.pos = source;
    token = {};
    parsing = true;
    printf("------------------------------------\n");
    while(parsing){
        token = get_token(&l);
        switch(token.type){
            case TOKEN_END:{parsing = false;}break;
            case TOKEN_IDENTIFIER:{
                parse_identifier(&l, token);
            }break;
        }
    }
    printf("Compilation finished");
    return 0;
}