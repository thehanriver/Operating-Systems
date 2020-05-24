#include <stdio.h>
#include "disk.h"


int main(){

	char buffer[64] = "my_dir";
	make_fs(buffer);
	fs_open(buffer);
	return 0;
}
