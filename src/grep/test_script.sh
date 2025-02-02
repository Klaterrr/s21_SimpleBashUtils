#!/bin/bash

# Colors for output
GREEN='\033[0;32m'   # Green
RED='\033[0;31m'     # Red
YELLOW='\033[0;33m'  # Yellow
NC='\033[0m'         # No Color

# Commands
S21_GREP="./s21_grep"
GREP="grep"

# Counters for passed and failed tests
SUCCESS_COUNT=0
FAILURE_COUNT=0

# Make sure the s21_grep executable exists
if ! [ -x "$S21_GREP" ]; then
    echo -e "${RED}Error:${NC} s21_grep executable not found or not executable."
    echo "Please compile your program and ensure the executable is named 's21_grep'."
    exit 1
fi

# Test files directory (you can adjust this as needed)
TEST_DIR="test_files"

# Create test files if they don't exist
mkdir -p "$TEST_DIR"
echo -e "This is a test file.\nTest line.\nAnother test line.\n" > "$TEST_DIR/test1.txt"
echo -e "Line1\nLine2\nLine3\nLine4\nLine5" > "$TEST_DIR/test2.txt"
echo -e "12345\nabcde\nABCDE\n!@#$%\n" > "$TEST_DIR/test3.txt"
echo -e "Pattern matching test.\npattern matching test.\nPattern Matching Test." > "$TEST_DIR/test4.txt"
echo -e "Empty file for testing." > "$TEST_DIR/empty.txt"
touch "$TEST_DIR/nonexistent.txt"  # Will be used to simulate a nonexistent file

# Patterns file for -f flag
echo -e "Test\nLine" > "$TEST_DIR/patterns.txt"

# Array of test cases
declare -a tests=(
    # Basic tests
    "'Test' $TEST_DIR/test1.txt"
    "-i 'test' $TEST_DIR/test1.txt"
    "-v 'test' $TEST_DIR/test1.txt"
    "-c 'test' $TEST_DIR/test1.txt"
    "-l 'test' $TEST_DIR/test1.txt"
    "-n 'test' $TEST_DIR/test1.txt"
    "-e 'test' -e 'Another' $TEST_DIR/test1.txt"
    "-f $TEST_DIR/patterns.txt $TEST_DIR/test1.txt"
    "-o 'test' $TEST_DIR/test1.txt"
    "'test' $TEST_DIR/test1.txt $TEST_DIR/test2.txt"  # Multiple files
    "'test' $TEST_DIR/nonexistent_file.txt"  # Nonexistent file without -s
    "-s 'test' $TEST_DIR/nonexistent_file.txt"  # Nonexistent file with -s
    # Combined flags
    "-n -e 'Line[13]' $TEST_DIR/test2.txt"
    "-c -e 'Line[1-6]$' $TEST_DIR/test2.txt"
    "-iv 'line' $TEST_DIR/test1.txt"
    "-in 'Line' $TEST_DIR/test2.txt"
    "-o 'Line' $TEST_DIR/test2.txt"
    # Edge cases
    "-e '' $TEST_DIR/test1.txt"  # Empty pattern
    "-f $TEST_DIR/nonexistent.txt $TEST_DIR/test1.txt"  # Nonexistent pattern file
    "-e 'test' -e 'PATTERN' $TEST_DIR/empty.txt"  # Empty input file
    # Special characters
    "'[a-z]' $TEST_DIR/test3.txt"
    "-i '[A-Z]' $TEST_DIR/test3.txt"
    # Long patterns
    "'Pattern matching test' $TEST_DIR/test4.txt"
    "-i 'pattern matching test' $TEST_DIR/test4.txt"
    "-v 'no match' $TEST_DIR/test4.txt"
    # Testing -h flag
    "-h 'test' $TEST_DIR/test1.txt $TEST_DIR/test2.txt"
    # Suppressing errors with -s
    "-s 'test' $TEST_DIR/nonexistent_file.txt"
)

# Function to run a test case
run_test() {
    local test_command="$1"
    local grep_command="$GREP $test_command"
    local s21_grep_command="$S21_GREP $test_command"

    # Run the commands and capture outputs
    eval "$grep_command" > grep_output.txt 2> grep_error.txt
    local grep_exit_code=$?
    eval "$s21_grep_command" > s21_grep_output.txt 2> s21_grep_error.txt
    local s21_grep_exit_code=$?

    # Compare outputs and exit codes
    if diff -q grep_output.txt s21_grep_output.txt > /dev/null && \
       diff -q grep_error.txt s21_grep_error.txt > /dev/null; then
        echo -e "${GREEN}✅ Test passed:${NC} $test_command"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
        echo -e "${RED}❌ Test failed:${NC} $test_command"
        echo -e "${YELLOW}--- Expected Output (grep) ---${NC}"
        cat grep_output.txt
        cat grep_error.txt
        echo -e "${YELLOW}--- Actual Output (s21_grep) ---${NC}"
        cat s21_grep_output.txt
        cat s21_grep_error.txt
        FAILURE_COUNT=$((FAILURE_COUNT + 1))
    fi

    # Clean up
    rm -f grep_output.txt s21_grep_output.txt grep_error.txt s21_grep_error.txt
}

# Run all tests
for test_case in "${tests[@]}"; do
    run_test "$test_case"
done

# Summary
echo -e "\n========================================"
echo -e "Test Summary:"
echo -e "Total tests run: $((SUCCESS_COUNT + FAILURE_COUNT))"
echo -e "${GREEN}Passed: $SUCCESS_COUNT${NC}"
echo -e "${RED}Failed: $FAILURE_COUNT${NC}"
echo -e "========================================"

# Remove test files (optional)
# rm -rf "$TEST_DIR"
