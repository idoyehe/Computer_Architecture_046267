/* 046267 Computer Architecture - Spring 2016 - HW #2 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#define ERROR -1
#define OK 0
#define MAX_BTB 32
#define MAX_HISTORY 256
#define PC_ALIGN 30
#define SHARE_LSB 2
#define SHARE_MID 16

typedef enum{ SNT = 0, WNT = 1, WT = 2, ST = 3} FSM;
typedef enum{NOT_TAKEN = -1, TAKEN = 1}CALL;
typedef enum{NOT_SHARED = 0, LSB = 1, MID = 2}Shared;

typedef struct {
	FSM fsm;
}TwoBitCounter;

int initTwoBitCounter(TwoBitCounter *fsm){
	if(fsm == NULL)
		return ERROR;
	fsm->fsm = WNT;
	return OK;
}

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

CALL getTwoBitCounterResult(TwoBitCounter *FSM){
	if(FSM->fsm > WNT){
		return TAKEN;
	}
	return NOT_TAKEN;
}

typedef struct{
	int bitSize;
	uint8_t history;
	uint8_t mask;
}History;

int initHistory(History *history,int bitSize){
	if(bitSize < 1 || bitSize > 8 || history == NULL){
		return ERROR;
	}
	history->bitSize = bitSize;
	history->history = 0;
	history->mask = 0;
    for(int i = 0; i<bitSize; i++){
        history->mask*=2;
        history->mask++;
    }
	return OK;
}

void updateHistory(History *history,CALL call){
    history->history = history->history << 1;
    if (call == TAKEN) {
        history->history++;
    }
    history->history = history->history & history->mask; //history AND mask
}

void resetHistory(History *history){
    history->history = 0;
}

uint8_t getFSMIndex(History *history){
    return (history->history & history ->mask);
}

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
    btbEntry->target =0;
    for(int i = 0; i < tagBitSize; i++){
        btbEntry->tagMask = btbEntry->tagMask << 1;
        btbEntry->tagMask++;
    }
    return OK;
}

int updateBTBEntry(BTBEntry *btbEntry,uint32_t pc, uint32_t targetPc){
    if(btbEntry == NULL){
        return ERROR;
    }
    pc /= 4;
    btbEntry->tag = pc & btbEntry->tagMask;
    btbEntry->target = targetPc;
    return OK;
}

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
    Shared shared;
}BTBTable;

int initBTBTable(BTBTable *btbTable,unsigned btbSize, unsigned historySize, unsigned tagSize,
            bool isGlobalHist, bool isGlobalTable, Shared shared){
    if(btbTable == NULL || (btbSize != 2 && btbSize != 4 && btbSize != 8 && btbSize != 16 && btbSize != 32 )){
        return ERROR;
    }
    btbTable->isGlobalHist = isGlobalHist;
    btbTable->isGlobalTable =isGlobalTable;
    btbTable->btbSize = btbSize;
    btbTable->shared = shared;
    int logBtbSize=0;
    while(btbSize > 1){
        btbSize = btbSize >> 1;
        logBtbSize++;
    }
    btbTable->indexMask = 0;
    for(int i = 0;i < logBtbSize ; i++){
        btbTable->indexMask = btbTable->indexMask << 1;
        btbTable->indexMask ++;

    }
    for(int i = 0; i < MAX_BTB;i++){
        if(initBTBEntry(btbTable->btbTable + i,tagSize) == ERROR){
            return ERROR;
        }
    }
    for(int i = 0; i < MAX_HISTORY; i++){
        if(initHistory(btbTable->localHistory +i,historySize) == ERROR){
            return ERROR;
        }
        if(initTwoBitCounter(btbTable->globalFSM +i) == ERROR){
            return ERROR;
        }
    }
    if(initHistory(&(btbTable->globalHistory),historySize) == ERROR){
        return ERROR;
    }
    for(int i = 0; i < MAX_BTB; i++){
        for(int j = 0; j < MAX_HISTORY; j++) {
            if (initTwoBitCounter(&(btbTable->localFSM[i][j])) == ERROR) {
                return ERROR;
            }
        }
    }

    return OK;
}

int indexBTBEntryCalc(BTBTable *btbTable,uint32_t pc){
    if(btbTable == NULL){
        return ERROR;
    }
    pc = pc >> 2;//remove 2 LSB
    return (pc & btbTable ->indexMask);//calculating index for btbEntry
}

int indexTwoBitCounter(BTBTable *btbTable,uint32_t pc) {
    if (btbTable == NULL) {
        return ERROR;
    }
    History *currHistory = &(btbTable->globalHistory);
    if (btbTable->localHistory) {
        int indexHistory = indexBTBEntryCalc(btbTable, pc);
        currHistory = btbTable->localHistory + indexHistory;
    }
    uint8_t rawIndexTwoBitCounter = getFSMIndex(currHistory);
    if (btbTable->isGlobalHist || btbTable->isGlobalTable) {
        uint8_t calcadPC = 0;
        switch (btbTable->shared) {
            case NOT_SHARED:
                return rawIndexTwoBitCounter;
            case LSB: {
                calcadPC = (uint8_t) (pc >> SHARE_LSB);
                break;
            }
            case MID: {
                calcadPC = (uint8_t) (pc >> SHARE_MID);
                break;
            }
        }
        rawIndexTwoBitCounter =  calcadPC ^ rawIndexTwoBitCounter;
        return rawIndexTwoBitCounter & currHistory ->mask;
    }
    return rawIndexTwoBitCounter;
}

BTBTable globalBTBTable;
SIM_stats globalState;


int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize,
             bool isGlobalHist, bool isGlobalTable, int Shared){
    globalState.size = 0;
    globalState.size += btbSize * (tagSize + PC_ALIGN);
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

    return initBTBTable(&globalBTBTable,btbSize,historySize,tagSize,isGlobalHist,isGlobalTable,Shared);
}

bool BP_predict(uint32_t pc, uint32_t *dst){
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	return;
}

void BP_GetStats(SIM_stats *curStats) {
    if(curStats == NULL){
        return;
    }
    curStats->size = globalState.size;
    curStats->br_num = globalState.br_num;
    curStats->flush_num = globalState.flush_num;
}
