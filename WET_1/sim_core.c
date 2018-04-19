/* 046267 Computer Architecture - Spring 2017 - HW #1 */
/* This file should hold your implementation of the CPU pipeline core simulator */

#include "sim_api.h"

#define NOP 0
#define ERROR -1

/*Globals*/

SIM_coreState CPU;

static int generateNOP(SIM_cmd *nop){
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

    for(int r = 0; r < SIM_REGFILE_SIZE; r++ ) {
        CPU.regFile[r] = NOP;
    }

    for(int p = 0; p < SIM_PIPELINE_DEPTH; p++ ) {
        if(generateNOP(&(CPU.pipeStageState[p].cmd)) == ERROR){
            return ERROR;
        }
        CPU.pipeStageState[p].src1Val = NOP;
        CPU.pipeStageState[p].src2Val = NOP;
    }
    return 0;
}

/*! SIM_CoreClkTick: Update the core simulator's state given one clock cycle.
  This function is expected to update the core pipeline given a clock cycle event.
*/
void SIM_CoreClkTick() {











    return;
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

