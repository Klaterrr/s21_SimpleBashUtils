#include "s21_grep.h"

/* Entry point of the program. */
int main(int argc, char *argv[]) {
  ProgramArguments args = {argc, argv, NULL};
  Flags flags = {0};
  PatternNode *patterns = NULL;
  int exitCode = 0;

  if (!initializeArgumentTypes(&args)) {
    fprintf(stderr, "Memory allocation error!\n");
    return 1;
  }

  if (!parseFlags(&args, &flags) || !validateArguments(&args, &flags)) {
    exitCode = 1;
  } else if (!parsePatterns(&args, &flags, &patterns)) {
    exitCode = 1;
  } else {
    if (processFiles(&args, &flags, patterns) != 0) {
      exitCode = 1;
    }
  }

  free(args.argumentTypes);
  freePatterns(patterns);

  return exitCode;
}

/* Initializes the argument types array. */
int initializeArgumentTypes(ProgramArguments *args) {
  args->argumentTypes = (int *)calloc((size_t)args->argumentCount, sizeof(int));
  return args->argumentTypes != NULL;
}

/* Validates the parsed arguments for errors and required operands. */
int validateArguments(const ProgramArguments *args, const Flags *flags) {
  int hasError = 0;
  int hasFilePath = 0;
  int hasPattern = 0;
  int validation = 1;

  for (int i = 1; i < args->argumentCount; i++) {
    if (args->argumentTypes[i] == ERROR_FLAG ||
        args->argumentTypes[i] == ERROR_PATTERN ||
        args->argumentTypes[i] == ERROR_PATTERN_FILE) {
      hasError = 1;
      break;
    }
    if (args->argumentTypes[i] == ARG_FILE_PATH) {
      hasFilePath = 1;
    }
    if (args->argumentTypes[i] == ARG_PATTERN ||
        args->argumentTypes[i] == ARG_PATTERN_FILE) {
      hasPattern = 1;
    }
  }

  if (hasError || !hasFilePath || (!hasPattern && !flags->flagF)) {
    fprintf(stderr, "grep: missing pattern or file operand\n");
    validation = 0;
  }

  return validation;
}

/* Parses command-line flags and initializes the Flags structure. */
int parseFlags(ProgramArguments *args, Flags *flags) {
  int success = 1;

  for (int i = 1; i < args->argumentCount && success; i++) {
    if (args->argumentValues[i][0] == '-' &&
        args->argumentTypes[i] == ARG_NONE) {
      processFlag(args, flags, args->argumentValues[i], &i);
      /* Check for errors after processing the flag */
      if (args->argumentTypes[i] == ERROR_FLAG ||
          args->argumentTypes[i] == ERROR_PATTERN ||
          args->argumentTypes[i] == ERROR_PATTERN_FILE) {
        success = 0;
      }
    } else if (args->argumentTypes[i] == ARG_NONE) {
      /* Determine if the argument is a pattern or a file path. */
      if (!flags->standardPattern && !flags->flagE && !flags->flagF) {
        args->argumentTypes[i] = ARG_PATTERN;
        flags->standardPattern++;
      } else {
        args->argumentTypes[i] = ARG_FILE_PATH;
        flags->fileCount++;
      }
    }
  }

  return success;
}

/* Processes an individual flag and updates the Flags structure. */
void processFlag(ProgramArguments *args, Flags *flags, char *flagString,
                 int *index) {
  size_t length = strlen(flagString);
  for (size_t i = 1; i < length; i++) {
    char flagChar = flagString[i];
    switch (flagChar) {
      case 'e':
        HANDLE_PATTERN_FLAG('e', flagE, ERROR_PATTERN, ARG_PATTERN);
      case 'f':
        HANDLE_PATTERN_FLAG('f', flagF, ERROR_PATTERN_FILE, ARG_PATTERN_FILE);
      case 'i':
        flags->flagI = 1;
        break;
      case 'v':
        flags->flagV = 1;
        break;
      case 'c':
        flags->flagC = 1;
        break;
      case 'l':
        flags->flagL = 1;
        break;
      case 'n':
        flags->flagN = 1;
        break;
      case 'h':
        flags->flagH = 1;
        break;
      case 's':
        flags->flagS = 1;
        break;
      case 'o':
        flags->flagO = 1;
        break;
      default:
        args->argumentTypes[*index] = ERROR_FLAG;
        fprintf(stderr, "grep: invalid option -- %c\n", flagChar);
        return;
    }
  }
  args->argumentTypes[*index] = ARG_FLAG;
}

/* Parses all patterns from arguments and pattern files. */
int parsePatterns(ProgramArguments *args, Flags *flags,
                  PatternNode **patterns) {
  int success = 1;
  PatternNode *patternList = NULL;
  /* Load patterns from '-e' flags and standard patterns. */
  for (int i = 1; i < args->argumentCount && success; i++) {
    if (args->argumentTypes[i] == ARG_PATTERN) {
      regex_t regexCompiled;
      int regexFlags = REG_EXTENDED | (flags->flagI ? REG_ICASE : 0);
      if (regcomp(&regexCompiled, args->argumentValues[i], regexFlags) != 0) {
        fprintf(stderr, "grep: invalid regular expression: %s\n",
                args->argumentValues[i]);
        success = 0;
      } else {
        PatternNode *newNode = (PatternNode *)malloc(sizeof(PatternNode));
        if (newNode == NULL) {
          fprintf(stderr, "Memory allocation error!\n");
          success = 0;
          regfree(&regexCompiled);
        } else {
          newNode->regexCompiled = regexCompiled;
          newNode->next = patternList;
          patternList = newNode;
        }
      }
    }
  }
  /* Load patterns from files specified with '-f' flag. */
  if (flags->flagF && success) {
    for (int i = 1; i < args->argumentCount && success; i++) {
      if (args->argumentTypes[i] == ARG_PATTERN_FILE) {
        if (!loadPatternsFromFile(args->argumentValues[i], flags,
                                  &patternList)) {
          success = 0;
        }
      }
    }
  }
  if (success) {
    *patterns = patternList;
  } else {
    freePatterns(patternList);
  }
  return success;
}

/* Loads patterns from a file specified with '-f' flag. */
int loadPatternsFromFile(const char *patternFilePath, Flags *flags,
                         PatternNode **patterns) {
  FILE *file = fopen(patternFilePath, "r");
  if (file == NULL) {
    if (!flags->flagS) {
      fprintf(stderr, "grep: %s: No such file or directory\n", patternFilePath);
    }
    return 0;
  }

  char *line = NULL;
  size_t length = 0;
  int success = 1;

  while (getline(&line, &length, file) != -1 && success) {
    char *newlineChar = strchr(line, '\n');
    if (newlineChar) {
      *newlineChar = '\0';
    }
    regex_t regexCompiled;
    int regexFlags = REG_EXTENDED | (flags->flagI ? REG_ICASE : 0);
    if (regcomp(&regexCompiled, line, regexFlags) != 0) {
      fprintf(stderr, "grep: invalid regular expression: %s\n", line);
      success = 0;
    } else {
      PatternNode *newNode = (PatternNode *)malloc(sizeof(PatternNode));
      if (newNode == NULL) {
        fprintf(stderr, "Memory allocation error!\n");
        success = 0;
        regfree(&regexCompiled);
      } else {
        newNode->regexCompiled = regexCompiled;
        newNode->next = *patterns;
        *patterns = newNode;
      }
    }
  }

  free(line);
  fclose(file);
  return success;
}

/* Frees the memory allocated for compiled patterns. */
void freePatterns(PatternNode *patterns) {
  PatternNode *current = patterns;
  while (current != NULL) {
    PatternNode *next = current->next;
    regfree(&current->regexCompiled);
    free(current);
    current = next;
  }
}

/* Processes all files specified in the arguments. */
int processFiles(ProgramArguments *args, const Flags *flags,
                 PatternNode *patterns) {
  int success = 1;
  int processedFiles = 0;

  for (int i = 1; i < args->argumentCount; i++) {
    if (args->argumentTypes[i] == ARG_FILE_PATH) {
      const char *filePath = args->argumentValues[i];
      if (!processFile(filePath, flags, patterns)) {
        success = 0;
      }
      processedFiles++;
    }
  }

  if (processedFiles == 0 && !flags->flagS) {
    fprintf(stderr, "grep: no files to process\n");
    success = 0;
  }

  return success;
}

/* Processes an individual file. */
int processFile(const char *filePath, const Flags *flags,
                PatternNode *patterns) {
  FILE *file = fopen(filePath, "r");
  struct stat statBuffer;
  int success = 1;

  if (!stat(filePath, &statBuffer) && S_ISDIR(statBuffer.st_mode)) {
    if (!flags->flagS) {
      fprintf(stderr, "grep: %s: Is a directory\n", filePath);
    }
    success = 0;
  } else if (file == NULL) {
    if (!flags->flagS) {
      fprintf(stderr, "grep: %s: No such file or directory\n", filePath);
    }
    success = 0;
  } else {
    LineInfo lineInfo = {0};
    lineInfo.filePath = (char *)filePath;
    lineInfo.lineNumber = 0;
    lineInfo.matchCount = 0;

    while (readLine(file, &lineInfo)) {
      processLine(&lineInfo, flags, patterns);
    }

    if (flags->flagC) {
      if (flags->flagL) {
        lineInfo.matchCount = lineInfo.matchCount > 0 ? 1 : 0;
      }
      if (!(flags->flagL && lineInfo.matchCount == 0)) {
        printFilePath(filePath, flags);
        printf("%d\n", lineInfo.matchCount);
      }
    }
    if (flags->flagL && lineInfo.matchCount > 0) {
      printf("%s\n", filePath);
    }

    free(lineInfo.lineContent);
    fclose(file);
  }

  return success;
}

/* Reads a line from the file and updates LineInfo. */
int readLine(FILE *file, LineInfo *lineInfo) {
  ssize_t readResult =
      getline(&lineInfo->lineContent, &lineInfo->lineLength, file);
  if (readResult != -1) {
    char *newlineChar = strchr(lineInfo->lineContent, '\n');
    if (newlineChar) {
      *newlineChar = '\0';
    }
    lineInfo->lineNumber++;
    lineInfo->isMatch = 0;
    return 1;
  }
  return 0;
}

/* Processes a single line from the file. */
void processLine(LineInfo *lineInfo, const Flags *flags,
                 PatternNode *patterns) {
  if (matchLine(lineInfo, flags, patterns)) {
    lineInfo->isMatch = 1;
  }
  if (flags->flagV) {
    lineInfo->isMatch = !lineInfo->isMatch;
  }
  if (lineInfo->isMatch) {
    lineInfo->matchCount++;
    if (!flags->flagC && !flags->flagL) {
      printMatchedLine(lineInfo, flags);
    }
  }
}

/* Matches the line against all patterns. */
int matchLine(LineInfo *lineInfo, const Flags *flags, PatternNode *patterns) {
  int isMatched = 0;
  PatternNode *current = patterns;

  while (current != NULL && !isMatched) {
    regex_t *regex = &current->regexCompiled;
    regmatch_t match;

    if (flags->flagO) {
      const char *currentPosition = lineInfo->lineContent;
      while (regexec(regex, currentPosition, 1, &match, 0) == 0) {
        isMatched = 1;
        printMatchingPart(lineInfo, flags, &match);
        currentPosition += match.rm_eo;
      }
    } else {
      if (regexec(regex, lineInfo->lineContent, 1, &match, 0) == 0) {
        isMatched = 1;
      }
    }
    current = current->next;
  }

  return isMatched;
}

/* Prints a matched line according to the flags. */
void printMatchedLine(const LineInfo *lineInfo, const Flags *flags) {
  if (flags->flagO) {
    /* The matching parts are already printed in matchLine function. */
    return;
  }
  printFilePath(lineInfo->filePath, flags);
  printLineNumber(lineInfo->lineNumber, flags);
  printf("%s\n", lineInfo->lineContent);
}

/* Prints the matching part of the line when the '-o' flag is used. */
void printMatchingPart(const LineInfo *lineInfo, const Flags *flags,
                       const regmatch_t *match) {
  printFilePath(lineInfo->filePath, flags);
  printLineNumber(lineInfo->lineNumber, flags);
  printf("%.*s\n", (int)(match->rm_eo - match->rm_so),
         lineInfo->lineContent + match->rm_so);
}

/* Prints the file path if multiple files are being processed. */
void printFilePath(const char *filePath, const Flags *flags) {
  if (flags->fileCount > 1 && !flags->flagH) {
    printf("%s:", filePath);
  }
}

/* Prints the line number when the '-n' flag is used. */
void printLineNumber(int lineNumber, const Flags *flags) {
  if (flags->flagN) {
    printf("%d:", lineNumber);
  }
}
