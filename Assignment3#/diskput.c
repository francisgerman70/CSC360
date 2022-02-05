//This Program copies a file from the current Linux directory into specified directory (i.e., the root directory or a subdirectory) of the file system.
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
#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

int free_disk_size(char* p);
int fat_entry(int i, char* p);
int check_dir(char* p, char* subdir);
void put_in_root_dir(int file_size, int first_logical_cluster,  char* file_name, char* p);
void enter_fat(char* p, int value, int i);
int get_next_fat(char* p);
void check_file(char *p, char *ch);
void print_date_time(char * directory_entry_startPos);


//global variable
int fat;

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

	//Strtok and get filename
    char **sub_directory = malloc(sizeof(char));
	int sub_directory_length = 0;
    char *file_name;
	char *sub_upper;
    char *token;
	int sector;
    
    char *s = "/";
    token = strtok(argv[2], s);

    while (NULL != token) {
        sub_directory[sub_directory_length] = token;
        token = strtok(NULL, s);
        sub_directory_length++;
    }
	

    file_name = strdup(sub_directory[sub_directory_length-1]);
	

    
    
	//Check if the file exists in the current Linux dir
	int fd2;
	struct stat sb2;
	fd2 = open(file_name, O_RDWR);
	fstat(fd2, &sb2);
	if (fd2 < 0) {
		printf("File not found.\n");
		close(fd2);
		exit(1);
	}
	
	
	char * p2 = mmap(NULL,sb2.st_size , PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0); // p points to the starting pos of your mapped memory
	if (p2 == MAP_FAILED) {
		printf("Error: failed to map memory2\n");
		exit(1);
	}
	
	close(fd);

	// Convert the given filename to upper case
	int j = 0;
    while (file_name[j]) {
        file_name[j] = toupper(file_name[j]);
        j++;
    }
	
	int free_size = free_disk_size(p);
	int file_size = sb2.st_size;

	//Check if the disk has enough space to store the file
	if(file_size > free_size){
        printf("No enough free space in the disk image\n");
        exit(1);
    }

	if (sub_directory_length>=2){
		// Convert the given subdir to upper case
		sub_upper = sub_directory[0];
		int x = 0;
    	while (sub_upper[x]) {
        	sub_upper[x] = toupper(sub_upper[x]);
        	x++;
    	}

	//Check if the specified directory exists in the fat-12 image
	int exist = check_dir(p,sub_upper);
		
		if(exist==1){
			check_file(p+24640,file_name);
			sector = get_next_fat(p);
			put_in_root_dir(file_size,sector, file_name, p+24640);
			enter_fat(p, sector, fat);
		}else{
			
			printf("The directory not found\n");
        	exit(1);
		}
	}else{
		check_file(p+512*19,file_name);
		sector = get_next_fat(p);
		put_in_root_dir(file_size, sector, file_name, p+512*19);
		
		enter_fat(p, sector, fat);
		
		
	}
		
	
	munmap(p, sb.st_size);
	munmap(p2, file_size);
	close(fd);
	close(fd2);

	return 0;
}


int free_disk_size(char* p) {
	int sum_zone = p[19] + (p[20] << 8);
	int free_zone = 0;
	int count1;
	
	for (int i = 2; i < sum_zone-31; i++) {
		if (fat_entry(i, p+512) == 0x000) {
			free_zone+=1;
		}
	}
	count1 = 512 * free_zone;
	return count1;
}


int check_dir(char* p, char* subdir) {
	p+=512*19;
	int count =0 ;
	while (p[0] != 0x00) {	
		
		char* file_name = malloc(sizeof(char));
		
		int i =0;
		while(i != 8){
			if (p[i]==' '){
				break;
			}
			
			file_name[i] = p[i];
			i++;
		}
		if ((p[11] & 0x10) == 0x10){

        if(strcmp(file_name , subdir) == 0){ 
    		
			
			count++;
			
    	}
		}
        p += 32;
    }
	return count;
}

int get_next_fat(char* p) {
	int sum_zone = p[19] + (p[20] << 8);
    int i;
    for (i=2; i<=sum_zone-31; i++) {
        if (fat_entry(i, p+512) == 0x000) {
            fat = i;
            return i;
        }
    }
}

void enter_fat(char* p, int value, int i ) {
	p += 512;
	int value1;
	int value2;

	if ((i % 2) == 0) {
		value1 = (value >> 8) & 0x0F;
		value2 = value & 0xFF;
		p[512+ ((3*i) / 2) + 1] = value1;
		p[512 + ((3*i) / 2)] = value2;
		
		
	} else {
		value1 = (value << 4) & 0xF0;
		value2 = (value >> 4) & 0xFF;
		p[512 + ((3*i) / 2)] = value1;
		p[512 + ((3*i) / 2) + 1] = value2;
		
	}
}

int fat_entry(int i, char* p) {
	
	int high_side;
	int low_side;
	int sum;
	

	if (i % 2 == 0) {
		high_side = p[1 + ((3*i) / 2) ] & 0x0F;
		low_side = p[((3*i) / 2)] & 0xFF;
		sum = (high_side << 8) + low_side;
	} else {
		high_side = p[((3*i) / 2) ] & 0xF0; 
		low_side = p[1 + ((3*i) / 2)] & 0xFF;
		sum = (high_side >> 4) + (low_side << 4);
	}

	return sum;
}
void put_in_root_dir(int file_size, int first_logical_cluster,  char* file_name, char* p) {
	
	while (p[0] != 0x00) {
		p += 32;
	}

	int size = strlen(file_name);
	
	int i;
	for (i = 0; i < 8; i++) {
		if ('.' == file_name[i]){
			break;
		}
		p[i] = file_name[i];
	}
	
	for (; i<8; i++) {
        
        p[i] = ' ';
    }
	for (int x = 0; x < 3; x++) {
		p[x+8] = file_name[size-1];
		size--;
	}
	
	print_date_time(p);
	
	
	
	p[28] = (file_size & 0x000000FF);
	p[29] = (file_size & 0x0000FF00) >> 8;
	p[30] = (file_size & 0x00FF0000) >> 16;
	p[31] = (file_size & 0xFF000000) >> 24;
	p[26] = (first_logical_cluster - (p[27] << 8)) & 0xFF;
	p[27] = (first_logical_cluster - p[26]) >> 8;
	

}

void check_file(char *p, char *ch){
    
    while (p[0] != 0x00) {
        
		
		char* file_name = malloc(sizeof(char));
		char* remainder = malloc(sizeof(char));
		for (int i = 0; i < 8; i++) {
			if (p[i] == ' ') {
				continue;
			}
			file_name[i] = p[i];
		}
        
		for (int i = 0; i < 3; i++) {
			remainder[i] = p[i+8];
		}
		strcat(file_name, ".");
		strcat(file_name, remainder);

        if(strcmp(file_name , ch) == 0){ 
    		printf("File already exist\n");
        	exit(1);
    	}
        p += 32;
    }
    
    
}
void print_date_time(char * p){
	
	time_t c = time(NULL);
	struct tm *info = localtime(&c);
	int year = info->tm_year + 2000-2;
	int month = (info->tm_mon + 3);
	int day = info->tm_mday;
	int hour = info->tm_hour;
	int minute = info->tm_min;
	
	
	p[17] |= (year - 500);
	p[16] |= (month)<<6;
	p[16] |= (day);
	p[15] |= (hour << 3) & 0xF8;
	p[15] |= (minute - ((p[14] & 0xE0) >> 5)) >> 3;
	p[14] |= (minute - ((p[15] & 0x7) << 3)) << 5;
	
	
	
	return ;	
}