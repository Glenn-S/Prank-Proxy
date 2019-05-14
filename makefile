#Makefile
#
# CPSC 441 Assignment 1
# Glenn Skelton
# 10041868
# 
# A simple make file for creating artifacts for the web proxy
#
# To make the object file type "make proxy.o"
# To build the executable type "make all"
# To run a default version with a proxy already set type "make run"
# To clean up all object and execeutable files type "make clean" 

FLAGS = -Wall -g
CC = gcc
SRC = proxy.c
OBJ = $(SRC:.c=.o)
MAIN = proxy

all: $(OBJ)
	@echo Building Artifacts
	$(CC) $(FLAGS) $(OBJ) -o $(MAIN)

proxy.o: $(SRC)
	@echo Bulding Objects
	$(CC) $(FLAGS) $(SRC) -c

run:
	./$(MAIN) 12345

clean:
	@echo Destroying $(OBJ) $(MAIN)
	$(RM) *.o $(MAIN)

.PHONY:
	clean all

