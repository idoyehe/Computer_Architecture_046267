/* 046267 Computer Architecture - Spring 2017 - HW #1 */
/* This file should hold your implementation of the CPU pipeline core simulator */

#include "sim_api.h"
/////////////////////////////////////////////////
//Helper struct for passing values down the pipe
////////////////////////////////////////////////
typedef struct {
	int32_t PC_after_fetch;
	int32_t helper;
} CMD_vals;

////////////////////////////////
//Global Core Struct & variables
////////////////////////////////
SIM_coreState core;
CMD_vals pipe_vals[SIM_PIPELINE_DEPTH];
int DECODE_res;
int MEM_res;
int32_t temp_pc;
bool did_branch;

/////////////////////////////////
//Internal functions Declaration
/////////////////////////////////
void MakeNOP(PipeStageState* cmd);

bool Hazard(int regnum,int stages, pipeStage start);
void ClkTick_ForwardUnit(){}
void ClkTick_SplitUnit(){}
void ClkTick_NoUnit();

void NoUnit_IF();
int NoUnit_ID();
void NoUnit_EXE();
int NoUnit_MEM();
void NoUnit_WB();

//void Forward_IF();
//int Forward_ID();
//void Forward_EXE();
//int Forward_MEM();
//void Forward_WB();
//
//void Split_IF();
//int Split_ID();
//void Split_EXE();
//int Split_MEM();
//void Split_WB();

////////////////////////////
//Internal implementations
///////////////////////////
//Checks if there is a (Non-control) Hazard from a given stage, down the pipe
bool Hazard(int regnum,int stages, pipeStage start){
	int i;
	SIM_cmd_opcode opcode;
	for(i=1 ; i <= stages ; i++){
		opcode=core.pipeStageState[start+i].cmd.opcode;
		if(opcode == CMD_ADD || opcode == CMD_SUB || opcode == CMD_LOAD){
			if(core.pipeStageState[start+i].cmd.dst == regnum){
				return true;
			}
		}
	}
	return false;
}

//Creates a NOP command
void MakeNOP(PipeStageState* cmd){
	if(!cmd) return;
	SIM_cmd NOPcmd;
	NOPcmd.opcode=CMD_NOP;
	NOPcmd.dst=0;
	NOPcmd.isSrc2Imm=0;
	NOPcmd.src1=0;
	NOPcmd.src2=0;
	cmd->cmd=NOPcmd;
	cmd->src1Val=0;
	cmd->src2Val=0;
}


void ClkTick_NoUnit(){
	core.pc=temp_pc;
	//Updating regfile
	NoUnit_WB();
	PipeStageState NOP;
	MakeNOP(&NOP);

	//if last clock's MEM failed, STALL
	if(MEM_res != 0){
		core.pipeStageState[WRITEBACK]=NOP;
		MEM_res=NoUnit_MEM();
		return;
	}
	//Shifting inst. in MEM to WB
	core.pipeStageState[WRITEBACK]=core.pipeStageState[MEMORY];
	pipe_vals[WRITEBACK]=pipe_vals[MEMORY];

	if(did_branch){
		core.pipeStageState[FETCH]=NOP;
		core.pipeStageState[DECODE]=NOP;
		core.pipeStageState[EXECUTE]=NOP;
		did_branch=false;
	}
	//Shifting inst. in EXE to MEM
	core.pipeStageState[MEMORY]=core.pipeStageState[EXECUTE];
	pipe_vals[MEMORY]=pipe_vals[EXECUTE];

	if(DECODE_res != 0){
		core.pipeStageState[EXECUTE]=NOP;
		MEM_res=NoUnit_MEM();
		NoUnit_EXE(); //Will do nothing (NOP in EXECUTE)
		DECODE_res=NoUnit_ID();
		return;
	}
	//Shifting inst. in DEC to EXE
	core.pipeStageState[EXECUTE]=core.pipeStageState[DECODE];
	pipe_vals[EXECUTE]=pipe_vals[DECODE];
	//Shifting inst. in IF to DEC
	core.pipeStageState[DECODE]=core.pipeStageState[FETCH];
	pipe_vals[DECODE]=pipe_vals[FETCH];

	MEM_res=NoUnit_MEM();
	NoUnit_EXE();
	DECODE_res=NoUnit_ID();
	NoUnit_IF();
}

void NoUnit_WB(){
	SIM_cmd_opcode opcode=core.pipeStageState[WRITEBACK].cmd.opcode;
	if(opcode < CMD_ADD || opcode > CMD_LOAD ){
		return;
	}
	core.regFile[core.pipeStageState[WRITEBACK].cmd.dst]=
			pipe_vals[WRITEBACK].helper;
}
int NoUnit_MEM(){
	SIM_cmd_opcode opcode=core.pipeStageState[MEMORY].cmd.opcode;
	if(opcode < CMD_LOAD || opcode > CMD_BRNEQ ){
		return 0;
	}
	if(opcode >= CMD_BR && opcode <= CMD_BRNEQ){
		if(opcode == CMD_BR || pipe_vals[MEMORY].helper == 1){
			did_branch=true;
			temp_pc=pipe_vals[MEMORY].PC_after_fetch;
		}
	}
	if(opcode == CMD_LOAD){
		return SIM_MemDataRead(pipe_vals[MEMORY].helper,
				&pipe_vals[MEMORY].helper);
	}
	if(opcode == CMD_STORE){
		SIM_MemDataWrite(pipe_vals[MEMORY].helper,
				core.pipeStageState[MEMORY].src1Val);
	}
	return 0;
}

void NoUnit_EXE(){
	SIM_cmd_opcode opcode=core.pipeStageState[MEMORY].cmd.opcode;
	if(opcode == CMD_NOP ||  opcode == CMD_HALT){
		return;
	}

	if(opcode >= CMD_BR && opcode <= CMD_BRNEQ){
		pipe_vals[EXECUTE].PC_after_fetch+=pipe_vals[EXECUTE].helper;
		if(opcode == CMD_BREQ){
			pipe_vals[EXECUTE].helper=core.pipeStageState[EXECUTE].src1Val-
					core.pipeStageState[EXECUTE].src2Val == 0;
		}
		if(opcode == CMD_BRNEQ){
			pipe_vals[EXECUTE].helper=core.pipeStageState[EXECUTE].src1Val-
					core.pipeStageState[EXECUTE].src2Val != 0;
		}
	}

	if(opcode == CMD_STORE){
		pipe_vals[EXECUTE].helper+=core.pipeStageState[EXECUTE].src2Val;
	}
	if(opcode == CMD_LOAD){
		pipe_vals[EXECUTE].helper=core.pipeStageState[EXECUTE].src1Val+
				core.pipeStageState[EXECUTE].src2Val;
	}
	if(opcode == CMD_ADD){
		pipe_vals[EXECUTE].helper=core.pipeStageState[EXECUTE].src1Val+
						core.pipeStageState[EXECUTE].src2Val;
	}
	if(opcode == CMD_SUB){
		pipe_vals[EXECUTE].helper=core.pipeStageState[EXECUTE].src1Val-
						core.pipeStageState[EXECUTE].src2Val;
	}
}

int NoUnit_ID(){
	SIM_cmd_opcode opcode=core.pipeStageState[MEMORY].cmd.opcode;
	int hazard1=0;
	int hazard2=0;
	int hazarddst=0;
	if(opcode == CMD_NOP ||  opcode == CMD_HALT){
		return 0;
	}
	int dst=core.pipeStageState[DECODE].cmd.dst;
	int src1=core.pipeStageState[DECODE].cmd.src1;
	int32_t src2=core.pipeStageState[DECODE].cmd.src2;

	if(opcode != CMD_BR){
		core.pipeStageState[DECODE].src1Val=core.regFile[src1];
		hazard1=Hazard(src1,3,DECODE);
		if(core.pipeStageState[DECODE].cmd.isSrc2Imm){
			core.pipeStageState[DECODE].src2Val=src2;
		}
		else{
			core.pipeStageState[DECODE].src2Val=core.regFile[src2];
			hazard2=Hazard(src2,3,DECODE);
		}
	}
	if(opcode == CMD_STORE || opcode == CMD_BREQ || opcode == CMD_BRNEQ){
		pipe_vals[DECODE].helper=core.regFile[dst];
		hazarddst=Hazard(dst,3,DECODE);
	}
	return did_branch? 0 : hazard1 || hazard2 || hazarddst;
}

void NoUnit_IF(){
	PipeStageState command;
	command.src1Val=0;
	command.src2Val=0;
	SIM_MemInstRead(core.pc,&(command.cmd));
	if(!did_branch){
		temp_pc=core.pc+4;
	}
	pipe_vals[FETCH].helper=0;
	pipe_vals[FETCH].PC_after_fetch=temp_pc;
	core.pipeStageState[FETCH] = command;
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
	int i;
	core.pc=0;
	for (i=0;i<SIM_REGFILE_SIZE;i++){
		core.regFile[i]=0;
	}
	NoUnit_IF();
	PipeStageState NOPstate;
	MakeNOP(&NOPstate);
	for (i=1;i<SIM_PIPELINE_DEPTH;i++){
		core.pipeStageState[i] = NOPstate;
	}
	MEM_res=0;
	DECODE_res=0;
	did_branch=false;
	return 0;
}

/*! SIM_CoreClkTick: Update the core simulator's state given one clock cycle.
  This function is expected to update the core pipeline given a clock cycle event.
*/
void SIM_CoreClkTick() {
	if(forwarding){
		ClkTick_ForwardUnit();
	}
	else if (split_regfile){
		ClkTick_SplitUnit();
	}
	else {
		ClkTick_NoUnit();
	}
}

/*! SIM_CoreGetState: Return the current core (pipeline) internal state
    curState: The returned current pipeline state
    The function will return the state of the pipe at the end of a cycle
*/
void SIM_CoreGetState(SIM_coreState *curState) {
	int i;
	curState->pc=core.pc;
	for (i=0 ; i < SIM_REGFILE_SIZE ; i++){
		curState->regFile[i]=core.regFile[i];
	}
	for (i=0 ; i < SIM_PIPELINE_DEPTH ; i++){
		curState->pipeStageState[i]=core.pipeStageState[i];
	}

}

