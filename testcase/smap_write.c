#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 128
#define DEVICE_PATH "/dev/smap-smep"
#define SMAP_WRITE 200

int main(int argc, char **argv)
{
    int fd;
    int ret;
	char *map;

    fd = open(DEVICE_PATH, O_RDWR);
    if(fd < 0){
        perror("open failed");
        exit(1);
    }

	map = mmap(NULL , 1024, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if (!map) {
		printf("map is NULL\n");
		return -1;
	}else {
        printf("US: addr = 0x%lx \n", (unsigned long)map);
    }

	memset(map, 0, sizeof(BUF_SIZE));

	mlockall(MCL_CURRENT);

    ioctl(fd, SMAP_WRITE, map);

    close(fd);
	return 0;
}
