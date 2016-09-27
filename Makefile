GXX = g++
FLAGS = -g -O3 -std=c++0x 
LIB_PATHS = ./lib
TARGET_PATHS = .
HEAD_PATHS = ./ctpapi
TARGETS = main
SRC_PATHS = ./src
LIBS = -L$(LIB_PATHS)/ -lthosttraderapi -lthostmduserapi 
HEADERS = -I$(HEAD_PATHS)

all : $(TARGETS)

main: $(SRC_PATHS)/main.o
	$(GXX) $(FLAGS) $(HEADERS) $(LIBS) $^ -o $(TARGET_PATHS)/main

$(SRC_PATHS)/main.o: $(SRC_PATHS)/main.cpp $(SRC_PATHS)/CTPTraderSpi.h $(SRC_PATHS)/Comment.h  $(SRC_PATHS)/TradeManager.h
	$(GXX) $(FLAGS) $(HEADERS) $(LIBS) -c $(SRC_PATHS)/main.cpp -o $@


.PHONY:clean
clean:
	-rm $(SRC_PATHS)/*.o $(TARGET_PATHS)/main

	
	
	
  