FILES = Main.o Predictor.o
NAME = predictors
CC = g++
CFLAG = -Wall -Wextra -DDEBUG -g -pedantic -std=c++14
OPTION = $(CFLAG) -c

all: $(NAME)


$(NAME): $(FILES)
	$(CC) $(CFLAG) $(FILES) -o $(NAME)


Predictor.o: Predictor.cpp
	$(CC) $(OPTION) Predictor.cpp -o Predictor.o

Main.o: Main.cpp
	$(CC) $(OPTION) Main.cpp -o Main.o

clean:
	rm *.o $(NAME)

run:
	./$(NAME)




checkmem: all
	valgrind ./$(NAME)
