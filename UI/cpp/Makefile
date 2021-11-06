DIR_Config   = ./lib/Config
DIR_EPD      = ./lib/LCD
DIR_FONTS    = ./lib/Fonts
DIR_GUI      = ./lib/GUI
DIR_BIN      = ./bin

OBJ_C = $(wildcard ${DIR_EPD}/*.c ${DIR_Config}/*.c ${DIR_GUI}/*.c ${DIR_FONTS}/*.c)
OBJ_O = main.o $(patsubst %.c,${DIR_BIN}/%.o,$(notdir ${OBJ_C}))

TARGET = main

USELIB = USE_WIRINGPI_LIB
DEBUG = -D $(USELIB)
LIB = -lrt -lpigpio -lncurses -pthread -lwiringPi -lm 

CC = g++
MSG = -g -O0 -Wall
CFLAGS += $(MSG) $(DEBUG)

${TARGET}:${OBJ_O}
	$(CC) $(CFLAGS) $(OBJ_O) -o $@ $(LIB)
main.o:main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o -I $(DIR_Config) -I $(DIR_GUI) -I $(DIR_EPD)
   
${DIR_BIN}/%.o:$(DIR_EPD)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ -I $(DIR_Config)
    
${DIR_BIN}/%.o:$(DIR_FONTS)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@
    
${DIR_BIN}/%.o:$(DIR_GUI)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ -I $(DIR_Config)  -I $(DIR_EPD)

${DIR_BIN}/%.o:$(DIR_Config)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB)
	
clean :
	rm $(DIR_BIN)/*.*
	rm $(TARGET).o
	rm $(TARGET) 
