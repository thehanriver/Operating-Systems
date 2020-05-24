# File System
## Idea
Implement a file system on top of a virtual disk and understand how file systems work

## Description
**Basically, trying to implement a file system on top of a Linux file system

I didn't understand how to use the super_block so trying to navigate around memory was especially challenging. I didn't pass/complete many of the functions properly but I managed to pass some.

make_fs: it first checks to see if make_disk is successful. Then it calls mount fs which loads the content of memory into the disk, which is the superblock, FAT, and directory entry.
It then callocs a size of 64 for DIR and a size of a block for FAT. Soon it calls unmount_fs which writes back everything from disk into memory.

mount_fs: first checks to see if open_disk works. It creates a char buffer the size of a block. Then it block_reads everything onto the 1st block for superblock. It does the same for FAT and for the directory entry.

umount_fs: this does the exact opposite but in a similar way to mount_fs. instead of block_read it does block_write with the same procedure. Instead of checking for open_disk though, it checks if disk is properly closed at the end.

fs_open: this opens a a file with the same name as the input. If name is NULL, has been previously deleted or if it's non existent it returns -1. It returns -1 when there are too many fd opens. Otherwise it looks for where the file lies in the directory and sets the appropriate values in fildes[i], i is the index of the file. In case, the file doesnt exist it returns a -1;

fs_close: this does the same except it checks if filds is good or not (good meaning not negative or not beyond MAX). In doing so, it resets everything to default inside the fildes array.

fs_lseek: gets the offset and puts it into offset of the fildes array;

other functions: either broken or only error checking is completed due to seg faulting so much that I just scrapped everything and restarted but didnt have time to finish. Sadly this project was very hard for my mind to wrap around.
