#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define BUFFER_SIZE 65536
// compile: gcc -fPIC -shared -O3 -o fast_read.so fast_read.c

int get_file_rows(char *filename) {
    int fd;
    char buf[BUFFER_SIZE];

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        return 1;
    }

    int lines = 0;
    while (1) {
        ssize_t count = read(fd, buf, sizeof(buf));
        if (count == -1) {
            return 1;
        }
        if (count == 0) {
            break;
        }
        char *p = buf;
        char *end = buf + count;
        while (p != end) {
            lines += *p++ == '\n';
        }
    }
    close(fd);
    return lines;
}


int get_file_cols(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open file\n");
        return -1;
    }

    int count = 1;
    char ch;
    while ((ch = fgetc(file)) != '\n' && ch != EOF) {
        if (ch == ',') {
            count++;
        }
        if (ch == '\n') {
            break;
        }
    }
    fclose(file);
    return count;
}


double fast_atof(const char* str) {
    double result = 0.0;
    double fraction = 1.0;
    int state = 0; // 0: integer part, 1: fraction part
    int sign = 1;  // 1: positive, -1: negative

    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    }

    while (*str) {
        if (*str == '.') {
            state = 1;
        } else {
            if (state == 0) {
                result = result * 10.0 + (*str - '0');
            } else if (state == 1) {
                fraction /= 10.0;
                result += (*str - '0') * fraction;
            }
        }
        str++;
    }
    return result * sign;
}

double* get_file(const char* filename, int row_num, int col_num) {
    int fd;
    ssize_t bytes_read;
    char buffer[BUFFER_SIZE];
    size_t buffer_pos = 0;
    double* data;
    size_t data_pos = 0;

    // 基于文件的预期大小预先分配数组
    unsigned long long total_numbers = col_num * row_num;
    data = (double*) malloc(total_numbers * sizeof(double));
    if (!data) {
        perror("Error allocating memory");
        return NULL;
    }

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening the file");
        free(data);
        return NULL;
    }
    int commas_num = 0;

    while ((bytes_read = read(fd, buffer + buffer_pos, BUFFER_SIZE - buffer_pos)) > 0) {
        char* start = buffer;
        char* end;
        size_t total_bytes = bytes_read + buffer_pos; // Total bytes in buffer after the read
        size_t remaining = total_bytes;

        while (remaining > 0) {
            // 处理,
            for (int i = commas_num; i < col_num-1 && remaining > 0; i++) {
                end = memchr(start, ',', remaining);
                if (!end) {
                    break;
                }
                commas_num++;
                *end = '\0';
                data[data_pos++] = fast_atof(start);
                remaining -= (end - start + 1);
                start = end + 1;
            }

            // 处理\n
            if (remaining > 0) {
                end = memchr(start, '\n', remaining);
                if (end) {
                    *end = '\0';
            
            if ((*start) == 't') {
			    data[data_pos++] = 1;  // 处理true
		    }
		    else {
			    data[data_pos++] = 0;  // 处理false
		    }
                    remaining -= (end - start + 1);
                    start = end + 1;
                    commas_num = 0;
                } else {
                    break;
                }
            }
        }

        if (remaining > 0) {
            memmove(buffer, start, remaining);
        }
        buffer_pos = remaining;
    }

    if (bytes_read == -1) {
        perror("Error reading the file");
        free(data);
        close(fd);
        return NULL;
    }

    if (buffer_pos > 0) {
        data[data_pos++] = fast_atof(buffer);
    }

    close(fd);
    // free(data);
    return data;
}

int main() {
    struct timeval start, end;
    double time_taken;
    gettimeofday(&start, NULL); // 开始计时

    int cols = get_file_cols("data.csv");
    printf("%d\n", cols);
    
    int rows = get_file_rows("data.csv");
    printf("%d\n", rows);
    
    get_file("./data.csv", 10, 10000000);
    gettimeofday(&end, NULL); // 结束计时

    time_taken = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Time taken to read the file: %f seconds\n", time_taken);
}
