#include "s21_cat.h"

int main(int argc, char *argv[]) {
  Flags flags = parse_flags(argc, argv);
  int file_count = argc - optind;
  const char **files = (const char **)&argv[optind];

  if (file_count == 0) {
    fprintf(stderr, "s21_cat: No files specified\n");
    return EXIT_FAILURE;
  }

  int status = process_files(file_count, files, &flags);
  return status;
}

Flags parse_flags(int argc, char *argv[]) {
  Flags flags = {0};
  int opt;
  while ((opt = getopt(argc, argv, "+bEnstTeEvA")) != -1) {
    switch (opt) {
      case 'b':
        flags.b_flag = 1;
        break;
      case 'E':
        flags.e_flag = 1;
        break;
      case 'e':
        flags.e_flag = 1;
        flags.v_flag = 1;
        break;
      case 'n':
        flags.n_flag = 1;
        break;
      case 's':
        flags.s_flag = 1;
        break;
      case 'T':
        flags.t_flag = 1;
        break;
      case 't':
        flags.t_flag = 1;
        flags.v_flag = 1;
        break;
      case 'v':
        flags.v_flag = 1;
        break;
      case 'A':
        flags.e_flag = 1;
        flags.t_flag = 1;
        flags.v_flag = 1;
        break;

      default:
        fprintf(stderr, "usage: s21_cat [-bEnstTeEvA] [file ...]\n");
        exit(EXIT_FAILURE);
    }
  }
  return flags;
}

int process_files(int file_count, const char *files[], const Flags *flags) {
  int status = EXIT_SUCCESS;
  for (int i = 0; i < file_count; ++i) {
    const char *filename = files[i];
    if (process_file(filename, flags) != 0) {
      status = EXIT_FAILURE;
    }
  }
  return status;
}

int process_file(const char *filename, const Flags *flags) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "s21_cat: %s: No such file or directory\n", filename);
    return 1;
  }
  int status = process_stream(fp, flags);
  fclose(fp);
  return status;
}

int process_stream(FILE *fp, const Flags *flags) {
  int c;
  int prev_c = '\n';
  int line_number = 1;
  int blank_line = 0;

  while ((c = fgetc(fp)) != EOF) {
    if (flags->s_flag) {
      if (c == '\n') {
        if (prev_c == '\n') {
          blank_line++;
          if (blank_line > 1) {
            prev_c = c;
            continue;
          }
        } else {
          blank_line = 0;
        }
      } else {
        blank_line = 0;
      }
    }

    if (prev_c == '\n') {
      if (flags->b_flag && c != '\n') {
        printf("%6d\t", line_number++);
      } else if (flags->n_flag && !flags->b_flag) {
        printf("%6d\t", line_number++);
      }
    }

    process_character(c, flags);
    prev_c = c;
  }

  return 0;
}

void process_character(int c, const Flags *flags) {
  if (c == '\t' && flags->t_flag) {
    printf("^I");
  } else if (c == '\n') {
    if (flags->e_flag) {
      printf("$\n");
    } else {
      putchar(c);
    }
  } else if (flags->v_flag && is_non_printable(c)) {
    if (c == 127) {
      printf("^?");
    } else if (c < 127) {
      printf("^%c", c + 64);
    } else {
      printf("M-^%c", c - 64);
    }
  } else {
    putchar(c);
  }
}

int is_non_printable(int c) {
  return (c < 32 && c != '\n' && c != '\t') || c == 127 ||
         (c > 127 && c <= 159);
}
