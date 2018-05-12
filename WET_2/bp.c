/* 046267 Computer Architecture - Spring 2016 - HW #2 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <stdio.h>

#define ERROR (-1)
#define OK 0
#define MAX_BTB 32
#define MAX_HISTORY 256
#define PC_ALIGN 30
#define SHARE_LSB 2
#define SHARE_MID 16

typedef enum{ SNT = 0, WNT = 1, WT = 2, ST = 3} FSM;
typedef enum{NOT_TAKEN = -1, TAKEN = 1}CALL;
typedef enum{NOT_SHARED = 0, LSB = 1, MID = 2}Shared;

/*wrapper for the FSM*/
typedef struct {
	FSM fsm;
}TwoBitCounter;


int initTwoBitCounter(TwoBitCounter *fsm){
	if(fsm == NULL)
		return ERROR;
	fsm->fsm = WNT;
	return OK;
}
/*Handle the FSM behavior*/
int updateTwoBitCounter(TwoBitCounter *FSM,CALL call){
	if(FSM == NULL)
		return ERROR;
	int state = FSM->fsm;
	state+=call;
	if(state < SNT){
		state = SNT;
	}
	if(state > ST){
		state = ST;
	}
	FSM->fsm = state;
	return OK;
}
/*Transform FSM state to predictor call*/
CALL getTwoBitCounterResult(TwoBitCounter *FSM){
	if(FSM->fsm > WNT){
		return TAKEN;
	}
	return NOT_TAKEN;
}
/*history wrapper*/
typedef struct{
    unsigned bitSize;
	uint8_t history;
	uint8_t mask;
}History;

int initHistory(History *history, unsigned bitSize){
	if(bitSize < 1 || bitSize > 8 || history == NULL){
		return ERROR;
	}
	history->bitSize = bitSize;
	history->history = 0;
	history->mask = 0;
	//generating history mask to get index for FSM
    for(int i = 0; i<bitSize; i++){
        history->mask = history->mask << 1;
        history->mask++;
    }
	return OK;
}
/*updating history with given call*/
void updateHistory(History *history,CALL call){
    history->history = history->history << 1;
    if (call == TAKEN) {
        history->history++;
    }
    history->history = history->history & history->mask; //history AND mask
}

/*reset history to 0*/
void resetHistory(History *history){
    history->history = 0;
}

/*return from the history the FSM index*/
uint8_t getFSMIndex(History *history){
    return (history->history & history ->mask);
}

/*BTB entry wrapper*/
typedef struct{
    uint32_t tag;
    uint32_t tagMask;
    uint32_t target;
}BTBEntry;

int initBTBEntry(BTBEntry *btbEntry,int tagBitSize){
    if(tagBitSize < 0 || tagBitSize > 30 || btbEntry == NULL){
        return ERROR;
    }
    btbEntry->tag = 0;
    btbEntry->target = 0;
    btbEntry->tagMask = 0;
    //generating mask for saving tag
    for(int i = 0; i < tagBitSize; i++){
        btbEntry->tagMask = btbEntry->tagMask << 1;
        btbEntry->tagMask++;
    }
    return OK;
}
/*updating btb with given parameters*/
int updateBTBEntry(BTBEntry *btbEntry,uint32_t pc, uint32_t targetPc){
    if(btbEntry == NULL){
        return ERROR;
    }
    pc = pc >> 2;
    btbEntry->tag = pc & btbEntry->tagMask;
    btbEntry->target = targetPc;
    return OK;
}
/*Branch Predictor wrapper*/
typedef struct{
    BTBEntry btbTable[MAX_BTB];
    uint32_t indexMask;
    History localHistory[MAX_BTB];
    History globalHistory;
    TwoBitCounter localFSM[MAX_BTB][MAX_HISTORY];
    TwoBitCounter globalFSM[MAX_HISTORY];
    bool isGlobalHist;
    bool isGlobalTable;
    unsigned btbSize;
    unsigned tagSize;
    unsigned historySize;
    Shared shared;
}BranchPredictor;

int initBranchPredictor(BranchPredictor *branchPredictor, unsigned btbSize, unsigned historySize, unsigned tagSize,
                        bool isGlobalHist, bool isGlobalTable, Shared shared){
    if(branchPredictor == NULL || (btbSize != 1 && btbSize != 2 && btbSize != 4 && btbSize != 8 && btbSize != 16 && btbSize != 32 )){
        return ERROR;
    }
    branchPredictor->isGlobalHist = isGlobalHist;
    branchPredictor->isGlobalTable =isGlobalTable;
    branchPredictor->btbSize = btbSize;
    branchPredictor->tagSize = tagSize;
    branchPredictor->shared = shared;
    branchPredictor->historySize = historySize;
    int logBtbSize=0;
    while(btbSize > 1){
        btbSize = btbSize >> 1;
        logBtbSize++;
    }
    branchPredictor->indexMask = 0;
    for(int i = 0;i < logBtbSize ; i++){
        branchPredictor->indexMask = branchPredictor->indexMask << 1;
        branchPredictor->indexMask ++;

    }
    for(int i = 0; i < MAX_BTB;i++){
        if(initBTBEntry(branchPredictor->btbTable + i,tagSize) == ERROR){
            return ERROR;
        }
    }
    for(int i = 0; i < MAX_HISTORY; i++){
        if(initHistory(branchPredictor->localHistory +i,historySize) == ERROR){
            return ERROR;
        }
        if(initTwoBitCounter(branchPredictor->globalFSM +i) == ERROR){
            return ERROR;
        }
    }
    if(initHistory(&(branchPredictor->globalHistory),historySize) == ERROR){
        return ERROR;
    }
    for(int i = 0; i < MAX_BTB; i++){
        for(int j = 0; j < MAX_HISTORY; j++) {
            if (initTwoBitCounter(&(branchPredictor->localFSM[i][j])) == ERROR) {
                return ERROR;
            }
        }
    }

    return OK;
}
/*given pc return the index in the btb table*/
int getIndexBTBEntry(BranchPredictor *btbTable, uint32_t pc){
    if(btbTable == NULL){
        return ERROR;
    }
    pc = pc >> 2;//remove 2 LSB
    return (pc & btbTable ->indexMask);//calculating index for btbEntry
}

/*given pc return the index of the FSM basing on it's history*/
int getIndexTwoBitCounter(BranchPredictor *btbTable,uint32_t pc) {
    if (btbTable == NULL) {
        return ERROR;
    }
    History *currHistory = &(btbTable->globalHistory);
    if (!(btbTable->isGlobalHist)) {
        int indexHistory = getIndexBTBEntry(btbTable, pc);
        currHistory = btbTable->localHistory + indexHistory;
    }
    uint8_t rawIndexTwoBitCounter = getFSMIndex(currHistory);
    uint8_t calcadPC = 0;
    switch (btbTable->shared) {
        case NOT_SHARED:
            return rawIndexTwoBitCounter;
        case LSB: {//for share LSB
            calcadPC = (uint8_t) (pc >> SHARE_LSB);
            rawIndexTwoBitCounter = calcadPC ^ rawIndexTwoBitCounter;
            break;
        }
        case MID: {//for share MID
            calcadPC = (uint8_t) (pc >> SHARE_MID);
            rawIndexTwoBitCounter = calcadPC ^ rawIndexTwoBitCounter;
            break;
        }
    }
    return rawIndexTwoBitCounter & currHistory->mask;
}

BranchPredictor globalBranchPred;
SIM_stats globalState;


int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize,
             bool isGlobalHist, bool isGlobalTable, int Shared){
    /*size calculating*/
    globalState.size = btbSize * (tagSize + PC_ALIGN);
    if (isGlobalHist == true){
        globalState.size += historySize;
    }
    else
        globalState.size +=btbSize * historySize;
    int tableSize = 2;
    for(int i = 0; i < historySize ;i++){
        tableSize *=2;
    }

    if(isGlobalTable){
        globalState.size +=  tableSize;
    }
    else
        globalState.size +=  btbSize * tableSize;

    return initBranchPredictor(&globalBranchPred, btbSize, historySize, tagSize,
                               isGlobalHist, isGlobalTable, Shared);
}

bool BP_predict(uint32_t pc, uint32_t *dst){
    int btbIndex = getIndexBTBEntry(&globalBranchPred,pc);
    BTBEntry tempEntry;
    initBTBEntry(&tempEntry,globalBranchPred.tagSize);
    updateBTBEntry(&tempEntry,pc,0);

    if(globalBranchPred.btbTable[btbIndex].tag != tempEntry.tag){//unknown PC
        (*dst) = pc +4;
        return false;
    }

    int fsmIndex = getIndexTwoBitCounter(&globalBranchPred,pc);//get FSM index
    CALL branchCall;
    if(globalBranchPred.isGlobalTable){
        branchCall = getTwoBitCounterResult(globalBranchPred.globalFSM+fsmIndex);
    }
    else{
        branchCall = getTwoBitCounterResult(&(globalBranchPred.localFSM[btbIndex][fsmIndex]));
    }
    if(branchCall == TAKEN){
        (*dst) = globalBranchPred.btbTable[btbIndex].target;
        return true;
    }
    (*dst) = pc + 4;
    return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
    globalState.br_num++;
    if((pred_dst != (pc + 4)  && !taken )|| (pred_dst != targetPc && taken)){//need flash
        globalState.flush_num++;
    }

    int btbIndex = getIndexBTBEntry(&globalBranchPred,pc);
    BTBEntry tempEntry;
    initBTBEntry(&tempEntry,globalBranchPred.tagSize);
    updateBTBEntry(&tempEntry,pc,targetPc);

    if(globalBranchPred.btbTable[btbIndex].tag != tempEntry.tag){
        //clean the local history and local FSM table for new Branch ins.
        initHistory(globalBranchPred.localHistory + btbIndex,globalBranchPred.historySize);
        for(int i = 0; i < MAX_HISTORY; i++) {
            initTwoBitCounter(&(globalBranchPred.localFSM[btbIndex][i]));
        }
    }
    globalBranchPred.btbTable[btbIndex] = tempEntry;
    CALL actualCall = taken ? TAKEN : NOT_TAKEN;

    int tableIndex = getIndexTwoBitCounter(&globalBranchPred,pc);
    //first updating FSM
    if(globalBranchPred.isGlobalTable){
        updateTwoBitCounter(globalBranchPred.globalFSM + tableIndex,actualCall);
    }

    else{
        updateTwoBitCounter(&(globalBranchPred.localFSM[btbIndex][tableIndex]),actualCall);
    }
    //second updating history
    if(globalBranchPred.isGlobalHist){
        updateHistory(&globalBranchPred.globalHistory,actualCall);
    }
    else{
        updateHistory(globalBranchPred.localHistory + btbIndex,actualCall);
    }
}

void BP_GetStats(SIM_stats *curStats) {
    //exporting state
    if(curStats == NULL){
        return;
    }
    curStats->size = globalState.size;
    curStats->br_num = globalState.br_num;
    curStats->flush_num = globalState.flush_num;
}
