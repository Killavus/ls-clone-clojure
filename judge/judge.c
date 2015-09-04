#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

#include <sys/param.h>
#include <dirent.h>

#define MAXFILENAMELEN 1024

struct app_config_t {
  char dir_to_list[MAXPATHLEN + 1];
  int tree_listing;
};

enum dir_entry_type {
  TYPE_FILE = 0,
  TYPE_FOLDER
};

struct dir_entry_t {
  char name[MAXFILENAMELEN];
  enum dir_entry_type type;
};

void configure_defaults(struct app_config_t *config) {  
  config->tree_listing = 0;
  char *set_cwd = getcwd(config->dir_to_list, MAXPATHLEN + 1);
  if(!set_cwd) {
    perror("getcwd");
    exit(EXIT_FAILURE);
  }
}

void configure_from_cmd(struct app_config_t *config, int argc, char *argv[]) {
  extern int optind;

  struct option available_options[] = {
    { "tree", no_argument, NULL, 't' }
  };

  char **options_source = argv;
  int options_count = argc;
  int found_t_at = -1;

  int parsed_option;
  while((parsed_option = 
          getopt_long(options_count, options_source, "t", available_options, NULL)) != -1) {
    switch(parsed_option) {
      case 't':
        found_t_at = optind;
        config->tree_listing = 1;
        break;
    }

    options_source += optind;
    options_count -= optind;
  }

  if(config->tree_listing && found_t_at == argc) {
    char msg[MAXPATHLEN + 29];
    snprintf(msg, MAXPATHLEN + 29, "Usage: %s [-t / --tree] [dir]\n", argv[0]);

    write(STDERR_FILENO, msg, strlen(msg));
    exit(EXIT_FAILURE);
  }

  if((argc == 2 && !config->tree_listing) || (argc > 2)) {
    strncpy(config->dir_to_list, 
            argv[argc - 1], 
            (size_t) MAXPATHLEN);
  }
}

void configure(struct app_config_t *config, int argc, char *argv[]) {
  configure_defaults(config);
  configure_from_cmd(config, argc, argv);
}

struct dir_entry_t* scan_directory(char *path, size_t *scanned_len) {
  DIR *directory = opendir(path);
  if(directory == NULL) {
    perror(path);
    exit(EXIT_FAILURE);
  }

  size_t predicted_size  = 8;
  size_t current_ind    = 0;

  struct dir_entry_t *entries = (struct dir_entry_t*) 
    malloc(predicted_size * sizeof(struct dir_entry_t));

  extern int errno;
  struct dirent *cursor;
  while((cursor = readdir(directory)) != NULL) {
    strncpy(entries[current_ind].name, cursor->d_name, MAXFILENAMELEN);
    entries[current_ind].type = (cursor->d_type == DT_DIR ? 
                                   TYPE_FOLDER : TYPE_FILE);

    ++current_ind;
    if(current_ind == predicted_size) {
      predicted_size *= 2;
      void *realloc_ptr = realloc(entries, 
                                  predicted_size * sizeof(struct dir_entry_t));

      if(realloc_ptr == NULL) {
        perror("realloc");
        exit(EXIT_FAILURE);
      }

      if(realloc_ptr != entries) {
        entries = (struct dir_entry_t *) realloc_ptr;
      }
    }
    errno = 0;
  }

  void *fitted_entries = realloc(entries, 
                           (current_ind + 1) * sizeof(struct dir_entry_t));

  if(fitted_entries == NULL) {
    perror("realloc");
    exit(EXIT_FAILURE);
  }

  if(fitted_entries != entries) {
    entries = (struct dir_entry_t *) fitted_entries;
  }

  if(errno) {
    perror("readdir");
    free(entries);
    exit(EXIT_FAILURE);
  }

  closedir(directory);
  (*scanned_len) = current_ind;
  return entries;
}

void prepare_tree_decorator(char *buf, int depth) {
  if(depth == 0) {
    buf[0] = '\0';
    return;
  }

  for(int i = 0; i < depth; ++i) {
    buf[i] = ' ';
  }

  buf[depth] = '\\';
  buf[depth + 1] = '_';
  buf[depth + 2] = ' ';
  buf[depth + 3] = '\0';
}

void display_tree(char *path, int depth) {
  size_t entries_len;
  struct dir_entry_t *entries;
  char new_path[MAXPATHLEN + 1],
       formatted_name[MAXFILENAMELEN + 1024],
       tree_decorator[1024];

  prepare_tree_decorator(tree_decorator, depth);

  entries = scan_directory(path, &entries_len);  
  for(size_t ind = 0; ind < entries_len; ++ind) {
    if(strncmp(entries[ind].name, ".", MAXFILENAMELEN) == 0) continue;
    if(strncmp(entries[ind].name, "..", MAXFILENAMELEN) == 0) continue;

    memset(formatted_name, 0, sizeof(formatted_name));
    memset(new_path, 0, sizeof(new_path));

    int is_folder = (entries[ind].type == TYPE_FOLDER) ? 1 : 0;
    char *name = entries[ind].name;

    strncat(formatted_name, tree_decorator, MAXFILENAMELEN + 1024);
    strncat(formatted_name, name, MAXFILENAMELEN + 1024);
    if(is_folder) {
      strncat(formatted_name, "/", MAXFILENAMELEN + 1024);
    }
    strncat(formatted_name, "\n", MAXFILENAMELEN + 1024);
    write(STDOUT_FILENO, formatted_name, strlen(formatted_name));

    if(is_folder) {
      strncat(new_path, path, MAXPATHLEN + 1);
      strncat(new_path, "/", MAXPATHLEN + 1);
      strncat(new_path, name, MAXPATHLEN + 1);

      display_tree(new_path, depth + 1);
    }
  }
  free(entries);
}

void display_shallow(char *path) {
  size_t directory_entries_length;

  struct dir_entry_t *entries = scan_directory(path, &directory_entries_length);
  for(size_t entry_ind = 0; entry_ind < directory_entries_length; ++entry_ind) {
    if(strncmp(entries[entry_ind].name, 
                "..", strlen(entries[entry_ind].name)) == 0 ||
       strncmp(entries[entry_ind].name, 
                ".", strlen(entries[entry_ind].name)) == 0
    ) { continue; }
    write(STDOUT_FILENO, entries[entry_ind].name, 
            strlen(entries[entry_ind].name));

    if(entries[entry_ind].type == TYPE_FOLDER) {
      write(STDOUT_FILENO, "/", sizeof("/"));
    }

    write(STDOUT_FILENO, " ", sizeof(" "));
  }
  write(STDOUT_FILENO, "\n", sizeof("\n"));
  free(entries);
}

int main(int argc, char *argv[]) {
  struct app_config_t app_config;
  configure(&app_config, argc, argv);

  if(app_config.tree_listing) {
    display_tree(app_config.dir_to_list, 0);
  }
  else {
    display_shallow(app_config.dir_to_list);
  }

  return EXIT_SUCCESS;
}
