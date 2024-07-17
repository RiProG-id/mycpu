#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <glob.h>
#include <limits.h>
#include <errno.h>

void print_help() {
    printf("mycpu v1.0 by RiProG-id\n");
    printf("Usage: mycpu -- <check|default|forcemin|forcemax>\n");
    printf("Options:\n");
    printf("  --check       Print min and max frequencies for each CPU\n");
    printf("  --default     Set CPU frequencies to default min and max\n");
    printf("  --forcemin    Force both min and max frequencies to the minimum available frequency\n");
    printf("  --forcemax    Force both min and max frequencies to the maximum available frequency\n");
    printf("  --help        Show this help message\n");
}

void read_unlock(const char *filepath, char *output, size_t size) {
    struct stat st;
    if (stat(filepath, &st) == 0) {
        if (access(filepath, R_OK) != 0) {
            chmod(filepath, st.st_mode | S_IRUSR);
        }
        FILE *file = fopen(filepath, "r");
        if (file) {
            fread(output, 1, size, file);
            fclose(file);
        } else {
            perror("Error reading file");
        }
    } else {
        perror("Error stating file");
    }
}

void unlock_write_lock(const char *filepath, const char *value) {
    struct stat st;
    if (stat(filepath, &st) == 0) {
        if (access(filepath, W_OK) != 0) {
            chmod(filepath, st.st_mode | S_IWUSR);
        }
        FILE *file = fopen(filepath, "w");
        if (file) {
            fprintf(file, "%s", value);
            fclose(file);
            chmod(filepath, st.st_mode & ~S_IWUSR);
            printf("Successfully wrote '%s' to '%s'\n", value, filepath);
        } else {
            perror("Error opening file for writing");
        }
    } else {
        perror("Error stating file");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        print_help();
        return 1;
    }

    glob_t globbuf;
    char output[4096];

    if (glob("/sys/devices/system/cpu/cpu?/cpufreq", 0, NULL, &globbuf) == 0) {
        int valid_argument = 0;
        for (size_t i = 0; i < globbuf.gl_pathc; ++i) {
            char scaling_available_frequencies[512];
            snprintf(scaling_available_frequencies, sizeof(scaling_available_frequencies), "%s/scaling_available_frequencies", globbuf.gl_pathv[i]);

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
                fprintf(stderr, "Error reading frequencies for %s\n", globbuf.gl_pathv[i]);
                continue;
            }

            if (strcmp(argv[1], "--check") == 0) {
                printf("Min frequency for %s: %d\n", globbuf.gl_pathv[i], min_value);
                printf("Max frequency for %s: %d\n", globbuf.gl_pathv[i], max_value);
                valid_argument = 1;
            } else if (strcmp(argv[1], "--default") == 0) {
                char min_freq_path[512];
                char max_freq_path[512];
                snprintf(min_freq_path, sizeof(min_freq_path), "%s/scaling_min_freq", globbuf.gl_pathv[i]);
                snprintf(max_freq_path, sizeof(max_freq_path), "%s/scaling_max_freq", globbuf.gl_pathv[i]);
                char min_value_str[16], max_value_str[16];
                snprintf(min_value_str, sizeof(min_value_str), "%d", min_value);
                snprintf(max_value_str, sizeof(max_value_str), "%d", max_value);
                unlock_write_lock(min_freq_path, min_value_str);
                unlock_write_lock(max_freq_path, max_value_str);
                valid_argument = 1;
            } else if (strcmp(argv[1], "--forcemin") == 0) {
                char min_freq_path[512];
                char max_freq_path[512];
                snprintf(min_freq_path, sizeof(min_freq_path), "%s/scaling_min_freq", globbuf.gl_pathv[i]);
                snprintf(max_freq_path, sizeof(max_freq_path), "%s/scaling_max_freq", globbuf.gl_pathv[i]);
                char min_value_str[16];
                snprintf(min_value_str, sizeof(min_value_str), "%d", min_value);
                unlock_write_lock(min_freq_path, min_value_str);
                unlock_write_lock(max_freq_path, min_value_str);
                valid_argument = 1;
            } else if (strcmp(argv[1], "--forcemax") == 0) {
                char min_freq_path[512];
                char max_freq_path[512];
                snprintf(min_freq_path, sizeof(min_freq_path), "%s/scaling_min_freq", globbuf.gl_pathv[i]);
                snprintf(max_freq_path, sizeof(max_freq_path), "%s/scaling_max_freq", globbuf.gl_pathv[i]);
                char max_value_str[16];
                snprintf(max_value_str, sizeof(max_value_str), "%d", max_value);
                unlock_write_lock(min_freq_path, max_value_str);
                unlock_write_lock(max_freq_path, max_value_str);
                valid_argument = 1;
            }
        }
        globfree(&globbuf);

        if (!valid_argument) {
            fprintf(stderr, "Invalid argument: %s\n", argv[1]);
            return 1;
        }
    } else {
        perror("Error glob");
        return 1;
    }

    return 0;
}
