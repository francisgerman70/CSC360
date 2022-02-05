//This Program displays basic information about the filesystem
//Used tutorial slides, FAT12 sheet, sample code and other websites as an aid for assignment
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void disk_label(char* p);
void sum_disk_size(char* p);
void free_disk_size(char* p);
int fat_entry(int n, char* p);
void get_files(char* p, int offset);
void get_subfiles(char* p, int offset);

//Global variables declaration
int count = 0;
int disk_size = 0;

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
	
	close(fd);
	

    
	char *OSname = malloc(sizeof(char));
    
	int i=0;
	while(i!=8){
		OSname[i] = p[3+i];
		i++;
	}
    printf("OS Name: %s\n", OSname);

    
	
    disk_label(p);
    //sum_disk_size(p);
	//This function prints the Total size of the disk
	int disk_size = sb.st_size;
	printf("Total size of the disk: %d bytes\n", disk_size);
    free_disk_size(p);
	printf("==============\n");
	int offset = 512*19;
	get_files(p,offset);
	printf("=============\n");
	//number of copies of the FAT
	int numFatCopies = p[16];
	printf("Number of FAT copies: %d\n", numFatCopies);
	// Sectors per FAT
	int fatSectors = p[22] + (p[23] << 8);
	printf("Sectors per FAT: %d\n", fatSectors);
    
}
// This function Prints the Label of the disk
void disk_label(char* p) {
	char *LabelOfDisk = malloc(sizeof(char));
	p += 512 * 19;
	
	for (int i = 0; i < 8; i++) {
		LabelOfDisk[i] = p[i+96];
	}

    printf("Label of the disk: %s\n", LabelOfDisk);
}	


// This function prints Free size of the disk
void free_disk_size(char* p) {
	int sum_zone = p[19] + (p[20] << 8);
	int free_zone = 0;
	int count1;
	
	for (int i = 2; i < sum_zone-31; i++) {
		if (fat_entry(i, p+512) == 0x000) {
			free_zone+=1;
		}
	}
	count1 = 512 * free_zone;
	printf("Free size of the disk: %d bytes\n", count1);
}
// This function Returns FAT entry
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
// This function counts and returns the number of files in the root and any subdirectory 
void get_files(char* p, int offset) {
	
	while (p[offset+0] != 0x00) {
		if ((p[offset+11] & 0x8) == 0 && (p[offset+11] & 0x2) == 0) {
			count++;
			
		}
		if ((p[offset+11] & 0x10) == 0x10){


			int subdir_offset = 24640;
            get_subfiles(p,subdir_offset);
			
		}
		
		p += 32;
	}

	printf("The number of files in the disk (including all files in the root directory and files in all subdirectories): %d\n\n", count);
}
 // This function counts and returns the number of files in the subdirectory    
void get_subfiles(char* p, int offset){
	
	while (p[offset+0] != 0x00) {
		if ((p[offset+11] & 0x8) == 0 && (p[offset+11] & 0x2) == 0 ) {
			count++;
			
		}

		p += 32;
	}

	
}


