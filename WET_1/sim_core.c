/* 046267 Computer Architecture - Spring 2017 - HW #1 */
/* This file should hold your implementation of the CPU pipeline core simulator */

#include "sim_api.h"

#define NOP 0
#define ERROR (-1)

/*!struct that make the buffers in the pip with more signals*/
typedef struct {
    PipeStageState pip;
    int32_t pc_after_fetch;
    int32_t alu_res;
    int32_t mem_data;
    int32_t dest;
    bool branch_taken;
} Buffer;

/*!Globals variables that describe the CPU pipeline*/
SIM_coreState CORE;
Buffer wide_pipe[SIM_PIPELINE_DEPTH];//pipeline buffers

/*!signals*/
bool hazard_signal;//HDU signal

bool memoryFailiureSignal;//memory signal

int writeBackValue;//WB value at the end of pip
int writeBackRegister;//WB register at the end of pip
bool writeBackSignal;//Flag that indicate if need to WB when clock rise

uint32_t branchAddress;//new pc after branch
bool branchSignal;//Flag that indicate if need to branch in next IF

bool forwardingMEMSignal;
bool forwardingWBSignal;

/*This Function get buffer and inject into it a NOP*/
static int _generateNOP_(Buffer *nop){
    if(nop ==NULL){
        return ERROR;
    }
    nop->pip.cmd.opcode = CMD_NOP;
    nop->pip.cmd.src1 = NOP;
    nop->pip.cmd.src2 = NOP;
    nop->pip.cmd.dst = NOP;
    nop->pip.cmd.isSrc2Imm = false;
    nop->pip.src1Val = NOP;
    nop->pip.src2Val = NOP;
    nop->alu_res = NOP;
    nop ->dest = NOP;
    nop ->mem_data = NOP;
    nop->pc_after_fetch = NOP;
    nop->branch_taken = false;
    return 0;
}

/*!flushing the pipeline buffers from IF to EXE and updating PC to branch address*/
static void _flush_pip_(){
    Buffer nop;
    _generateNOP_(&nop);
    for (int p = FETCH; p <= EXECUTE ; p++){
        wide_pipe[p] = nop;
        CORE.pipeStageState[p] = nop.pip;
    }
    CORE.pc = branchAddress;
}

/*!This function is the hazard detection unit*/
static bool _hazard_detect_unit_(PipeStageState *stage, pipeStage stage_name){
    hazard_signal = false;
    forwardingMEMSignal = false;
    forwardingWBSignal = false;
    SIM_cmd_opcode opcode = stage->cmd.opcode;
    if(opcode == CMD_NOP || opcode == CMD_HALT || opcode == CMD_BR
       || opcode == CMD_BREQ || opcode == CMD_BRNEQ || branchSignal){
        return false;
    }
    if(stage->cmd.dst == CORE.pipeStageState[DECODE].cmd.src1 ||
       (!CORE.pipeStageState[DECODE].cmd.isSrc2Imm && stage->cmd.dst == CORE.pipeStageState[DECODE].cmd.src2)){
        //RAW hazard detected
        if(stage_name == EXECUTE){
            hazard_signal = true;
            if(forwarding){
                if(stage->cmd.opcode !=CMD_LOAD){
                    hazard_signal = false;
                    forwardingMEMSignal = true;
                }
                else{
                    assert(stage->cmd.opcode ==CMD_LOAD);
                    forwardingMEMSignal = false;
                }
            }
            return true;
        }
        if(stage_name == MEMORY){
            hazard_signal = true;
            if(forwarding){
                hazard_signal = false;
                forwardingWBSignal = true;
            }
            return true;
        }
        if(stage_name == WRITEBACK){
            hazard_signal = true;
            return true;
        }
    }
    return false;
}

static void _forwarding_unit_(Buffer *from){
    int forwardData;
    if(from->pip.cmd.opcode == CMD_LOAD){
        forwardData = from->mem_data;
    }
    else{
        forwardData = from->alu_res;
    }

    if(wide_pipe[EXECUTE].pip.cmd.src1 == from->pip.cmd.dst)
        wide_pipe[EXECUTE].pip.src1Val = forwardData;
    if(wide_pipe[EXECUTE].pip.cmd.src2 == from->pip.cmd.dst)
        wide_pipe[EXECUTE].pip.src2Val = forwardData;
    forwardingWBSignal = false;
    forwardingMEMSignal = false;
}



/*!This function is to handling the FETCH stage in the pipeline*/
static int _IF_(Buffer *buffer){
    if(buffer == NULL){
        return ERROR;
    }

    if(memoryFailiureSignal || hazard_signal) {
        return 0;
    }

    if(branchSignal){
        _flush_pip_();
        hazard_signal = false;
        branchSignal = false;
    }

     CORE.pc+=4;
    (*buffer) = wide_pipe[FETCH];
    PipeStageState instruction;
    instruction.src1Val = NOP;
    instruction.src2Val = NOP;
    SIM_MemInstRead((uint32_t)CORE.pc,&(instruction.cmd));
    wide_pipe[FETCH].pip = instruction;
    wide_pipe->pc_after_fetch = CORE.pc;
    wide_pipe->alu_res = ERROR;
    wide_pipe->mem_data = ERROR;
    CORE.pipeStageState[FETCH] = instruction;
    return 0;
}

/*!This function is to handling the DECODE stage in the pipeline*/
static int _ID_(Buffer *buffer, bool advancePip){
    if(buffer == NULL) {
        return ERROR;
    }

    if(advancePip){
        if (memoryFailiureSignal || hazard_signal){
            assert(buffer != NULL);
            _generateNOP_(buffer);
            hazard_signal = false;
        }
        else{
            Buffer temp = *buffer;
            (*buffer) = wide_pipe[DECODE];
            wide_pipe[DECODE] = temp;
            CORE.pipeStageState[DECODE] = wide_pipe[DECODE].pip;
        }
            return 0;
    }

    SIM_cmd_opcode cmd_opcode= wide_pipe[DECODE].pip.cmd.opcode;

    if(cmd_opcode == CMD_NOP ||cmd_opcode == CMD_HALT ){
        return 0;
    }

    int src1 = wide_pipe[DECODE].pip.cmd.src1;
    int32_t src2 =  wide_pipe[DECODE].pip.cmd.src2;
    bool srcImmd =  wide_pipe[DECODE].pip.cmd.isSrc2Imm;

    wide_pipe[DECODE].pip.src1Val = CORE.regFile[src1];
    if (srcImmd)
        wide_pipe[DECODE].pip.src2Val = src2;
    else
        wide_pipe[DECODE].pip.src2Val = CORE.regFile[src2];

    if(cmd_opcode >= CMD_STORE && cmd_opcode <= CMD_BRNEQ) {
        //dst register hold a memory address
        int destRegister = wide_pipe[DECODE].pip.cmd.dst;
        wide_pipe[DECODE].dest = CORE.regFile[destRegister];
    }
    else
        wide_pipe[DECODE].dest = wide_pipe[DECODE].pip.cmd.dst;

    CORE.pipeStageState[DECODE] = wide_pipe[DECODE].pip;//COPY to CORE pipeline

    if(_hazard_detect_unit_(&CORE.pipeStageState[EXECUTE], EXECUTE)){
        return 0;
    }
    if(_hazard_detect_unit_(&CORE.pipeStageState[MEMORY], MEMORY)){
        return 0;
    }
    if(!split_regfile &&
       _hazard_detect_unit_(&CORE.pipeStageState[WRITEBACK], WRITEBACK)){
        return 0;
    }
    return 0;
}
/*!This function is to handling the EXECUTE stage in the pipeline*/
static int _EXE_(Buffer *buffer) {
    if (buffer == NULL) {
        return ERROR;
    }
    if (memoryFailiureSignal) {
        return 0;
    }

    Buffer temp = *buffer;
    (*buffer) = wide_pipe[EXECUTE];
    wide_pipe[EXECUTE] = temp;


    if(forwarding){
        if (forwardingWBSignal) {
            assert(!forwardingMEMSignal);
            _forwarding_unit_(&(wide_pipe[MEMORY]));
        }
        if(forwardingMEMSignal){
            assert(!forwardingWBSignal);
            _forwarding_unit_(buffer);
        }
    }

    CORE.pipeStageState[EXECUTE] = wide_pipe[EXECUTE].pip;

    SIM_cmd_opcode cmd_opcode = CORE.pipeStageState[EXECUTE].cmd.opcode;

    //ALU behavior according to cmd opcode
    switch (cmd_opcode) {
        case CMD_NOP:
        case CMD_HALT:
            break;

        case CMD_ADD:
        case CMD_ADDI:
        case CMD_LOAD:
            wide_pipe[EXECUTE].alu_res =
                    CORE.pipeStageState[EXECUTE].src1Val +
                    CORE.pipeStageState[EXECUTE].src2Val;
            break;

        case CMD_SUB:
        case CMD_SUBI:
            wide_pipe[EXECUTE].alu_res =
                    CORE.pipeStageState[EXECUTE].src1Val -
                    CORE.pipeStageState[EXECUTE].src2Val;
            break;

        case CMD_STORE:
            wide_pipe[EXECUTE].alu_res =
                    wide_pipe[EXECUTE].dest + CORE.pipeStageState[EXECUTE].src2Val;
            break;


        case CMD_BRNEQ:
            wide_pipe[EXECUTE].branch_taken =
                    (CORE.pipeStageState[EXECUTE].src1Val != CORE.pipeStageState[EXECUTE].src2Val);
            wide_pipe[EXECUTE].alu_res =
                    wide_pipe[EXECUTE].pc_after_fetch + wide_pipe[EXECUTE].dest;
            break;


        case CMD_BREQ:
            wide_pipe[EXECUTE].branch_taken =
                    (CORE.pipeStageState[EXECUTE].src1Val == CORE.pipeStageState[EXECUTE].src2Val);
            wide_pipe[EXECUTE].alu_res =
                    wide_pipe[EXECUTE].pc_after_fetch + wide_pipe[EXECUTE].dest;
            break;

        case CMD_BR:
            wide_pipe[EXECUTE].branch_taken = true;
            wide_pipe[EXECUTE].alu_res =
                    wide_pipe[EXECUTE].pc_after_fetch + wide_pipe[EXECUTE].dest;
            break;

        default:
            break;

    }
    return 0;
}

/*!This function is to handling the MEMORY stage in the pipeline*/
int _MEM_(Buffer *buffer) {
    if (buffer == NULL) {
        return ERROR;
    }

    if (!memoryFailiureSignal) {
        /*No memory failure fetching from execute*/
        Buffer temp = *buffer;
        (*buffer) = wide_pipe[MEMORY];
        wide_pipe[MEMORY] = temp;
        CORE.pipeStageState[MEMORY] = temp.pip;
    } else {
        /*Memory read failed previous cycle NOT fetching from execute and trying to read again
         * inject nop to WB */
        assert(buffer != NULL);
        _generateNOP_(buffer);
        memoryFailiureSignal = false;
    }

    SIM_cmd_opcode cmd_opcode = CORE.pipeStageState[MEMORY].cmd.opcode;


    switch (cmd_opcode) {
        case CMD_NOP:
        case CMD_HALT:
        case CMD_ADD:
        case CMD_ADDI:
        case CMD_SUB:
        case CMD_SUBI:
            break;

        case CMD_STORE:
            SIM_MemDataWrite((uint32_t) wide_pipe[MEMORY].alu_res,
                             wide_pipe[MEMORY].pip.src1Val);
            break;

        case CMD_LOAD:
            memoryFailiureSignal =
                    (SIM_MemDataRead((uint32_t) wide_pipe[MEMORY].alu_res,
                                     &(wide_pipe[MEMORY].mem_data)) == ERROR);
            break;

        case CMD_BR:
        case CMD_BREQ:
        case CMD_BRNEQ:
            if (wide_pipe[MEMORY].branch_taken == true) {
                branchAddress = (uint32_t) wide_pipe[MEMORY].alu_res;
                branchSignal = true;
                hazard_signal = false;
            }
            break;
        default:
            break;
    }
    return 0;
}

/*!This function is to handling the WRITEBACK stage in the pipeline*/
int _WB_(Buffer *buffer) {
    if(buffer == NULL){
        return ERROR;
    }
    wide_pipe[WRITEBACK] = (*buffer);
    CORE.pipeStageState[WRITEBACK] = wide_pipe[WRITEBACK].pip;

    SIM_cmd_opcode cmd_opcode= CORE.pipeStageState[WRITEBACK].cmd.opcode;


    switch (cmd_opcode) {
        case CMD_NOP:
        case CMD_HALT:
        case CMD_STORE:
        case CMD_BR:
        case CMD_BREQ:
        case CMD_BRNEQ: break;

        case CMD_LOAD:
            writeBackValue = wide_pipe[WRITEBACK].mem_data;
            writeBackRegister = wide_pipe[WRITEBACK].dest;
            writeBackSignal = true;
            break;

        case CMD_ADD:
        case CMD_ADDI:
        case CMD_SUB:
        case CMD_SUBI:
            writeBackValue = wide_pipe[WRITEBACK].alu_res;
            writeBackRegister = wide_pipe[WRITEBACK].dest;
            writeBackSignal = true;
            break;
        default:
            break;
    }
    if(split_regfile && writeBackSignal){
        CORE.regFile[writeBackRegister] = writeBackValue;
        writeBackSignal = false;
        writeBackValue = ERROR;
    }
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
    CORE.pc = NOP;
    /*Reset signals*/

    hazard_signal = false;
    memoryFailiureSignal =false;
    writeBackSignal =false;
    branchSignal = false;
    forwardingMEMSignal = false;
    forwardingWBSignal = false;

    for (int r = 0; r < SIM_REGFILE_SIZE; r++) {
        CORE.regFile[r] = NOP;
    }

    Buffer nop;
    if (_generateNOP_(&nop) == ERROR) {
        return ERROR;
    }

    for (int p = 0; p < SIM_PIPELINE_DEPTH; p++) {
        CORE.pipeStageState[p] = nop.pip;
        wide_pipe[p]=nop;
    }
    SIM_MemInstRead((uint32_t)CORE.pc,&(wide_pipe[FETCH].pip.cmd));
    CORE.pipeStageState[FETCH]=wide_pipe[FETCH].pip;
    return 0;
}

/*! SIM_CoreClkTick: Update the core simulator's state given one clock cycle.
  This function is expected to update the core pipeline given a clock cycle event.
*/
void SIM_CoreClkTick() {

    if(writeBackSignal){//case need to WB when clock rise
        CORE.regFile[writeBackRegister] = writeBackValue;
        writeBackSignal = false;
        writeBackValue = ERROR;
    }

    Buffer buffer;
    _generateNOP_(&buffer);
    if (_IF_(&buffer) == ERROR){
        return;
    }
    //Buffer hold instruction after IF
    if(_ID_(&buffer, true) == ERROR){
        return;
    }
    //Buffer hold instruction after ID
    if(_EXE_(&buffer) == ERROR){
        return;
    }
    //Buffer hold instruction after EXE with ALU Result
    if(_MEM_(&buffer) == ERROR){
        return;
    }
    //Buffer hold instruction after MEM with ALU Result
    if(_WB_(&buffer) == ERROR){
        return;
    }
    _generateNOP_(&buffer);
    //Decoding again after EXE, MEM and WB is updated
    if(_ID_(&buffer, false) == ERROR){
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

    curState->pc = CORE.pc;

    for (int r=0; r < SIM_REGFILE_SIZE ; r++) {
        curState->regFile[r] = CORE.regFile[r];
    }

    for (int p = FETCH; p < SIM_PIPELINE_DEPTH; p++){
        curState->pipeStageState[p] = CORE.pipeStageState[p];
    }
}
