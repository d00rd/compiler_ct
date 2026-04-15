CC = gcc
CFLAGS = -Wall -std=c99
LEXER = lexer
TESTS_DIR = tests
TEST_FILES = $(wildcard $(TESTS_DIR)/*.c)

.PHONY: all run clean

all: $(LEXER)

$(LEXER): lexer.c
	$(CC) $(CFLAGS) -o $(LEXER) lexer.c

run: $(LEXER)
	@for test_file in $(TEST_FILES); do \
		echo "Running lexer on $$test_file:"; \
		./$(LEXER) $$test_file; \
		echo ""; \
	done

clean:
	rm -f $(LEXER)
	rm -f tests-out/*.out
