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
    INPUT=0,
    OUTPUT,
    ADDK,
    ADDP,
    LOAD,
    JUMPU,JUMPZ, JUMPC, JUMPNZ, JUMPNC
};

char* Instructions[] =
{
    "input", "output", "addk","addp", "load", "jumpu", "jumpz", "jumpc", "jumpnz", "jumpnc",
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

static void
NumTo8BitBin(int Num, char* Result){
    for(int I = 8; I >= 0; I--){
        int K = Num >> I;
        
        if(K & 1) Result[I] = '1';
        else Result[I] = '0';
    }
    Result[9] = 0;
}

int main(int argc, char** args){
    char* Source = OpenSource(args[1]);
    
    char* Pos = Source;
    bool Parsing = true;
    while(Parsing){
        if(*Pos){
            char Instruction[17];
            Instruction[17] = 0;
            for(int I = 0; I < 16; I++) Instruction[I] = '0';
            if(IsAlpha(*Pos)){
                token Token = {};
                char* StartPos = Pos;
                while(IsAlpha(*Pos)){
                    Pos++;
                }
                Token.Size = Pos - StartPos;
                Token.Data = StartPos;
                if(TokenMatch(Token, LOAD)){
                    Token.Type = LOAD;
                    token Num = {};
                    while(IsWhitespace(*Pos)) Pos++;
                    char* StartPos = Pos;
                    while(IsNumber(*Pos)){
                        Pos++;
                    }
                    Num.Size = Pos - StartPos;
                    Num.Data = StartPos;
                    
                    int Addr = TokenToNumber(Num);
                    NumTo8BitBin(Addr, &Instruction[8]);
                    printf("DATA_RAM_WORD'(\"%s\"),\n", Instruction);
                }
                if(TokenMatch(Token, OUTPUT)){
                    Token.Type = OUTPUT;
                    token Num = {};
                    while(IsWhitespace(*Pos)) Pos++;
                    char* StartPos = Pos;
                    while(IsNumber(*Pos)){
                        Pos++;
                    }
                    Num.Size = Pos - StartPos;
                    Num.Data = StartPos;
                    Instruction[0] = '1';
                    Instruction[1] = '1';
                    Instruction[2] = '1';
                    int Addr = TokenToNumber(Num);
                    NumTo8BitBin(Addr, &Instruction[8]);
                    printf("DATA_RAM_WORD'(\"%s\"),\n", Instruction);
                    
                }
                
                if(TokenMatch(Token, ADDK)){
                    Token.Type = ADDK;
                    token Num = {};
                    while(IsWhitespace(*Pos)) Pos++;
                    char* StartPos = Pos;
                    while(IsNumber(*Pos)){
                        Pos++;
                    }
                    Num.Size = Pos - StartPos;
                    Num.Data = StartPos;
                    Instruction[1] = '1';
                    int Addr = TokenToNumber(Num);
                    NumTo8BitBin(Addr, &Instruction[8]);
                    printf("DATA_RAM_WORD'(\"%s\"),\n", Instruction);
                }
                
                if(TokenMatch(Token, ADDP)){
                    Token.Type = ADDP;
                    token Num = {};
                    while(IsWhitespace(*Pos)) Pos++;
                    char* StartPos = Pos;
                    while(IsNumber(*Pos)){
                        Pos++;
                    }
                    Num.Size = Pos - StartPos;
                    Num.Data = StartPos;
                    Instruction[0] = '1';
                    Instruction[1] = '1';
                    int Addr = TokenToNumber(Num);
                    NumTo8BitBin(Addr, &Instruction[8]);
                    printf("DATA_RAM_WORD'(\"%s\"),\n", Instruction);
                }
                
                if(TokenMatch(Token, JUMPU)){
                    Token.Type = JUMPU;
                    token Num = {};
                    while(IsWhitespace(*Pos)) Pos++;
                    char* StartPos = Pos;
                    while(IsNumber(*Pos)){
                        Pos++;
                    }
                    Num.Size = Pos - StartPos;
                    Num.Data = StartPos;
                    Instruction[0] = '1';
                    int Addr = TokenToNumber(Num);
                    NumTo8BitBin(Addr, &Instruction[8]);
                    printf("DATA_RAM_WORD'(\"%s\"),\n", Instruction);
                }
                
                if(TokenMatch(Token, JUMPZ)){
                    Token.Type = JUMPZ;
                    token Num = {};
                    while(IsWhitespace(*Pos)) Pos++;
                    char* StartPos = Pos;
                    while(IsNumber(*Pos)){
                        Pos++;
                    }
                    Num.Size = Pos - StartPos;
                    Num.Data = StartPos;
                    Instruction[0] = '1';
                    Instruction[3] = '1';
                    int Addr = TokenToNumber(Num);
                    NumTo8BitBin(Addr, &Instruction[8]);
                    printf("DATA_RAM_WORD'(\"%s\"),\n", Instruction);
                }
                
                
                if(TokenMatch(Token, JUMPC)){
                    Token.Type = JUMPC;
                    token Num = {};
                    while(IsWhitespace(*Pos)) Pos++;
                    char* StartPos = Pos;
                    while(IsNumber(*Pos)){
                        Pos++;
                    }
                    Num.Size = Pos - StartPos;
                    Num.Data = StartPos;
                    Instruction[0] = '1';
                    Instruction[3] = '1';
                    Instruction[4] = '1';
                    int Addr = TokenToNumber(Num);
                    NumTo8BitBin(Addr, &Instruction[8]);
                    printf("DATA_RAM_WORD'(\"%s\"),\n", Instruction);
                }
                
                if(TokenMatch(Token, JUMPNZ)){
                    Token.Type = JUMPNZ;
                    token Num = {};
                    while(IsWhitespace(*Pos)) Pos++;
                    char* StartPos = Pos;
                    while(IsNumber(*Pos)){
                        Pos++;
                    }
                    Num.Size = Pos - StartPos;
                    Num.Data = StartPos;
                    Instruction[0] = '1';
                    Instruction[3] = '1';
                    Instruction[5] = '1';
                    int Addr = TokenToNumber(Num);
                    NumTo8BitBin(Addr, &Instruction[8]);
                    printf("DATA_RAM_WORD'(\"%s\"),\n", Instruction);
                }
                
                
                if(TokenMatch(Token, JUMPNC)){
                    Token.Type = JUMPNC;
                    token Num = {};
                    while(IsWhitespace(*Pos)) Pos++;
                    char* StartPos = Pos;
                    while(IsNumber(*Pos)){
                        Pos++;
                    }
                    Num.Size = Pos - StartPos;
                    Num.Data = StartPos;
                    Instruction[0] = '1';
                    Instruction[3] = '1';
                    Instruction[4] = '1';
                    Instruction[5] = '1';
                    int Addr = TokenToNumber(Num);
                    NumTo8BitBin(Addr, &Instruction[8]);
                    printf("DATA_RAM_WORD'(\"%s\"),\n", Instruction);
                }
                
            }
            Pos++;
        }else{
            Parsing = false;
        }
    }
    
    return 0;
}