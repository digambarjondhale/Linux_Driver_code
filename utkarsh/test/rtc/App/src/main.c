/****************INCLUDE SECTION*********************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/****************MACRO DEFINITION*********************/
#define DS1307_DEVICE "/dev/my_ds1307_dev"

/*************FUNCTION DECLARATION******************/
char* check(char *);

/*
 * function: check()
 *
 * It validates the user input by extracting and returning a string containing only numeric characters.
 *
 * @param char *str: pointer to input string to be validated
 *
 * @return char *: a pointer to the validated numeric string, or NULL if the input string contains non-numeric characters
 *
 */
char* check(char *str){
	char *ptr = str;
	static char chpt[20];
	int i = 0;
	while(*ptr != '\0'){
		if(*ptr >= '0' && *ptr <= '9'){
			chpt[i] = *ptr;
			i++;
		}
		else{
			return NULL;
		}
		*ptr++;
	}
	chpt[i] = '\0';
	return chpt;
}

/****************MAIN FUNCTION*********************/
int main(){
	int x, n;
	char *choice;
	ssize_t bytes_read, bytes_written;
	char ch[20], read_buf[30], write_buf[12];
	/* open the device file for performing read and write operations */
	int fd = open(DS1307_DEVICE, O_RDWR);
	if(fd < 0){
		perror("Failed to open the device");
		return -1;
	}
	do{
		printf("\n\t\tEnter the operation you want to perform..\n");
		printf("\t\t1.READ date and time\n\t\t2.RESET date and time\n\t\t3.EXIT\n\t\t");
		fflush(stdin);
		gets(ch);
		choice = check(ch);
		if(choice == NULL){
			printf("\t\tEnter valid choice\n");
		}
		else{
			x = atoi(choice);
			switch(x){
				case 1:
					printf("\n\t\tReading time and date:\n");
					bytes_read = read(fd, read_buf, sizeof(read_buf));
					if(bytes_read < 0){
						perror("Error reading from the device");
						close(fd);
						return -1;
					}
					printf("\n\t\t%s\n", read_buf);
					break;
				case 2:
					printf("\n\t\tEnter new time and date (HHMMSSDDMMYY): ");
					fflush(stdin);
					gets(ch);
					choice = check(ch);
					if(choice == NULL)
						printf("Enter valid input\n");
					else{
						n = strlen(choice);
						if(n == 12){
							strcpy(write_buf, choice);
							bytes_written = write(fd, write_buf, strlen(write_buf) + 1);
							if(bytes_written < 0){
								perror("Error writing to the device");
								close(fd);
								return -1;
							}
							printf("\n\t\tNew time and date set\n");
							printf("\n\t\tTime and date: %s\n", write_buf);
						}
						else
							printf("Enter valid input\n");
					}
					break;
				case 3:
					close(fd);
					exit(1);
					fflush(stdin);
					break;
				default:
					printf("\t\tEnter valid choice\n");
					fflush(stdin);
					break;
			}
		}
	}while(x != 3);
	close(fd);
	return 0;
}
