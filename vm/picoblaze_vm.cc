#include "picoblaze_vm.h"


static void
open_object_file(char* filename,unsigned int* mem){
    
    FILE* file_pointer = fopen(filename, "rb");
    
    if(file_pointer){
        fseek(file_pointer, 0, SEEK_END);
        size_t size = ftell(file_pointer);
        fseek(file_pointer, 0, SEEK_SET);
        
        fread(mem, size, 1, file_pointer);
        fclose(file_pointer);
    }else {
        printf("Unable to open file");
    }
}

#define get_opcode(x) ((x) & 0x000FF000)
#define get_regx(x) (((x) & 0x00000F00) >> 8)
#define get_regy(x) (((x) & 0x000000F0) >> 4)
#define get_address(x) ((x) & 0x00000FFF)
#define get_constant(x) ((x) & 0x000000FF)

#define STACKSIZE 16
#define RAMSIZE 256
#define MEMSIZE 64

static void
display_memory(unsigned char* stack, unsigned char* ram, unsigned int* mem){
    printf("\nSTACK-------------------------\n");
    for(int i = 0; i < STACKSIZE; i++){
        printf("%03x ", stack[i]);
    }
    printf("\n\nRAM---------------------------\n");
    for(int i = 0; i < RAMSIZE; i+=16){
        for(int j = 0; j < 16; j++){
            printf("%03x ", ram[i+j]);
        }
        printf("\n");
    }
}

int main(int argc, char** args){
    
    unsigned char stack[STACKSIZE] = {};
    unsigned char ram[RAMSIZE] = {};
    unsigned int mem[MEMSIZE] = {};
    
    unsigned char pc = 0;
    Status status = {};
    
    open_object_file(args[1], mem);
    
    bool running = true;
    
    while(running){
        unsigned int instruction = mem[pc];
        unsigned int opcode = get_opcode(instruction);
        switch(opcode){
            case OPCODE_LOAD_CONSTANT:{
                int regx = get_regx(instruction);
                int constant = get_constant(instruction);
                stack[regx] = constant;
            }break;
            case OPCODE_LOAD_REGISTER:{
                int regx = get_regx(instruction);
                int regy = get_regy(instruction);
                stack[regx] = stack[regy];
            }break;
            case OPCODE_JUMP_U:{
                int address = get_address(instruction);
                pc = --address;
            }break;
            case OPCODE_JUMP_NZ:{
                int address = get_address(instruction);
                if(!status.zero){
                    pc = --address;
                }
            }break;
            case OPCODE_JUMP_Z:{
                int address = get_address(instruction);
                if(status.zero){
                    pc = --address;
                }
            }break;
            case OPCODE_JUMP_NC:{
                int address = get_address(instruction);
                if(!status.carry){
                    pc = --address;
                }
            }break;
            case OPCODE_ADD_CONSTANT:{
                int regx = get_regx(instruction);
                int constant = get_constant(instruction);
                unsigned value = stack[regx] + constant;
                if(value < stack[regx]) {
                    status.carry = true;
                }else{
                    status.carry = true;
                }
                stack[regx] = value;
                
            }break;
            case OPCODE_SUB_CONSTANT:{
                int regx = get_regx(instruction);
                int constant = get_constant(instruction);
                stack[regx] -= constant;
                if(stack[regx] == 0) {
                    status.zero = true;
                }else{
                    status.zero = false;
                }
            }break;
            case OPCODE_SUB_REGISTER:{
                int regx = get_regx(instruction);
                int regy = get_regy(instruction);
                if((int16_t)stack[regx] - (int16_t)stack[regy] < 0){
                    status.carry = true;
                }else{
                    status.carry = false;
                }
                stack[regx] -= stack[regy];
                if(stack[regx] == 0) {
                    status.zero = true;
                }else{
                    status.zero = false;
                }
            }break;
            case OPCODE_INPUT_REGISTER:{
                int regx = get_regx(instruction);
                int regy = get_regy(instruction);
                stack[regx] = ram[stack[regy]];
            }break;
            case OPCODE_OUTPUT_REGISTER:{
                int regx = get_regx(instruction);
                int regy = get_regy(instruction);
                ram[stack[regy]] = stack[regx];
            }break;
            case OPCODE_HALT:{
                running = false;
            }break;
        }
        pc++;
    }
    display_memory(stack, ram, mem);
    return 0;
}