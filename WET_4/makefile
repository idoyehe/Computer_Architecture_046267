CC = g++
PROG_OBJS = tagEntry.o cacheLevel.o TwoLevelCache.o cacheSim.o
PROG = cacheSim

			 
DEBUG_FLAG =  -g
COMP_FLAG = -std=c++11 -Wall -DNDEBUG

##################### main: Two Level Cache simulator
$(PROG) : $(PROG_OBJS)
	$(CC) $(DEBUG_FLAG) $(PROG_OBJS) -o $@
	
	
##################### Two Level Cache objects	
	
cacheSim.o: cacheSim.cpp cacheLevel.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.cpp
			
TwoLevelCache.o: TwoLevelCache.cpp TwoLevelCache.h cacheLevel.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.cpp
	
cacheLevel.o: cacheLevel.cpp cacheLevel.h tagEntry.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.cpp
	
tagEntry.o: tagEntry.cpp tagEntry.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.cpp
	

clean:
	rm -f $(PROG) $(PROG_OBJS)
	