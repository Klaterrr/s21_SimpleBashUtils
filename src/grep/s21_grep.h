#ifndef S21_GREP_H
#define S21_GREP_H

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define HANDLE_PATTERN_FLAG(flag, flagField, errorType, type)                 \
  do {                                                                        \
    flags->flagField = 1;                                                     \
    if (i + 1 < length) {                                                     \
      char *value = &flagString[i + 1];                                       \
      args->argumentValues[*index] = value;                                   \
      args->argumentTypes[*index] = type;                                     \
      return;                                                                 \
    } else if (*index + 1 < args->argumentCount) {                            \
      (*index)++;                                                             \
      args->argumentTypes[*index] = type;                                     \
    } else {                                                                  \
      args->argumentTypes[*index] = errorType;                                \
      fprintf(stderr, "grep: option requires an argument -- %c\n", flagChar); \
    }                                                                         \
    return;                                                                   \
  } while (0)

/* Enumeration for argument types and error codes. */
typedef enum {
  ARG_NONE = 0,           /* Unprocessed argument. */
  ARG_FLAG = 1,           /* Indicates a flag argument. */
  ARG_PATTERN = 2,        /* Indicates a pattern argument. */
  ARG_PATTERN_FILE = 3,   /* Indicates a pattern file argument. */
  ARG_FILE_PATH = 4,      /* Indicates a file path argument. */
  ERROR_FLAG = -1,        /* Error with parsing flags. */
  ERROR_PATTERN = -2,     /* Error with pattern argument. */
  ERROR_PATTERN_FILE = -3 /* Error with pattern file argument. */
} ArgumentType;

/* Structure to hold command-line arguments and their types. */
typedef struct {
  int argumentCount;     /* Number of arguments. */
  char **argumentValues; /* Array of argument strings. */
  int *argumentTypes;    /* Array of argument types. */
} ProgramArguments;

/* Structure to hold the state of all possible flags. */
typedef struct {
  int standardPattern; /* Counter for standard patterns without flags. */
  int fileCount;       /* Number of files to read. */
  int flagE;           /* Indicates the '-e' flag. */
  int flagI;           /* Indicates the '-i' flag (ignore case). */
  int flagV;           /* Indicates the '-v' flag (invert match). */
  int flagC;           /* Indicates the '-c' flag (count of matching lines). */
  int flagL;           /* Indicates the '-l' flag (list matching files). */
  int flagN;           /* Indicates the '-n' flag (line numbers). */
  int flagH;           /* Indicates the '-h' flag (no file names). */
  int flagS;           /* Indicates the '-s' flag (suppress errors). */
  int flagF;           /* Indicates the '-f' flag (patterns from file). */
  int flagO;           /* Indicates the '-o' flag (only matching parts). */
} Flags;

/* Structure to hold information about the current line being processed. */
typedef struct {
  char *filePath;    /* Path of the file. */
  char *lineContent; /* Content of the current line. */
  size_t lineLength; /* Length of the line buffer. */
  int lineNumber;    /* Current line number in the file. */
  int isMatch;       /* Indicates if the current line matches the pattern. */
  int matchCount;    /* Total number of matches found. */
} LineInfo;

/* Structure to hold compiled regular expressions. */
typedef struct PatternNode {
  regex_t regexCompiled;    /* Compiled regular expression. */
  struct PatternNode *next; /* Pointer to the next pattern node. */
} PatternNode;

/* Function prototypes. */
int initializeArgumentTypes(ProgramArguments *args);
int validateArguments(const ProgramArguments *args, const Flags *flags);
int parseFlags(ProgramArguments *args, Flags *flags);
void processFlag(ProgramArguments *args, Flags *flags, char *flagString,
                 int *index);
int parsePatterns(ProgramArguments *args, Flags *flags, PatternNode **patterns);
int loadPatternsFromFile(const char *patternFilePath, Flags *flags,
                         PatternNode **patterns);
void freePatterns(PatternNode *patterns);
int processFiles(ProgramArguments *args, const Flags *flags,
                 PatternNode *patterns);
int processFile(const char *filePath, const Flags *flags,
                PatternNode *patterns);
int readLine(FILE *file, LineInfo *lineInfo);
void processLine(LineInfo *lineInfo, const Flags *flags, PatternNode *patterns);
int matchLine(LineInfo *lineInfo, const Flags *flags, PatternNode *patterns);
void printMatchedLine(const LineInfo *lineInfo, const Flags *flags);
void printMatchingPart(const LineInfo *lineInfo, const Flags *flags,
                       const regmatch_t *match);
void printFilePath(const char *filePath, const Flags *flags);
void printLineNumber(int lineNumber, const Flags *flags);

#endif /* S21_GREP_H */
