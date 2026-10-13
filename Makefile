CC = cc
AR = ar
CFLAGS = -std=c11 -Wall -Wextra -Werror -Wpedantic -g -Iinclude
CPPFLAGS =
LDFLAGS =

SRC = src/myasan.c src/tracker.c src/guards.c src/report.c src/inspection.c
OBJ = $(SRC:.c=.o)
TEST_SRC = tests/runner.c tests/test_allocation.c tests/test_free.c \
           tests/test_calloc_realloc.c tests/test_guards.c \
           tests/test_inspection.c tests/test_reports.c

.PHONY: all test check clean fclean re sanitize

all: libmyasan.a

libmyasan.a: $(OBJ)
	$(AR) rcs $@ $(OBJ)

src/%.o: src/%.c src/internal.h include/myasan.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

tests/myasan_tests: $(TEST_SRC) libmyasan.a
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_SRC) libmyasan.a $(LDFLAGS) -o $@

test: tests/myasan_tests
	./tests/myasan_tests

check: test

sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer" \
		LDFLAGS="-fsanitize=address,undefined" test
	$(MAKE) clean

clean:
	rm -f $(OBJ) libmyasan.a tests/myasan_tests tests/myasan_test.log

fclean: clean

re: clean all
