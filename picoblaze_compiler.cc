#include "picoblaze_compiler.h"

#define panic(a) printf("PANIC: %s", a); exit(0)

static char*
open_source(char* filename){
    char* result = 0;
    FILE* file_pointer = fopen(filename, "r");
    
    if(file_pointer){
        fseek(file_pointer, 0, SEEK_END);
        size_t size = ftell(file_pointer);
        fseek(file_pointer, 0, SEEK_SET);
        
        result = (char*)malloc(size+2);
        fread(result, size, 1, file_pointer);
        result[size-1] = '\n';
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

static void
eat_whitespace(Lexer* l){
    while(is_space(*l->pos)){
        l->pos++;
    }
}

static Token
get_token(Lexer* l){
    eat_whitespace(l);
    
    char c = *l->pos;
    Token token = {};
    
    switch(c){
        case ':':{token.type = TOKEN_COLON;l->pos++; } break;
        case ',':{token.type = TOKEN_COMMA; l->pos++;} break;
        case '\0':{token.type = TOKEN_END;}break;
        case ';':{while(!is_newline(*l->pos)){l->pos++;}}break;
        default:{
            token.type = TOKEN_IDENTIFIER;
            if(is_alpha(c)){
                token.text = l->pos;
                while(is_alpha(*l->pos)){
                    l->pos++;
                }
                token.size = l->pos - token.text;
            }
            else if(is_digit(c)){
                token.text = l->pos;
                while(is_digit(*l->pos)){
                    l->pos++;
                }
                token.size = l->pos - token.text;
            }
        }break;
    }
    return token;
}


int main(int argc, char** args){
    char* source = open_source(args[1]);
    
    Lexer l = {};
    l.pos = source;
    
    bool parsing = true;
    Token token = {};
    while(parsing){
        token = get_token(&l);
        switch(token.type){
            case TOKEN_END:{parsing = false;}break;
            case TOKEN_IDENTIFIER:{printf("%.*s\n", token.size, token.text);}break;
        }
    }
    return 0;
}