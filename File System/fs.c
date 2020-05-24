#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"

#define MAX_FILDES 32
#define MAX_F_NAME 15
#define MAX_FILES 64
#define DISK_BLOCKS 8192
#define BLOCK_SIZE 4096

#define FREE -2
#define RESERVED -3

struct super_block{
	int fat_idx; //first block of the FAT
	int fat_len; //Length of FAT in blocks
	int dir_idx; //First block of directory
	int dir_len; //Length of directory in blocks
	int data_idx; //First block of file-data
};

struct dir_entry {
	int used; //Is this file-"slot" in use
	char name[MAX_F_NAME + 1]; //DOH?
	int size; //file size
	int head; //first data block of file
	int ref_cnt; //how many open file
		    //descripters are there?
	//ref_cnt > 0 ->cannot delete file
};

struct file_descriptor{
	int used; //fd in use
	int file; // the first block of the file
		  // (f) to which fd refers too
	int offset; //position of fd within f
};
	
struct super_block fs;
struct file_descriptor fildes[MAX_FILDES];
int *FAT; //Will be populated with the FAT data
struct dir_entry *DIR; //will be populated with the directory data

static int check = 0;
static int open = 0;
static int delete = 0;
int make_fs(char *disk_name);
int mount_fs(char *disk_name);
int umount_fs(char *disk_name);
//int fs_open(char *name);
//int fs_close(int fildes);
//int fs_create(char *name);
//int fs_delete(char *name);
//int fs_read(int fildes, void *buf, size_t nbyte);
//int fs_write(int fildes, void *buf, size_t nbyte);
//int fs_get_filesize(int fildes);
//int fs_listfiles(char ***files);
//int fs_lseek(int fildes, off_t offset);
//int fs_truncate(int fildes, off_t length);

int make_fs(char* disk_name){
	printf("im in make_fs\n");
	
	check = make_disk(disk_name);
	if(check == -1)
		return -1;

	check = mount_fs(disk_name);
	if(check == -1)
		return -1;
	
	DIR = (struct dir_entry*) calloc(MAX_FILES,(sizeof(struct dir_entry )));
	FAT = (int *) calloc (DISK_BLOCKS,(sizeof(int)));		 
	
	check = umount_fs(disk_name);
	if(check == -1)
		return -1;			
return 0;	 
}

int mount_fs(char *disk_name){
	printf("i'm in mount\n");
	check = open_disk(disk_name);
	if(check == -1)
		return -1;

	// block read fat and dir into memory
	char buffer[BLOCK_SIZE];

	memset(buffer, 0, BLOCK_SIZE);
	block_read(0,buffer);
	memcpy(buffer,&fs , sizeof(struct super_block));

	memset(buffer, 0, BLOCK_SIZE);	
	fs.dir_idx = block_read(1,buffer);
	fs.dir_len = sizeof(buffer+1);
	memcpy(buffer+1, &DIR, sizeof(struct dir_entry));
	
	memset(buffer, 0 ,BLOCK_SIZE);
	fs.fat_idx = block_read(2,buffer);
	fs.fat_len = sizeof(buffer+2);
	memcpy(buffer+2, &FAT, sizeof(int));


	return 0;
}

int umount_fs(char *disk_name){
	
	printf("im in un mount_fs\n");
       
        // block read fat and dir into memory
	char buffer[BLOCK_SIZE];
        memset(buffer, 0, BLOCK_SIZE);
        block_write(0,buffer);	
        memcpy(buffer,&fs , sizeof(struct super_block));
	
        memset(buffer, 0, BLOCK_SIZE);
        fs.dir_idx = block_write(1,buffer);
	fs.dir_len = sizeof(buffer+1);
        memcpy(buffer+1, &DIR, sizeof(struct dir_entry));

        memset(buffer, 0 ,BLOCK_SIZE);
        fs.fat_idx = block_write(2,buffer);
	fs.fat_len = sizeof(buffer+2);
        memcpy(buffer+2, &FAT, sizeof(int));
	
       	
	check = close_disk(disk_name);
	if(check == -1)
		return -1;

	return 0;
}

int fs_open(char *name){
	
	//printf("im in fsopen\n");
	//printf("idx of dir: %d\n",fs.dir_idx); 
	struct dir_entry *search = DIR;
	if(name == NULL)
		return -1;
	if(delete == 1)
		return -1;

	int i = 0;	
	while(i<MAX_FILES){
		if(strcmp(search->name,name))
			break;
		i++;
		search = search+i;
	}
	if(i == MAX_FILES)
		return -1;
	else{	
	fildes[i].used = 1; 		
	fildes[i].file = (DIR+i)->head;
	fildes[i].offset = 0;
	}
	
	if(search->ref_cnt == MAX_FILDES)
		return -1;

	DIR = DIR +i;
	if(!strcmp(search->name,name)){
		if(DIR->ref_cnt < MAX_FILDES)
			{DIR->ref_cnt++;
			fildes[i].file = search->head;
			fildes[i].offset = 0;
			}
		else if(DIR->ref_cnt == MAX_FILDES)
			return -1;
		else
			return -1;	
		}
	DIR->ref_cnt++;		 			
	return 0;
}
int fs_close(int filds){
		
	if(open == 0)
		return -1;
	if(filds>=MAX_FILDES)
		return -1;
	if(filds<0)
		return -1;
	if(fildes[filds].used == 0)
		return -1;

	fildes[filds].used = 0;
	fildes[filds].offset=-1;
	fildes[filds].file = -1;
	
	int i=0;
	while(i<64){
		int parse =(DIR+i)->head;
		if(parse == fildes[filds].file){	
			break;}
		i++;}
	if (i ==64)
		return -1;
	DIR = (DIR+1);
	DIR->ref_cnt--;
	 return 0;}

int fs_create(char *name){
 
	if(name == NULL)
		return -1;
	if(strlen(name)>15)
		return -1;
	if(!strcmp(DIR->name,name))
		return -1;
	
	delete = 0;	
	return 0;}

int fs_delete(char *name){ 
	if(open == 1)
		return -1;
	if(strcmp(DIR->name,name))
		return -1;

	int i = 0;
	while(i<MAX_FILES){
        	if(!strcmp(DIR->name,name))
                	break;
                else
                        return -1;
                i++;
        }
	
	fildes[i].offset = -1;
	fildes[i].used = 0;
	fildes[i].file = -1;
	delete = 1;	
	
return 0;}

int fs_read(int fildes, void *buf, size_t nbyte){
	//This function attempts to read nbyte bytes of data from the file referenced by the descriptor fildes into the buffer pointed to by buf.	 
	if(fildes<0)
		return -1;
	if(fildes>MAX_FILDES)
		return -1;	
	//char buffer[nbyte];
	//memset(buffer,0,BLOCK_SIZE);
	//block_read((fildes/BLOCK_SIZE)+nbyte,buffer);
	//memcpy(buf,buffer,nbyte);
	
	 return 0;}

int fs_write (int fildes, void *buf, size_t nbyte){ 
	if(fildes<0)
		return -1;
	if(fildes>MAX_FILDES)
		return -1;

return 0;}
int fs_get_filesize(int fildes) {

	if(fildes>MAX_FILDES)
		return -1;
	if(fildes<0)
		return -1;
	
	return DIR->size;
	}

int fs_listfiles(char ***files){ return 0;}
int fs_lseek(int filds, off_t offset){ 

	struct file_descriptor *point;
	if(point == NULL)
		return -1;
	if(filds>MAX_FILDES)
		return -1;
	if(filds < 0)
		return -1;
	if(DIR->size < offset)
		return -1;
	if(offset<0)
		return -1;
	
	point->offset = offset;
	fildes[filds].offset = offset;
	return 0;}
int fs_truncate(int fildes, off_t length){ 
	if(fildes<0)
		return -1;
	if(fildes>MAX_FILDES)
		return -1;
	if(length<0)
		return -1;
	return 0;}
