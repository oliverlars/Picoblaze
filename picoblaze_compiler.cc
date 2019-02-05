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
        
        result = (char*)malloc(size+1);
        fread(result, size, 1, file_pointer);
        result[size] = 0;
        fclose(file_pointer);
    }else {
        panic("unable to open file");
    }
    return result;
}

#define is_newline(x) ((x) == '\n' || (x) == '\v')
#define is_space(x) ((x) == ' ' || (x) == '\t' || is_newline(x))
#define is_digit(x) ((x) <= '9' && (x) >= '0')
#define is_alpha(x) (((x) <= 'Z' && (x) >= 'A') || ((x) <= 'z' && (x) >= 'a'))

#define print_string(x) printf("%.*s\n", x.len, x.text)

static void
eat_whitespace(Lexer* l){
    while(is_space(*l->pos)){
        l->pos++;
    }
}

static bool
match_token(Token token, char* string){
    for(int i = 0; i < token.str.len; i++){
        if(!(token.str.text[i] == string[i])){
            return false;
        }
    }
    return string[token.str.len] == '\0';
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
    
    char c = *l->pos;
    Token token = {};
    
    switch(c){
        case '.':{token.type = TOKEN_FULLSTOP;l->pos++;}break;
        case ':':{token.type = TOKEN_COLON;l->pos++; } break;
        case ',':{token.type = TOKEN_COMMA; l->pos++;} break;
        case '\0':{token.type = TOKEN_END;}break;
        case ';':{while(!is_newline(*l->pos)){l->pos++;}}break;
        default:{
            token.type = TOKEN_IDENTIFIER;
            if(is_alpha(c)){
                token.str.text = l->pos;
                while(is_alpha(*l->pos)){
                    l->pos++;
                }
                token.str.len = l->pos - token.str.text;
            }
            else if(is_digit(c)){
                token.type = TOKEN_NUMBER_LITERAL;
                token.str.text = l->pos;
                while(is_digit(*l->pos)){
                    l->pos++;
                }
                token.str.len = l->pos - token.str.text;
            }
            
        }break;
    }
    return token;
}


static Token
peek_token(Lexer l){
    eat_whitespace(&l);
    
    char c = *l.pos;
    Token token = {};
    
    
    switch(c){
        case '.':{token.type = TOKEN_FULLSTOP;l.pos++;}break;
        case ':':{token.type = TOKEN_COLON;l.pos++; } break;
        case ',':{token.type = TOKEN_COMMA; l.pos++;} break;
        case '\0':{token.type = TOKEN_END;}break;
        case ';':{while(!is_newline(*l.pos)){l.pos++;}}break;
        default:{
            if(is_alpha(c)){
                token.type = TOKEN_IDENTIFIER;
                token.str.text = l.pos;
                while(is_alpha(*l.pos)){
                    l.pos++;
                }
                token.str.len = l.pos - token.str.text;
            }
            else if(is_digit(c)){
                token.type = TOKEN_NUMBER_LITERAL;
                token.str.text = l.pos;
                while(is_digit(*l.pos)){
                    l.pos++;
                }
                token.str.len = l.pos - token.str.text;
            }
            
        }break;
    }
    return token;
}

static Token
require_token(Lexer* l, TokenType type){
    
    Token token = get_token(l);
    if(token.type != type){
        print_string(token.str);
        panic("Unexpected token");
    }
    return token;
}

static Token
require_keyword(Lexer* l, char* keyword){
    Token token = peek_token(*l);
    require_token(l, TOKEN_IDENTIFIER);
    if(!match_token(token, keyword)){
        panic("Unexpected keyword");
    }
    return token;
}

static bool
expect_token(Lexer* l, TokenType type){
    Token token = peek_token(*l);
    return token.type == type;
}

static bool
expect_keyword(Lexer* l, char* keyword){
    Token token = peek_token(*l);
    if(!expect_token(l, TOKEN_IDENTIFIER)){
        return false;
    }
    return match_token(token, keyword);
}

static inline void
skip_token(Lexer* l){
    get_token(l);
}


static void
parse_identifier(Lexer* l, Token token){
    if(match_token(token, "jump")){
        Token condition = get_token(l);
        switch(condition.type){
            case TOKEN_IDENTIFIER:{
                if(match_token(condition, "nz")){
                }else if(match_token(condition, "z")){
                }else if(match_token(condition, "nc")){
                }else if(match_token(condition, "c")){
                }
            }break;
            case TOKEN_NUMBER_LITERAL:{
            }break;
        }
    }else if(match_token(token, "call")){
    }else if(match_token(token, "return")){
    }else if(match_token(token, "add")){
    }else if(match_token(token, "addc")){
    }else if(match_token(token, "sub")){
    }else if(match_token(token, "subc")){
    }else if(match_token(token, "compare")){
    }else if(match_token(token, "load")){
        require_keyword(l, "s");
        Token regx = require_token(l, TOKEN_NUMBER_LITERAL);
        require_token(l, TOKEN_COMMA);
        if(expect_keyword(l, "s")){
            skip_token(l);
            Token regy = require_token(l, TOKEN_NUMBER_LITERAL);
        }else{
            Token value = require_token(l, TOKEN_NUMBER_LITERAL);
        }
    }else if(match_token(token, "and")){
    }else if(match_token(token, "or")){
    }else if(match_token(token, "xor")){
    }else if(match_token(token, "sr0")){
    }else if(match_token(token, "sr1")){
    }else if(match_token(token, "srx")){
    }else if(match_token(token, "sra")){
    }else if(match_token(token, "rr")){
    }else if(match_token(token, "sl0")){
    }else if(match_token(token, "sl1")){
    }else if(match_token(token, "slx")){
    }else if(match_token(token, "sla")){
    }else if(match_token(token, "rl")){
    }else if(match_token(token, "store")){
    }else if(match_token(token, "fetch")){
    }else if(match_token(token, "input")){
    }else if(match_token(token, "output")){
    }else{
        if (!expect_token(l, TOKEN_COLON)){
            panic("Expected colon after label");
        }
        printf("label jump address: %d, string: ", map_get(l->label_map, token.str));
        print_string(token.str);
    }
}

static void 
parse_labels(Lexer* l, Token token){
    if(expect_token(l, TOKEN_COLON)){
        printf("this is a label: ");
        print_string(token.str);
        map_insert(l->label_map, token.str, l->instruction_count);
    }if(is_instruction(token)){
        l->instruction_count++;
        printf("This is an instruction: ");
        print_string(token.str);
    }
}

int main(int argc, char** args){
    char* source = open_source(args[1]);
    
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
    while(parsing){
        token = get_token(&l);
        switch(token.type){
            case TOKEN_END:{parsing = false;}break;
            case TOKEN_IDENTIFIER:{
                print_string(token.str);
                parse_identifier(&l, token);
            }break;
            
        }
    }
    return 0;
}