/* 046267 Computer Architecture - Spring 2017 - HW #1 */
/* This file should hold your implementation of the CPU pipeline core simulator */

#include "sim_api.h"

#define NOP 0
#define ERROR (-1)

/*Globals*/

typedef struct {
    PipeStageState pip;
    int32_t pc_after_fetch;
    int32_t alu_res;
    int32_t mem_data;
    int32_t dest;
    bool branch_taken;
} Buffer;

SIM_coreState CPU;
Buffer wide_pipe[SIM_PIPELINE_DEPTH];





static int __generateNOP(SIM_cmd *nop){
    if(nop ==NULL){
        return ERROR;
    }
    nop->opcode=CMD_NOP;
    nop->dst = NOP;
    nop->isSrc2Imm = false;
    nop->src1=NOP;
    nop->src2=NOP;
    return 0;
}

static int __IF(Buffer *buffer){
    if(buffer == NULL){
        return ERROR;
    }

    (*buffer) = wide_pipe[FETCH];

    PipeStageState instruction;
    instruction.src1Val = NOP;
    instruction.src2Val = NOP;
    SIM_MemInstRead((uint32_t)CPU.pc,&(instruction.cmd));
    CPU.pc+=4;
    wide_pipe[FETCH].pip = instruction;
    wide_pipe->pc_after_fetch = CPU.pc;
    wide_pipe->alu_res = ERROR;
    wide_pipe->mem_data = ERROR;

    CPU.pipeStageState[FETCH] = instruction;

    return 0;
}


static int __ID(Buffer *buffer){
    if(buffer == NULL){
        return ERROR;
    }
    Buffer temp = *buffer;

    (*buffer) = wide_pipe[DECODE];

    wide_pipe[DECODE] = temp;

    CPU.pipeStageState[DECODE] = wide_pipe[DECODE].pip;

    SIM_cmd_opcode cmd_opcode= CPU.pipeStageState[DECODE].cmd.opcode;

    if(cmd_opcode == CMD_NOP ||cmd_opcode == CMD_HALT ){
        return 0;
    }
    int src1 = CPU.pipeStageState[DECODE].cmd.src1;
    int32_t src2 = CPU.pipeStageState[DECODE].cmd.src2;
    bool srcImmd = CPU.pipeStageState[DECODE].cmd.isSrc2Imm;

    CPU.pipeStageState[DECODE].src1Val = CPU.regFile[src1];
    if (srcImmd)
        CPU.pipeStageState[DECODE].src2Val = src2;
    else
        CPU.pipeStageState[DECODE].src2Val = CPU.regFile[src2];

    if(cmd_opcode >= CMD_STORE && cmd_opcode <= CMD_BRNEQ) {
        //dst register hold a memory address
        int dest_reg = CPU.pipeStageState[DECODE].cmd.dst;
        wide_pipe[DECODE].dest = CPU.regFile[dest_reg];
    }
    else
        wide_pipe[DECODE].dest = CPU.pipeStageState->cmd.dst;
    return 0;
}

static int __EXE(Buffer *buffer){
    if(buffer == NULL ){
        return ERROR;
    }
    Buffer temp = *buffer;

    (*buffer) = wide_pipe[EXECUTE];

    wide_pipe[EXECUTE] = temp;

    CPU.pipeStageState[EXECUTE] = wide_pipe[EXECUTE].pip;

    SIM_cmd_opcode cmd_opcode= CPU.pipeStageState[EXECUTE].cmd.opcode;

    if(cmd_opcode == CMD_NOP ||cmd_opcode == CMD_HALT ){
        return 0;
    }

    if(cmd_opcode == CMD_ADD || cmd_opcode == CMD_ADDI){
        wide_pipe[EXECUTE].alu_res = CPU.pipeStageState[EXECUTE].src1Val +
                                     CPU.pipeStageState[EXECUTE].src2Val;
        return 0;
    }

    if(cmd_opcode == CMD_SUB || cmd_opcode == CMD_SUBI){
        wide_pipe[EXECUTE].alu_res = CPU.pipeStageState[EXECUTE].src1Val -
                                     CPU.pipeStageState[EXECUTE].src2Val;
        return 0;
    }

    if(cmd_opcode == CMD_LOAD){
        wide_pipe[EXECUTE].alu_res = CPU.pipeStageState[EXECUTE].src1Val +
                                     CPU.pipeStageState[EXECUTE].src2Val;
        return 0;
    }

    if(cmd_opcode == CMD_STORE){
        wide_pipe[EXECUTE].alu_res = wide_pipe[EXECUTE].dest +
                                     CPU.pipeStageState[EXECUTE].src2Val;
        return 0;
    }

    if(cmd_opcode >= CMD_BR && cmd_opcode <= CMD_BRNEQ){
        wide_pipe[EXECUTE].alu_res = wide_pipe[EXECUTE].pc_after_fetch +
                                     wide_pipe[EXECUTE].dest;
        if(cmd_opcode == CMD_BRNEQ){
            wide_pipe[EXECUTE].branch_taken = (CPU.pipeStageState->src1Val!=
                                               CPU.pipeStageState->src2Val);
            return 0;

        }
        if(cmd_opcode == CMD_BREQ){
            wide_pipe[EXECUTE].branch_taken = (CPU.pipeStageState->src1Val==
                                               CPU.pipeStageState->src2Val);
            return 0;

        }
        assert(cmd_opcode == CMD_BR);
        wide_pipe[EXECUTE].branch_taken = true;
        return 0;
    }
    return 0;
}


int __MEM(Buffer *buffer) {
    if(buffer == NULL){
        return ERROR;
    }

    Buffer temp = *buffer;

    (*buffer) = wide_pipe[MEMORY];

    wide_pipe[MEMORY] = temp;

    CPU.pipeStageState[MEMORY] = temp.pip;

    SIM_cmd_opcode cmd_opcode= CPU.pipeStageState[MEMORY].cmd.opcode;

    if(cmd_opcode == CMD_NOP ||cmd_opcode == CMD_HALT ){
        return 0;
    }

    if(cmd_opcode <= CMD_SUBI || cmd_opcode >= CMD_BR ){
        // commands that does NOT use memory.
        return 0;
    }
    if(cmd_opcode == CMD_STORE){
        uint32_t write_addr = (uint32_t)wide_pipe[MEMORY].alu_res;
        SIM_MemDataWrite(write_addr,CPU.pipeStageState[MEMORY].src1Val);
    }

    if(cmd_opcode == CMD_LOAD){
        uint32_t read_addr = (uint32_t)wide_pipe[MEMORY].alu_res;
        SIM_MemDataRead(read_addr,&(wide_pipe[MEMORY].mem_data));
    }
    if(cmd_opcode >= CMD_BR && cmd_opcode <= CMD_BRNEQ){
        if(wide_pipe[MEMORY].branch_taken == true){
            uint32_t next_inst_addr = (uint32_t)wide_pipe[MEMORY].alu_res;
            CPU.pc = next_inst_addr;
        }
        return 0;
    }
    return 0;
}

int __WB(Buffer *buffer) {
    if(buffer == NULL){
        return ERROR;
    }
    wide_pipe[WRITEBACK] = (*buffer);
    CPU.pipeStageState[WRITEBACK] = buffer->pip;

    SIM_cmd_opcode cmd_opcode= CPU.pipeStageState[MEMORY].cmd.opcode;

    if(cmd_opcode == CMD_NOP ||cmd_opcode == CMD_HALT ){
        return 0;
    }

    if(cmd_opcode >= CMD_STORE){
        //command does NOT use WriteBack
        return 0;
    }
    int wb_value, wb_reg = wide_pipe[WRITEBACK].dest;
    if(cmd_opcode == CMD_LOAD){
        wb_value = wide_pipe[WRITEBACK].mem_data;
    }
    else{
        wb_value = wide_pipe[WRITEBACK].alu_res;
    }
    CPU.regFile[wb_reg] = wb_value;
    return 0;
}



/*! SIM_CoreReset: Reset the processor core simulator machine to start new simulation
  Use this API to initialize the processor core simulator's data structures.
  The simulator machine must complete this call with these requirements met:
  - PC = 0  (entry point for a program is at address 0)
  - All the register file is cleared (all registers hold 0)
  - The value of IF is the instuction in address 0x0
  \returns 0 on success. <0 in case of initialization failure.
*/
int SIM_CoreReset(void) {
    CPU.pc = NOP;

    for (int r = 0; r < SIM_REGFILE_SIZE; r++) {
        CPU.regFile[r] = NOP;
    }

    SIM_cmd nop;
    if (__generateNOP(&nop) == ERROR) {
        return ERROR;
    }

    for (int p = 0; p < SIM_PIPELINE_DEPTH; p++) {
        CPU.pipeStageState[p].cmd = nop;
        CPU.pipeStageState[p].src1Val = NOP;
        CPU.pipeStageState[p].src2Val = NOP;
        wide_pipe[p].pip = CPU.pipeStageState[p];
        wide_pipe[p].mem_data = NOP;
        wide_pipe[p].alu_res = NOP;
        wide_pipe[p].pc_after_fetch = NOP;
        wide_pipe[p].dest = NOP;
        wide_pipe[p].branch_taken = false;
    }
    return 0;
}

/*! SIM_CoreClkTick: Update the core simulator's state given one clock cycle.
  This function is expected to update the core pipeline given a clock cycle event.
*/
void SIM_CoreClkTick() {
    SIM_MemClkTick();
    Buffer buffer;
    buffer.mem_data = ERROR;//default value;
    if (__IF(&buffer) == ERROR){
        return;
    }
    //Buffer hold instruction after IF
    if(__ID(&buffer) == ERROR){
        return;
    }
    //Buffer hold instruction after ID
    if(__EXE(&buffer) == ERROR){
        return;
    }
    //Buffer hold instruction after EXE with ALU Result
    if(__MEM(&buffer) == ERROR){
        return;
    }
    //Buffer hold instruction after MEM with ALU Result
    if(__WB(&buffer) == ERROR){
        return;
    }
}



/*! SIM_CoreGetState: Return the current core (pipeline) internal state
    curState: The returned current pipeline state
    The function will return the state of the pipe at the end of a cycle
*/
void SIM_CoreGetState(SIM_coreState *curState) {

    if(curState == NULL){
        return;
    }

    curState->pc = CPU.pc;

    for (int r=0; r < SIM_REGFILE_SIZE ; r++) {
        curState->regFile[r] = CPU.regFile[r];
    }

    for (int p=0; p < SIM_PIPELINE_DEPTH; p++){
        curState->pipeStageState[p] = CPU.pipeStageState[p];
    }
}

