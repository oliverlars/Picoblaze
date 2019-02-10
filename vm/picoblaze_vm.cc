#define REGSIZE 16
#define STACKSIZE 32
#define RAMSIZE 256
#define MEMSIZE 64

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
        fclose(file_pointer);
    }
}

#define get_opcode(x) ((x) & 0x000FF000)
#define get_regx(x) (((x) & 0x00000F00) >> 8)
#define get_regy(x) (((x) & 0x000000F0) >> 4)
#define get_address(x) ((x) & 0x00000FFF)
#define get_constant(x) ((x) & 0x000000FF)


static void
display_memory(unsigned char* reg, unsigned char* ram, unsigned int* mem){
    printf("\nSTACK-------------------------\n");
    for(int i = 0; i < REGSIZE; i++){
        printf("%03x ", reg[i]);
    }
    printf("\n\nRAM---------------------------\n");
    for(int i = 0; i < RAMSIZE; i+=16){
        for(int j = 0; j < 16; j++){
            printf("%03x ", ram[i+j]);
        }
        printf("\n");
    }
}

static void
push_stack(unsigned int* stack,unsigned int* stack_ptr, int value){
    stack[*stack_ptr] = value;
    (*stack_ptr)++;
}

static int
pop_stack(unsigned int* stack, unsigned int* stack_ptr){
    (*stack_ptr)--;
    int result = stack[*stack_ptr];
    return result;
}

int main(int argc, char** args){
    
    unsigned char reg[REGSIZE] = {0};
    unsigned char ram[RAMSIZE] = {0};
    unsigned int mem[MEMSIZE] = {0};
    
    unsigned int stack[STACKSIZE] = {0};
    unsigned int stack_ptr = 0;
    unsigned char pc = 0;
    Status status = {};
    
    open_object_file(args[1], mem);
    
    bool running = true;
    while(running){
        unsigned instruction = mem[pc];
        unsigned opcode = get_opcode(instruction);
        switch(opcode){
            case OPCODE_LOAD_CONSTANT:{
                int regx = get_regx(instruction);
                int constant = get_constant(instruction);
                reg[regx] = constant;
            }break;
            case OPCODE_LOAD_REGISTER:{
                int regx = get_regx(instruction);
                int regy = get_regy(instruction);
                reg[regx] = reg[regy];
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
                unsigned regx = get_regx(instruction);
                unsigned constant = get_constant(instruction);
                unsigned value = reg[regx] + constant;
                if(value < reg[regx]) {
                    status.carry = true;
                }else{
                    status.carry = true;
                }
                reg[regx] = value;
                
            }break;
            case OPCODE_SUB_CONSTANT:{
                int regx = get_regx(instruction);
                int constant = get_constant(instruction);
                reg[regx] -= constant;
                if((int16_t)reg[regx] - (int16_t)constant < 0){
                    status.carry = true;
                }else{
                    status.carry = false;
                }
                if(reg[regx] == 0) {
                    status.zero = true;
                }else{
                    status.zero = false;
                }
            }break;
            case OPCODE_SUB_REGISTER:{
                int regx = get_regx(instruction);
                int regy = get_regy(instruction);
                if((int16_t)reg[regx] - (int16_t)reg[regy] < 0){
                    status.carry = true;
                }else{
                    status.carry = false;
                }
                reg[regx] -= reg[regy];
                if(reg[regx] == 0) {
                    status.zero = true;
                }else{
                    status.zero = false;
                }
            }break;
            case OPCODE_INPUT_REGISTER:{
                int regx = get_regx(instruction);
                int regy = get_regy(instruction);
                reg[regx] = ram[reg[regy]];
            }break;
            case OPCODE_OUTPUT_REGISTER:{
                int regx = get_regx(instruction);
                int regy = get_regy(instruction);
                ram[reg[regy]] = reg[regx];
            }break;
            case OPCODE_HALT:{
                running = false;
            }break;
            case OPCODE_CALL_U:{
                push_stack(stack, &stack_ptr, pc);
                int address = get_address(instruction);
                pc = --address;
            }break;
            case OPCODE_CALL_Z:{
                if(status.zero){
                    push_stack(stack, &stack_ptr, pc);
                    int address = get_address(instruction);
                    pc = --address;
                }
            }break;
            case OPCODE_CALL_NZ:{
                if(!status.zero){
                    push_stack(stack, &stack_ptr, pc);
                    int address = get_address(instruction);
                    pc = --address;
                }
            }break;
            case OPCODE_CALL_C:{
                if(status.carry){
                    push_stack(stack, &stack_ptr, pc);
                    int address = get_address(instruction);
                    pc = --address;
                }
            }break;
            case OPCODE_CALL_NC:{
                if(!status.carry){
                    push_stack(stack, &stack_ptr, pc);
                    int address = get_address(instruction);
                    pc = --address;
                }
            }break;
            case OPCODE_RETURN_U:{
                pc = pop_stack(stack, &stack_ptr);
            }break;
            case OPCODE_RETURN_Z:{
                if(status.zero){
                    pc = pop_stack(stack, &stack_ptr);
                }
            }break;
            case OPCODE_RETURN_NZ:{
                if(!status.zero){
                    pc = pop_stack(stack, &stack_ptr);
                }
            }break;
            case OPCODE_RETURN_C:{
                if(status.carry){
                    pc = pop_stack(stack, &stack_ptr);
                }
            }break;
            case OPCODE_RETURN_NC:{
                if(!status.carry){
                    pc = pop_stack(stack, &stack_ptr);
                }
            }break;
        }
        pc++;
    }
    display_memory(reg, ram, mem);
    return 0;
}