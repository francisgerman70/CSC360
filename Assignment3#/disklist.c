//This Program displays the contents of the root directory and all sub-directories (possibly multi-layers) in the file system.
//Used tutorial slides, FAT12 sheet, sample code and other websites as an aid for assignment
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>


#define timeOffset 14 //offset of creation time in directory entry
#define dateOffset 16 //offset of creation date in directory entry

//Global variables declaration
char *subdir;
int count2 = 0;



void list_files(char* p);
int get_size(char* p);
void directory(char *dir_name, char *p, int offset);
void print_date_time(char * directory_entry_startPos);

int main(int argc, char *argv[])
{
	int fd;
	struct stat sb;

	fd = open(argv[1], O_RDWR);
	fstat(fd, &sb);
	

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}
	
    char *root = "Root";
    char *dir_start= "/";
    printf("%s\n==================\n", root);
    int offset = 512 * 19;
    directory(dir_start, p ,offset);
	close(fd);
}


// This function lists the files present in directory and subdirectory
void list_files(char* p) {
	int offset = 512*19;
	int address = 33*512;
	while (p[0] != 0x00 && offset <address) {
		
		char* file_name = malloc(sizeof(char));
		char* remainder = malloc(sizeof(char));
		int i =0;
		while(i != 8){
			if (p[i]==' '){
				break;
			}
			
			file_name[i] = p[i];
			i++;
		}
		
        if ((p[11] & 0x10) != 0x10){
			char info ='F';
		
		
		int r = 0;
		while(r!=3){
			remainder[r] = p[r+8];
			r++;
		}
		strcat(file_name, ".");
		strcat(file_name, remainder);
		int j = 0;
    
    while (file_name[j]) {

        file_name[j] = tolower(file_name[j]);
        j++;
    }
        int size = get_size(p);
		
		if ((p[11] & 0x8) == 0 && (p[11] & 0x2) == 0) {
			printf("%c %10d %20s ", info, size, file_name);
			print_date_time(p);
		}

		p += 32;
		offset+=32;
    }else{
		
		int x = 0;
		while (file_name[x]) {

        file_name[x] = tolower(file_name[x]);
        x++;
    }
		char info = 'D';
        int count = 0;
        printf("%c %10d %20s ", info, count, file_name);
		print_date_time(p);
        subdir = file_name;
        count2++;

        p += 32;
		offset+=32;
    }
	}
}
// This function returns the size of file
int get_size(char* p) {
	int size = (p[28] & 0xFF) + ((p[29] & 0xFF) << 8) + ((p[30] & 0xFF) << 16) + ((p[31] & 0xFF) << 24);
	return size;
}

void directory(char *dir_start, char *p, int offset){
	
    list_files(p + offset);
    if(count2 > 0){
    printf("%s%s\n==================\n", dir_start,subdir);
    list_files(p + 24640);
}
    
}

void print_date_time(char * directory_entry_startPos){
	
	int time, date;
	int hours, minutes, day, month, year;
	
	time = *(unsigned short *)(directory_entry_startPos + timeOffset);
	date = *(unsigned short *)(directory_entry_startPos + dateOffset);
	
	//the year is stored as a value since 1980
	//the year is stored in the high seven bits
	year = ((date & 0xFE00) >> 9) + 1980;
	//the month is stored in the middle four bits
	month = (date & 0x1E0) >> 5;
	//the day is stored in the low five bits
	day = (date & 0x1F);
	
	printf("%d-%d-%d ", year, month, day);
	//the hours are stored in the high five bits
	hours = (time & 0xF800) >> 11;
	//the minutes are stored in the middle 6 bits
	minutes = (time & 0x7E0) >> 5;
	
	printf("%02d:%02d\n", hours, minutes);
	
	return ;	
}