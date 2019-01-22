#include <stdio.h>
#include <stdlib.h>

static char*
OpenSource(char* filename){
    char* Result = 0;
    FILE* FilePointer = fopen(filename, "r");
    
    if(FilePointer){
        fseek(FilePointer, 0, SEEK_END);
        size_t FileSize = ftell(FilePointer);
        fseek(FilePointer, 0, SEEK_SET);
        
        Result = (char*)malloc(FileSize+1);
        fread(Result, FileSize, 1, FilePointer);
        Result[FileSize] = 0;
        fclose(FilePointer);
    }else printf("Panic! unable to open file");
    return Result;
}

enum token_type{
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
    
    ADD,
    JUMP
};

char* Instructions[] =
{
    "load", "k", "p", "input", "output", "u", "z", "c", "nz", "nc",
    "add", "jump"
};

char* InstructionOpcodes[] =
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

struct token{
    token_type Type;
    char* Data;
    int Size;
};

bool TokenMatch(token Token, token_type Type){
    char* Pos = Instructions[Type];
    for(int Index =  0; Index < Token.Size; Index++, Pos++){
        if((*Pos == 0)||(Token.Data[Index] != *Pos)){
            return false;
        }
    }
    bool Result = (*Pos == 0);
    return Result;
}

bool IsWhitespace(char C){
    return C == ' ' || C == '\t' || C == '\n' || C == '\r' || C== '\v' || C== '\f';
}

bool IsAlpha(char C){
    return (C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z');
}

bool IsNumber(char C){
    return (C <= '9') && (C >= '0');
}
int Pow(int base, int exponent){
    int Result = 1;
    for(int I = 1; I < exponent; I++){
        Result *= base;
    }
    return Result;
}
int TokenToNumber(token Token){
    int Result = 0;
    for(int I = 0; I < Token.Size; I++){
        Result += (Token.Data[I] - '0') * Pow(10, (Token.Size - I));
    }
    return Result;
}

struct lexer{
    char* Pos;
};

token ParseKeyword(lexer* L){
    token Keyword = {};
    
    while(IsWhitespace(*L->Pos)) L->Pos++;
    char* StartPos = L->Pos;
    while(IsAlpha(*L->Pos)) L->Pos++;
    Keyword.Size = L->Pos - StartPos;
    Keyword.Data = StartPos;
    for(int Type =LOAD; Type <= JUMP; Type++){
        if(TokenMatch(Keyword, (token_type)Type)){
            Keyword.Type = (token_type)Type;
            break;
        }
    }
    
    return Keyword;
}

int ParseNumber(lexer* L){
    token Number = {};
    
    while(IsWhitespace(*L->Pos)) L->Pos++;
    char* StartPos = L->Pos;
    while(IsNumber(*L->Pos)) L->Pos++;
    Number.Size = L->Pos - StartPos;
    Number.Data = StartPos;
    
    return TokenToNumber(Number);
}

static void
NumTo8BitBin(int Num, char* Result){
    int NoBits = 7;
    for(int I = NoBits; I >= 0; I--){
        int K = Num >> I;
        if(K & 1) Result[NoBits-I] = '1';
        else Result[NoBits-I] = '0';
    }
}

void EmitBinary(int Num, token_type Type){
    char Operand[] = "00000000";
    NumTo8BitBin(Num, Operand);
    
    printf("DATA_RAM_WORD'(\"%.*s", 8,InstructionOpcodes[Type]);
    printf("%.*s\")\n", 8,Operand); 
}

int main(int argc, char** args){
    char* Source = OpenSource(args[1]);
    lexer Lex = {Source};
    char* Pos = Source;
    bool Parsing = true;
    int LineNo = 1;
    while(Parsing){
        if(*Lex.Pos){
            if(IsAlpha(*Lex.Pos)){
                token Current = ParseKeyword(&Lex);
                switch(Current.Type){
                    case LOAD:{
                        int Constant = ParseNumber(&Lex);
                        EmitBinary(Constant, LOAD);
                    }break;
                    case INPUT:{
                        int Address = ParseNumber(&Lex);
                        EmitBinary(Address, INPUT);
                    }break;
                    case OUTPUT:{
                        int Address = ParseNumber(&Lex);
                        EmitBinary(Address, OUTPUT);
                    }break;
                    case ADD:{
                        token Mode = ParseKeyword(&Lex);
                        switch(Mode.Type){
                            case ADD_K:{
                                int Constant = ParseNumber(&Lex);
                                EmitBinary(Constant, ADD_K);
                            }break;
                            case ADD_P:{
                                int Address = ParseNumber(&Lex);
                                EmitBinary(Address, ADD_P);
                            }break;
                        }
                    }break;
                    case JUMP:{
                        token Condition = ParseKeyword(&Lex);
                        switch(Condition.Type){
                            case JUMP_Z:{
                                int Address = ParseNumber(&Lex);
                                EmitBinary(Address, JUMP_Z);
                            }break;
                            case JUMP_C:{
                                int Address = ParseNumber(&Lex);
                                EmitBinary(Address, JUMP_C);
                            }break;
                            case JUMP_NZ:{
                                int Address = ParseNumber(&Lex);
                                EmitBinary(Address, JUMP_NZ);
                            }break;
                            case JUMP_NC:{
                                int Address = ParseNumber(&Lex);
                                EmitBinary(Address, JUMP_NC);
                            }break;
                            case JUMP_U:{
                                int Address = ParseNumber(&Lex);
                                EmitBinary(Address, JUMP_U);
                            }break;
                        }break;
                    }
                }
            }
            Lex.Pos++;
        }else{
            Parsing = false;
        }
    }
    
    return 0;
}