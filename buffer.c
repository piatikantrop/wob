#define _POSIX_C_SOURCE 200112L

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include "buffer.h"

int
wob_shm_create()
{
	int shmid = -1;
	char shm_name[NAME_MAX];
	for (unsigned char i = 0; i < UCHAR_MAX; ++i) {
		if (snprintf(shm_name, NAME_MAX, "/wob-%hhu", i) >= NAME_MAX) {
			break;
		}
		shmid = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (shmid > 0 || errno != EEXIST) {
			break;
		}
	}

	if (shmid < 0) {
		perror("shm_open");
		return -1;
	}

	if (shm_unlink(shm_name) != 0) {
		perror("shm_unlink");
		return -1;
	}

	return shmid;
}

void *
wob_shm_alloc(const int shmid, const size_t size)
{
	if (ftruncate(shmid, size) != 0) {
		perror("ftruncate");
		return NULL;
	}

	void *buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
	if (buffer == MAP_FAILED) {
		perror("mmap");
		return NULL;
	}

	return buffer;
}
