#include <errno.h>
#include <glob.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
void print_help() {
  printf("mycpu v2.1 by RiProG-id\n");
  printf("Usage: mycpu -- <check|default|forcemin|forcemax|help>\n");
  printf("Options:\n");
  printf("  --check       Print min and max frequencies for each CPU\n");
  printf("  --default     Set CPU frequencies to default min and max\n");
  printf("  --forcemin    Force both min and max frequencies to the minimum "
         "available frequency\n");
  printf("  --forcemax    Force both min and max frequencies to the maximum "
         "available frequency\n");
  printf("  --help        Show this help message\n");
}
void read_unlock(const char *filepath, char *output, size_t size) {
  struct stat st;
  if (stat(filepath, &st) == 0) {
    if (access(filepath, R_OK) != 0) {
      if (chmod(filepath, st.st_mode | S_IRUSR) != 0) {
        perror("chmod");
        return;
      }
    }
    FILE *file = fopen(filepath, "r");
    if (file != NULL) {
      fread(output, 1, size, file);
      fclose(file);
    } else {
      perror("fopen");
    }
  } else {
    perror("stat");
  }
}
void unlock_write_print_lock(const char *filepath, const char *value) {
  struct stat st;
  if (stat(filepath, &st) == 0) {
    if (access(filepath, W_OK) != 0) {
      if (chmod(filepath, st.st_mode | S_IWUSR) != 0) {
        perror("chmod");
        return;
      }
    }
    FILE *file = fopen(filepath, "w");
    if (file != NULL) {
      fprintf(file, "%s", value);
      fclose(file);
      if (chmod(filepath, st.st_mode & ~S_IWUSR) != 0) {
        perror("chmod");
      }
      printf("Wrote '%s' to '%s'\n", value, filepath);
    } else {
      perror("fopen");
    }
  } else {
    perror("stat");
  }
}
int main(int argc, char *argv[]) {
  if (argc != 2 || strcmp(argv[1], "--help") == 0) {
    print_help();
    return 1;
  }
  glob_t globbuf;
  char output[128];
  if (glob("/sys/devices/system/cpu/cpufreq/policy?", 0, NULL, &globbuf) == 0) {
    for (size_t i = 0; i < globbuf.gl_pathc; ++i) {
      char scaling_available_frequencies[128];
      snprintf(scaling_available_frequencies,
               sizeof(scaling_available_frequencies),
               "%s/scaling_available_frequencies", globbuf.gl_pathv[i]);
      read_unlock(scaling_available_frequencies, output, sizeof(output));
      char *token = strtok(output, " \n");
      int min_value = INT_MAX;
      int max_value = INT_MIN;
      while (token) {
        int value = atoi(token);
        if (value != 0) {
          if (value < min_value) {
            min_value = value;
          }
          if (value > max_value) {
            max_value = value;
          }
        }
        token = strtok(NULL, " \n");
      }
      if (min_value == INT_MAX || max_value == INT_MIN) {
        continue;
      }
      char cluster = globbuf.gl_pathv[i][strlen(globbuf.gl_pathv[i]) - 1];
      char min_freq_path[64];
      char max_freq_path[64];
      char min_freq_ppm[64];
      char max_freq_ppm[64];
      char min_value_path[32];
      char max_value_path[32];
      char min_value_ppm[32];
      char max_value_ppm[32];
      snprintf(min_freq_path, sizeof(min_freq_path), "%s/scaling_min_freq",
               globbuf.gl_pathv[i]);
      snprintf(max_freq_path, sizeof(max_freq_path), "%s/scaling_max_freq",
               globbuf.gl_pathv[i]);
      snprintf(min_freq_ppm, sizeof(min_freq_ppm),
               "/proc/ppm/policy/hard_userlimit_min_cpu_freq");
      snprintf(max_freq_ppm, sizeof(max_freq_ppm),
               "/proc/ppm/policy/hard_userlimit_max_cpu_freq");
      snprintf(min_value_path, sizeof(min_value_path), "%d", min_value);
      snprintf(max_value_path, sizeof(max_value_path), "%d", max_value);
      snprintf(min_value_ppm, sizeof(min_value_ppm), "%c %d", cluster,
               min_value);
      snprintf(max_value_ppm, sizeof(max_value_ppm), "%c %d", cluster,
               max_value);
      if (strcmp(argv[1], "--check") == 0) {
        printf("Cluster %c: Min freq = %d, Max freq = %d\n", cluster, min_value,
               max_value);
      } else if (strcmp(argv[1], "--default") == 0) {
        printf("Configuring CPU frequency for cluster %c...\n", cluster);
        unlock_write_print_lock(min_freq_path, min_value_path);
        unlock_write_print_lock(max_freq_path, max_value_path);
        unlock_write_print_lock(min_freq_ppm, min_value_ppm);
        unlock_write_print_lock(max_freq_ppm, max_value_ppm);
        printf("Reconfiguring CPU frequency for cluster %c...\n", cluster);
        unlock_write_print_lock(min_freq_path, min_value_path);
        unlock_write_print_lock(max_freq_path, max_value_path);
        unlock_write_print_lock(min_freq_ppm, min_value_ppm);
        unlock_write_print_lock(max_freq_ppm, max_value_ppm);
        printf("CPU frequency configuration for cluster %c is complete\n",
               cluster);
      } else if (strcmp(argv[1], "--forcemin") == 0) {
        printf("Configuring CPU frequency for cluster %c...\n", cluster);
        unlock_write_print_lock(min_freq_path, min_value_path);
        unlock_write_print_lock(max_freq_path, min_value_path);
        unlock_write_print_lock(min_freq_ppm, min_value_ppm);
        unlock_write_print_lock(max_freq_ppm, min_value_ppm);
        printf("Reconfiguring CPU frequency for cluster %c...\n", cluster);
        unlock_write_print_lock(min_freq_path, min_value_path);
        unlock_write_print_lock(max_freq_path, min_value_path);
        unlock_write_print_lock(min_freq_ppm, min_value_ppm);
        unlock_write_print_lock(max_freq_ppm, min_value_ppm);
        printf("CPU frequency configuration is complete\n");
      } else if (strcmp(argv[1], "--forcemax") == 0) {
        printf("Configuring CPU frequency for cluster %c...\n", cluster);
        unlock_write_print_lock(min_freq_path, max_value_path);
        unlock_write_print_lock(max_freq_path, max_value_path);
        unlock_write_print_lock(min_freq_ppm, max_value_ppm);
        unlock_write_print_lock(max_freq_ppm, max_value_ppm);
        printf("Reconfiguring CPU frequency for cluster %c...\n", cluster);
        unlock_write_print_lock(min_freq_path, max_value_path);
        unlock_write_print_lock(max_freq_path, max_value_path);
        unlock_write_print_lock(min_freq_ppm, max_value_ppm);
        unlock_write_print_lock(max_freq_ppm, max_value_ppm);
        printf("CPU frequency configuration for cluster %c is complete\n",
               cluster);
      } else {
        fprintf(stderr, "Invalid argument: %s\n", argv[1]);
        print_help();
        globfree(&globbuf);
        return 1;
      }
    }
    globfree(&globbuf);
  } else {
    perror("Error glob");
  }
  return 0;
}
