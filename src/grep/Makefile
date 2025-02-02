CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -D_GNU_SOURCE
LDFLAGS = -lm

SRCS = $(wildcard ./*.c)
OBJS = $(SRCS:.c=.o)

TARGET = s21_grep

.PHONY: all clean rebuild test checks all_checks all_fix cp_cf run_tests

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

cp_cf:
	cp ../../materials/linters/.clang-format .

checks:
	clang-format -n *.h *.c
	cppcheck --enable=all --check-level=exhaustive --verbose --suppress=missingIncludeSystem --suppress=checkersReport --std=c11 ${SRCS}

all_checks: cp_cf checks

all_fix:
	clang-format -i *.h *.c

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) .clang-format *.txt

rebuild: clean all

# Новая цель для тестирования программы
run_tests:
	@echo "Tests initializating..."
	@chmod +x ./test_script.sh
	@./test_script.sh
