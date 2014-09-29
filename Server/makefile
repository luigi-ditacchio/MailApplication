CC = gcc
INCLUDE = ./include/
BIN = ./bin/
SRC = ./src/
DATA = ./data/
MAILBOX = ./data/mailbox/

EXEC = server
OBJECTS = $(BIN)main.o $(BIN)thread.o $(BIN)signalHandling.o

EXEC_DEBUG = server_debug
OBJECTS_DEBUG = $(BIN)main_debug.o    $(BIN)IOThread_debug.o    $(BIN)workerThread_debug.o    $(BIN)signalHandling_debug.o    $(BIN)account_debug.o\
                $(BIN)interThreadsCommunication_debug.o    $(BIN)loadAdaptation_debug.o    $(BIN)mailbox_debug.o    $(BIN)timeout_debug.o


$(EXEC): $(OBJECTS)
	$(CC)  $(OBJECTS) -o $(EXEC) -lpthread

$(BIN)main.o: $(SRC)main.c   $(INCLUDE)main.h   $(INCLUDE)workerThread.h  $(INCLUDE)IOThread.h  $(INCLUDE)interThreadsCommunication.h\
                                  $(INCLUDE)clientState.h   $(INCLUDE)timeout.h   $(INCLUDE)signalHandling.h   $(INCLUDE)protocol.h
	$(CC) -c $(SRC)main.c -o $(BIN)main.o -I $(INCLUDE) -DDEBUG

$(BIN)IOThread.o: $(SRC)IOThread.c  $(INCLUDE)IOThread.h  $(INCLUDE)interThreadsCommunication.h  $(INCLUDE)protocol.h\
                                          $(INCLUDE)timeout.h   $(INCLUDE)clientState.h  $(INCLUDE)main.h
	$(CC) -c $(SRC)IOThread.c -o $(BIN)IOThread.o -I $(INCLUDE) -DDEBUG

$(BIN)workerThread.o: $(SRC)workerThread.c  $(INCLUDE)workerThread.h  $(INCLUDE)interThreadsCommunication.h\
                                                  $(INCLUDE)protocol.h   $(INCLUDE)clientState.h   $(INCLUDE)timeout.h   $(INCLUDE)mailbox.h   $(INCLUDE)account.h
	$(CC) -c $(SRC)workerThread.c -o $(BIN)workerThread.o -I $(INCLUDE) -DDEBUG

$(BIN)signalHandling.o: $(SRC)signalHandling.c  $(INCLUDE)signalHandling.h  $(INCLUDE)timeout.h   $(INCLUDE)workerThread.h  $(INCLUDE)IOThread.h   $(INCLUDE)account.h\
                                                      $(INCLUDE)protocol.h  $(INCLUDE)interThreadsCommunication.h    $(INCLUDE)loadAdaptation.h   $(INCLUDE)clientState.h   $(INCLUDE)mailbox.h
	$(CC) -c $(SRC)signalHandling.c -o $(BIN)signalHandling.o -I $(INCLUDE) -DDEBUG

$(BIN)account.o: $(SRC)account.c  $(INCLUDE)account.h  $(INCLUDE)workerThread.h    $(INCLUDE)protocol.h
	$(CC) -c $(SRC)account.c -o $(BIN)account.o -I $(INCLUDE) -DDEBUG
	
$(BIN)interThreadsCommunication.o: $(SRC)interThreadsCommunication.c  $(INCLUDE)interThreadsCommunication.h   $(INCLUDE)clientState.h  $(INCLUDE)timeout.h  $(INCLUDE)protocol.h
	$(CC) -c $(SRC)interThreadsCommunication.c -o $(BIN)interThreadsCommunication.o -I $(INCLUDE) -DDEBUG
	
$(BIN)loadAdaptation.o: $(SRC)loadAdaptation.c  $(INCLUDE)loadAdaptation.h  $(INCLUDE)account.h   $(INCLUDE)interThreadsCommunication.h  $(INCLUDE)IOThread.h\
                                                      $(INCLUDE)mailbox.h  $(INCLUDE)main.h    $(INCLUDE)workerThread.h   $(INCLUDE)clientState.h   $(INCLUDE)protocol.h  $(INCLUDE)timeout.h
	$(CC) -c $(SRC)loadAdaptation.c -o $(BIN)loadAdaptation.o -I $(INCLUDE) -DDEBUG
	
$(BIN)mailbox.o: $(SRC)mailbox.c  $(INCLUDE)mailbox.h  $(INCLUDE)workerThread.h   $(INCLUDE)protocol.h
	$(CC) -c $(SRC)mailbox.c -o $(BIN)mailbox.o -I $(INCLUDE) -DDEBUG
	
$(BIN)timeout.o: $(SRC)timeout.c  $(INCLUDE)timeout.h  $(INCLUDE)IOThread.h   $(INCLUDE)clientState.h
	$(CC) -c $(SRC)timeout.c -o $(BIN)timeout.o -I $(INCLUDE) -DDEBUG






#ESEGUIBILE DEBUG

$(EXEC_DEBUG): $(OBJECTS_DEBUG)
	$(CC)  $(OBJECTS_DEBUG) -o $(EXEC_DEBUG) -lpthread

$(BIN)main_debug.o: $(SRC)main.c   $(INCLUDE)main.h   $(INCLUDE)workerThread.h  $(INCLUDE)IOThread.h  $(INCLUDE)interThreadsCommunication.h\
                                  $(INCLUDE)clientState.h   $(INCLUDE)timeout.h   $(INCLUDE)signalHandling.h   $(INCLUDE)protocol.h
	$(CC) -c $(SRC)main.c -o $(BIN)main_debug.o -I $(INCLUDE) -DDEBUG

$(BIN)IOThread_debug.o: $(SRC)IOThread.c  $(INCLUDE)IOThread.h  $(INCLUDE)interThreadsCommunication.h  $(INCLUDE)protocol.h\
                                          $(INCLUDE)timeout.h   $(INCLUDE)clientState.h  $(INCLUDE)main.h
	$(CC) -c $(SRC)IOThread.c -o $(BIN)IOThread_debug.o -I $(INCLUDE) -DDEBUG

$(BIN)workerThread_debug.o: $(SRC)workerThread.c  $(INCLUDE)workerThread.h  $(INCLUDE)interThreadsCommunication.h\
                                                  $(INCLUDE)protocol.h   $(INCLUDE)clientState.h   $(INCLUDE)timeout.h   $(INCLUDE)mailbox.h   $(INCLUDE)account.h
	$(CC) -c $(SRC)workerThread.c -o $(BIN)workerThread_debug.o -I $(INCLUDE) -DDEBUG

$(BIN)signalHandling_debug.o: $(SRC)signalHandling.c  $(INCLUDE)signalHandling.h  $(INCLUDE)timeout.h   $(INCLUDE)workerThread.h  $(INCLUDE)IOThread.h   $(INCLUDE)account.h\
                                                      $(INCLUDE)protocol.h  $(INCLUDE)interThreadsCommunication.h    $(INCLUDE)loadAdaptation.h   $(INCLUDE)clientState.h   $(INCLUDE)mailbox.h
	$(CC) -c $(SRC)signalHandling.c -o $(BIN)signalHandling_debug.o -I $(INCLUDE) -DDEBUG

$(BIN)account_debug.o: $(SRC)account.c  $(INCLUDE)account.h  $(INCLUDE)workerThread.h    $(INCLUDE)protocol.h
	$(CC) -c $(SRC)account.c -o $(BIN)account_debug.o -I $(INCLUDE) -DDEBUG
	
$(BIN)interThreadsCommunication_debug.o: $(SRC)interThreadsCommunication.c  $(INCLUDE)interThreadsCommunication.h   $(INCLUDE)clientState.h  $(INCLUDE)timeout.h  $(INCLUDE)protocol.h
	$(CC) -c $(SRC)interThreadsCommunication.c -o $(BIN)interThreadsCommunication_debug.o -I $(INCLUDE) -DDEBUG
	
$(BIN)loadAdaptation_debug.o: $(SRC)loadAdaptation.c  $(INCLUDE)loadAdaptation.h  $(INCLUDE)account.h   $(INCLUDE)interThreadsCommunication.h  $(INCLUDE)IOThread.h\
                                                      $(INCLUDE)mailbox.h  $(INCLUDE)main.h    $(INCLUDE)workerThread.h   $(INCLUDE)clientState.h   $(INCLUDE)protocol.h  $(INCLUDE)timeout.h
	$(CC) -c $(SRC)loadAdaptation.c -o $(BIN)loadAdaptation_debug.o -I $(INCLUDE) -DDEBUG
	
$(BIN)mailbox_debug.o: $(SRC)mailbox.c  $(INCLUDE)mailbox.h  $(INCLUDE)workerThread.h   $(INCLUDE)protocol.h
	$(CC) -c $(SRC)mailbox.c -o $(BIN)mailbox_debug.o -I $(INCLUDE) -DDEBUG
	
$(BIN)timeout_debug.o: $(SRC)timeout.c  $(INCLUDE)timeout.h  $(INCLUDE)IOThread.h   $(INCLUDE)clientState.h
	$(CC) -c $(SRC)timeout.c -o $(BIN)timeout_debug.o -I $(INCLUDE) -DDEBUG



debug: $(EXEC_DEBUG)

clean:
	\rm $(BIN)*.o
	\rm $(EXEC)
	\rm $(EXEC_DEBUG)
	
clean_files:
	\rm $(DATA)*.rtf
	\rm $(MAILBOX)*.rtf
