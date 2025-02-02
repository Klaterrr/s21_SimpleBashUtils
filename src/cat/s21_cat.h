#ifndef S21_CAT_H
#define S21_CAT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  int b_flag;  // -b: Number non-blank output lines
  int e_flag;  // -e or -E: Display $ at end of each line
  int n_flag;  // -n: Number all output lines
  int s_flag;  // -s: Squeeze multiple adjacent blank lines
  int t_flag;  // -t or -T: Display TAB characters as ^I
  int v_flag;  // -v: Use ^ and M- notation, except for LFD and TAB
} Flags;

Flags parse_flags(int argc, char *argv[]);
int process_files(int file_count, const char *files[], const Flags *flags);
int process_file(const char *filename, const Flags *flags);
int process_stream(FILE *fp, const Flags *flags);
void process_character(int c, const Flags *flags);
int is_non_printable(int c);

#endif  // S21_CAT_H
