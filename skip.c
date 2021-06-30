#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

typedef struct str_list_node_t {
    char* str;
    struct str_list_node_t* next;
} str_list_node_t;

typedef struct {
    str_list_node_t* first;
    str_list_node_t* last;
} str_list_t;

str_list_t* str_list_new() {
    str_list_t* new_str_list = malloc(sizeof(str_list_t));
    new_str_list->first = NULL;
    new_str_list->last = NULL;
    return new_str_list;
}

void str_list_push(str_list_t* str_list, char* str) {
    str_list_node_t* new_node = malloc(sizeof(str_list_node_t));
    new_node->str = str;
    new_node->next = NULL;

    if (str_list->last == NULL) {
        str_list->first = new_node;
        str_list->last = new_node;
    }
    else {
        str_list->last->next = new_node;
        str_list->last = new_node;
    }
}

void str_list_flush(str_list_t* str_list, FILE* fh) {
    str_list_node_t* curr = str_list->first;
    while (curr != NULL) {
        str_list_node_t* next = curr->next;
        fprintf(fh, "%s", curr->str);
        free(curr->str);
        free(curr);
        curr = next;
    }
    str_list->first = NULL;
    str_list->last = NULL;
}

void str_list_destroy(str_list_t* str_list) {
    str_list_node_t* curr = str_list->first;
    while (curr != NULL) {
        str_list_node_t* next = curr->next;
        free(curr->str);
        free(curr);
        curr = next;
    }
    free(str_list);
}


char* str_slice(char* buffer, size_t start, size_t length) {
  char* str = malloc(sizeof(char) * length + 1);
  memcpy(str, buffer + start, length);
  str[length] = '\0';
  return str;
}

void skip(long lines_to_skip, FILE* fh_in, FILE* fh_out) {
  const size_t BUFFER_SIZE = 65536;
  const char LF = 10;

  char buffer[BUFFER_SIZE];
  size_t bytes_read = BUFFER_SIZE;
  long lines_found = 0;

  while (!feof(fh_in)) {
    bytes_read = fread(buffer, 1, BUFFER_SIZE, fh_in);
    int i = 0;

    if (lines_found < lines_to_skip) {
      for (; i<bytes_read; i++) {
        if (buffer[i] == LF) {
          lines_found++;
        }
        if (lines_found >= lines_to_skip) {
          i++;
          break;
        }
      }
    }
    if (lines_found >= lines_to_skip) {
      fwrite(buffer + i, 1, bytes_read - i, fh_out);
    }
  }
}

void skip_back(long lines_to_skip, FILE* fh_in, FILE* fh_out) {
  size_t BUFFER_SIZE = 65536;
  const char LF = 10;

  char read_buffer[BUFFER_SIZE];
  size_t bytes_read = BUFFER_SIZE;

  str_list_t* line_buffers[lines_to_skip];
  for (int j=0; j<lines_to_skip; j++) line_buffers[j] = str_list_new();

  int lines_found = 0;
  int lb_i = 0;
  int is_first_element = 1;

  while (!feof(fh_in)) {
    bytes_read = fread(read_buffer, 1, BUFFER_SIZE, fh_in);
    int i = 0;
    int line_start = 0;

    for (; i<bytes_read; i++) {
      int is_linefeed = read_buffer[i] == LF;
      if (is_linefeed || i == bytes_read - 1) {
        if (is_linefeed) lines_found++;

        if (is_first_element) {
          if (lines_found >= lines_to_skip) {
            str_list_flush(line_buffers[lb_i], fh_out);
          }
        }
        is_first_element = 0;

        int line_length = i - line_start + 1;
        char* str = str_slice(read_buffer, line_start, line_length);
        str_list_push(line_buffers[lb_i], str);

        line_start = i + 1;

        if (is_linefeed) {
          lb_i = (lb_i + 1) % lines_to_skip;
          is_first_element = 1;
        }
      }
    }
  }

  for (int j=0; j<lines_to_skip; j++) str_list_destroy(line_buffers[j]);
}

void print_help() {
  puts("usage: skip [options] [file ...]");
  puts("    -n NUMBER, --number NUMBER        number of lines to skip");
  puts("    -o FILE, --output FILE            path to output file");
  puts("    -b, --back                        skip lines from the back of the file");
  puts("    -h, --help                        print help");
}

int main(int argc, char** argv) {
  long lines_to_skip = 10;
  int from_back = 0;
  FILE* fh_in = stdin;
  FILE* fh_out = stdout;

  static struct option long_options[] = {
    { "number", required_argument, 0, 'n' },
    { "back", no_argument, 0, 'b' },
    { "help", no_argument, 0, 'h' }
  };

  while (1) {
    int option_index = 0;
    int c = getopt_long(argc, argv, "n:o:hb", long_options, &option_index);
    char *_tail;

    if (c == -1) break;

    switch (c) {
    case 'n':
      lines_to_skip = strtol(optarg, &_tail, 10);
      break;
    case 'o':
      fh_out = fopen(optarg, "wb");
      if (fh_out == NULL) {
        fprintf(stderr, "Can't write to file: %s\n", optarg);
        return 1;
      }
      break;
    case 'b':
      from_back = 1;
      break;
    case 'h':
      print_help();
      return 0;
    }
  }

  if (optind < argc) {
    int i;
    for (i = optind; i < argc; i++) {
      if (strcmp(argv[i], "-") == 0) {
        fh_in = stdin;
      }
      else {
        fh_in = fopen(argv[i], "rb");
        if (fh_in == NULL) {
          fprintf(stderr, "Can't read file: %s\n", argv[i]);
          return 1;
        }
      }

      if (from_back) skip_back(lines_to_skip, fh_in, fh_out);
      else skip(lines_to_skip, fh_in, fh_out);
      fclose(fh_in);
    }
  }
  else {
    if (from_back) skip_back(lines_to_skip, fh_in, fh_out);
    else skip(lines_to_skip, fh_in, fh_out);
    fclose(fh_in);
  }

  fclose(fh_out);

  return 0;
}
