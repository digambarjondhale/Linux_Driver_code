#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

int main(void) {
    printf("Raspberry Pi RTC DS1307 Sample\n");
    printf("========================================\n");

    int deviceHandle;
    int readBytes;
    char buffer[7];

    // initialize buffer
    buffer[0] = 0x00;

    // address of DS1307 RTC device
    int deviceI2CAddress = 0x68;

    // open device on /dev/my_ds1307_dev (this should match the device path you provided in your kernel module)
    if ((deviceHandle = open("/dev/my_ds1307_dev", O_RDWR)) < 0) {
        printf("Error: Couldn't open device! %d\n", deviceHandle);
        return 1;
    }

    // connect to DS1307 as i2c slave
    if (ioctl(deviceHandle, I2C_SLAVE, deviceI2CAddress) < 0) {
        printf("Error: Couldn't find device on address!\n");
        return 1;
    }

    // read response
    readBytes = read(deviceHandle, buffer, 7);
    if (readBytes != 7) {
        printf("Error: Received no data!");
    } else {
        // get data
        int seconds = buffer[0] & 0x7F; // Masking CH bit
        int minutes = buffer[1] & 0x7F;
        int hours = buffer[2] & 0x3F;   // Masking 12/24-hour bit
        int dayOfWeek = buffer[3] & 0x07;
        int day = buffer[4] & 0x3F;
        int month = buffer[5] & 0x1F;
        int year = buffer[6] + 2000;    // Convert 2-digit year to 4-digit year

        // and print results
        printf("Actual RTC-time:\n");
        printf("Date: %d-%02d-%02d\n", year, month, day);
        printf("Time: %02d:%02d:%02d\n", hours, minutes, seconds);
    }

    // close connection and return
    close(deviceHandle);
    return 0;
}

