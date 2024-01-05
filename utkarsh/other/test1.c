#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DS1307_DEVICE "/dev/my_ds1307_dev"

int main() {
    int fd = open(DS1307_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return -1;
    }

    char read_buf[30];
    char write_buf[12];

    printf("Reading time and date:\n");
    ssize_t bytes_read = read(fd, read_buf, sizeof(read_buf));
    if (bytes_read < 0) {
        perror("Error reading from the device");
        close(fd);
        return -1;
    }
   // read_buf[bytes_read] = '\0';
    printf("%s\n", read_buf);

    printf("Enter new time and date (HHMMSSDDMMYY): ");
    if (scanf("%s", write_buf) != 1) {
        printf("Invalid input\n");
        close(fd);
        return -1;
    }

    ssize_t bytes_written = write(fd, write_buf, strlen(write_buf));
    if (bytes_written < 0) {
        perror("Error writing to the device");
        close(fd);
        return -1;
    }

    printf("New time and date set:\n");
    printf("Time and date: %s\n", write_buf);

    close(fd);
    return 0;
}

