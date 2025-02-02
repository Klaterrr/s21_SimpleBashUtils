#!/bin/bash

# Colors for output
GREEN='\033[0;32m'   # Green
RED='\033[0;31m'     # Red
YELLOW='\033[0;33m'  # Yellow
NC='\033[0m'         # No Color

# Commands
S21_CAT="./s21_cat"
CAT="cat"

# Counters for passed and failed tests
SUCCESS_COUNT=0
FAILURE_COUNT=0

# Make sure the s21_cat executable exists
if ! [ -x "$S21_CAT" ]; then
    echo -e "${RED}Error:${NC} s21_cat executable not found or not executable."
    echo "Please compile your program and ensure the executable is named 's21_cat'."
    exit 1
fi

# Test files directory (you can adjust this as needed)
TEST_DIR="test_files_cat"

# Create test files if they don't exist
mkdir -p "$TEST_DIR"
echo -e "Line 1\n\nLine 3\nLine 4\n\n\nLine 7" > "$TEST_DIR/test1.txt"
echo -e "Hello\tWorld\nThis is a test file." > "$TEST_DIR/test2.txt"
echo -e "\x01\x02\x03Non-printable characters\x04\x05\x06" > "$TEST_DIR/test3.txt"
touch "$TEST_DIR/empty.txt"
touch "$TEST_DIR/nonexistent.txt"  # Will simulate a nonexistent file

# Array of test cases
declare -a tests=(
    # Basic usage
    "$TEST_DIR/test1.txt"
    "-b $TEST_DIR/test1.txt"
    "-e $TEST_DIR/test1.txt"
    "-n $TEST_DIR/test1.txt"
    "-s $TEST_DIR/test1.txt"
    "-t $TEST_DIR/test2.txt"
    "-v $TEST_DIR/test3.txt"
    "-T $TEST_DIR/test2.txt"
    "-E $TEST_DIR/test1.txt"
    "-A $TEST_DIR/test3.txt"  # Non-standard flag, should show error
    "$TEST_DIR/test1.txt $TEST_DIR/test2.txt"  # Multiple files
    "$TEST_DIR/nonexistent.txt"  # Nonexistent file
    # Combined flags
    "-benst $TEST_DIR/test1.txt"
    "-s -b $TEST_DIR/test1.txt"
    "-E -T $TEST_DIR/test2.txt"
    # Edge cases
    "$TEST_DIR/empty.txt"
    "-e $TEST_DIR/nonexistent.txt"
)

# Function to run a test case
run_test() {
    local test_command="$1"
    local cat_command="$CAT $test_command"
    local s21_cat_command="$S21_CAT $test_command"

    # Run the commands and capture outputs
    eval "$cat_command" > cat_output.txt 2> cat_error.txt
    local cat_exit_code=$?
    eval "$s21_cat_command" > s21_cat_output.txt 2> s21_cat_error.txt
    local s21_cat_exit_code=$?

    # Compare outputs and exit codes
    if diff -q cat_output.txt s21_cat_output.txt > /dev/null && \
       diff -q cat_error.txt s21_cat_error.txt > /dev/null && \
       [ $cat_exit_code -eq $s21_cat_exit_code ]; then
        echo -e "${GREEN}✅ Test passed:${NC} $test_command"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
        echo -e "${RED}❌ Test failed:${NC} $test_command"
        echo -e "${YELLOW}--- Expected Output (cat) ---${NC}"
        cat cat_output.txt
        cat cat_error.txt
        echo -e "${YELLOW}--- Actual Output (s21_cat) ---${NC}"
        cat s21_cat_output.txt
        cat s21_cat_error.txt
        FAILURE_COUNT=$((FAILURE_COUNT + 1))
    fi

    # Clean up
    rm -f cat_output.txt s21_cat_output.txt cat_error.txt s21_cat_error.txt
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
