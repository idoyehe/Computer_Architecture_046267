/* 046267 Computer Architecture - Spring 2017 - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include <assert.h>
#include "dflow_calc.h"
#define ERROR -1
#define OK 0
#define NO_DEPEND -1

typedef struct{
    InstInfo instruction;
    int src1_index_depend, src2_index_depend;
    unsigned int instruction_latency;
    int instruction_depth;
}InstructionWrapper;

typedef struct{
    InstructionWrapper* inst_array;
    int program_depth;
    unsigned int total_instruction;
}program_wrapper;

typedef program_wrapper* ProgramInfo;

static int setAllInstructionLatency(ProgramInfo programInfo, const unsigned int opsLatency[]){
    if(programInfo == NULL || opsLatency == NULL){
        return ERROR;
    }
    for(int i=0; i < programInfo->total_instruction;i++){
        programInfo->inst_array[i].instruction_latency =
                opsLatency[programInfo->inst_array[i].instruction.opcode];
    }
    return OK;
}

static int initInstructionArray(ProgramInfo programInfo, const unsigned int opsLatency[],InstInfo progTrace[], unsigned int numOfInsts){
    if(programInfo == NULL || opsLatency == NULL || progTrace == NULL){
        return ERROR;
    }
    programInfo->total_instruction = numOfInsts;
    for(int i = 0; i < numOfInsts; i++){
        programInfo->inst_array[i].instruction = progTrace[i];
        programInfo->inst_array[i].src1_index_depend = NO_DEPEND;
        programInfo->inst_array[i].src2_index_depend = NO_DEPEND;
    }
    return setAllInstructionLatency(programInfo,opsLatency);
}

static int setInstructionDependecies(ProgramInfo programInfo){
  if(programInfo == NULL){
      return ERROR;
  }
    for(int i = programInfo->total_instruction - 1; i > -1; i--){//src1 & src2 dependencies update
        for(int j = i-1; j > -1 ; j--){
            if(programInfo->inst_array[i].src1_index_depend != NO_DEPEND &&
               programInfo->inst_array[i].src2_index_depend != NO_DEPEND){
                break;
            }
            if(programInfo->inst_array[i].src1_index_depend == NO_DEPEND &&
               programInfo->inst_array[i].instruction.src1Idx == programInfo->inst_array[j].instruction.dstIdx){
                programInfo->inst_array[i].src1_index_depend = j;
            }
            if(programInfo->inst_array[i].src2_index_depend == NO_DEPEND &&
               programInfo->inst_array[i].instruction.src2Idx == programInfo->inst_array[j].instruction.dstIdx){
                programInfo->inst_array[i].src2_index_depend = j;
            }
        }
    }
    return OK;
}

static int depthInstructionCalc(ProgramInfo programInfo,int index){
    if(programInfo == NULL || index < 0 || index > programInfo->total_instruction -1){
        return ERROR;
    }
    if(programInfo->inst_array[index].src1_index_depend == NO_DEPEND
       && programInfo->inst_array[index].src2_index_depend == NO_DEPEND){
        programInfo ->inst_array[index].instruction_depth = 0;
        return OK;
    }
    int depth_src1 = 0, depth_src2 = 0;
    if(programInfo->inst_array[index].src1_index_depend != NO_DEPEND){
        int index_depend = programInfo->inst_array[index].src1_index_depend;
        depth_src1 = programInfo->inst_array[index_depend].instruction_depth +
                programInfo->inst_array[index_depend].instruction_latency;
    }
    if(programInfo->inst_array[index].src2_index_depend != NO_DEPEND){
        int index_depend = programInfo->inst_array[index].src2_index_depend;
        depth_src2 = programInfo->inst_array[index_depend].instruction_depth +
                     programInfo->inst_array[index_depend].instruction_latency;
    }
    programInfo ->inst_array[index].instruction_depth =
            (depth_src1 >= depth_src2) ? depth_src1 : depth_src2;
    return OK;
}

static int depthAllInstructionsCalc(ProgramInfo programInfo){
    if(programInfo == NULL){
        return ERROR;
    }
    for(int i = 0; i < programInfo->total_instruction; i++){
        if(depthInstructionCalc(programInfo,i) == ERROR)
            return ERROR;
    }
    return OK;
}

static int depthProgramCalc(ProgramInfo programInfo) {
    if (programInfo == NULL) {
        return ERROR;
    }
    programInfo->program_depth = 0;
    for (int i = 0; i < programInfo->total_instruction; i++) {
        if (programInfo->inst_array[i].instruction_depth + programInfo->inst_array[i].instruction_latency >
            programInfo->program_depth) {
            programInfo->program_depth = programInfo->inst_array[i].instruction_depth + programInfo->inst_array[i].instruction_latency;
        }
    }
    return OK;
}

ProgCtx analyzeProg(const unsigned int opsLatency[],  InstInfo progTrace[], unsigned int numOfInsts) {
    ProgramInfo programInfo = (ProgramInfo)malloc(sizeof(*programInfo));
    if(programInfo == NULL){
        return PROG_CTX_NULL;
    }
    programInfo->inst_array = (InstructionWrapper*)malloc(sizeof(InstructionWrapper)  * numOfInsts);
    if(programInfo->inst_array == NULL){
        free(programInfo);
        return PROG_CTX_NULL;
    }
    programInfo->total_instruction = numOfInsts;
    int res = 0;
    res = initInstructionArray(programInfo,opsLatency, progTrace, numOfInsts);
    assert(res == 0);
    res = setInstructionDependecies(programInfo);
    assert(res == 0);

    res = depthAllInstructionsCalc(programInfo);
    assert(res == 0);

    res = depthProgramCalc(programInfo);
    assert(res == 0);

    return programInfo;
}

void freeProgCtx(ProgCtx ctx) {
    ProgramInfo programInfo = (ProgramInfo)ctx;
    free(programInfo->inst_array);
    free(programInfo);
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    ProgramInfo programInfo = (ProgramInfo)ctx;
    if(programInfo == NULL || theInst < 0 || theInst >= programInfo->total_instruction) {
        return ERROR;
    }
    return programInfo->inst_array[theInst].instruction_depth;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    ProgramInfo programInfo = (ProgramInfo) ctx;
    if (programInfo == NULL || src1DepInst == NULL || src2DepInst == NULL ||
        theInst < 0 || theInst >= programInfo->total_instruction) {
        return ERROR;
    }
    *src1DepInst = programInfo->inst_array[theInst].src1_index_depend;
    *src2DepInst = programInfo->inst_array[theInst].src2_index_depend;
    return OK;
}

int getProgDepth(ProgCtx ctx) {
    ProgramInfo programInfo = (ProgramInfo)ctx;
    return programInfo->program_depth;
}
