//This Program copies a file from the root directory of the file system to the current directory in Linux.
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

void check_file(char *p, char *ch);
int fat_entry(int n, char* p);
void copy_file(char *p,char *newFile);
int get_size(char *p,char *newFile);
//Global Variable
int count = 0;


int main(int argc, char *argv[])
{
	int fd;
	struct stat sb;
    char *newFile;

	fd = open(argv[1], O_RDWR);
	fstat(fd, &sb);
	

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}
    
	
	newFile = argv[2];
    close(fd);

	// Convert the given filename to upper case, then search this filename from directory entries in root folder
    int j = 0;
    while (newFile[j]) {

        newFile[j] = toupper(newFile[j]);
        j++;
    }
	// check if filename is in root directory
    check_file(p + 512*19,newFile);
    if (count == 0){
        printf("File not found.\n");
    }else{
		//filename matching, extract the first logical cluster & file size.
        copy_file(p, newFile);
    }

	
	
	
}
// This function checks if filename is in root directory
void check_file(char *p, char *ch){
    
    while (p[0] != 0x00) {
        
		
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
        
		int r = 0;
		while(r!=3){
			remainder[r] = p[r+8];
			r++;
		}
		strcat(file_name, ".");
		strcat(file_name, remainder);

        if(strcmp(file_name , ch) == 0){ 
    		count++;
    	}
        p += 32;
    }
    
    
}
// This function checks copies file from root directory to current linux directory
void copy_file(char *p,char *newFile){
	int size = get_size(p,newFile);
    int cluster = (p[26]) + (p[27] << 8);
	int size_left = size;
	
	int Address;
	FILE *fp;
	fp = fopen(newFile, "w+");
    while(fat_entry(cluster,p+512) != 0xfff && size_left != 0 ){
		Address = 512 * (31 + cluster);
		for(int i = 0; i < 512; i++){
			fputc(p[Address + i], fp);
			
		}
		
		
		size_left--;
		
	}
	
	
}

int get_size(char *p,char *newFile){
	
	p+=512*19;
	while (p[0] != 0x00) {
        
		
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
        
		int r = 0;
		while(r!=3){
			remainder[r] = p[r+8];
			r++;
		}
		strcat(file_name, ".");
		strcat(file_name, remainder);

        if(strcmp(file_name , newFile) == 0){ 
    		int size=(p[28] & 0xFF) + ((p[29] & 0xFF) << 8) + ((p[30] & 0xFF) << 16) + ((p[31] & 0xFF) << 24);
			
			return size;
    	}
        p += 32;
		
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
