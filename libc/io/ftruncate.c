// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

int ftruncate(int fd, off_t length)
{
	if(fd < 0) {
		errno = EBADF;
		return -1;
	}
	int que = syscall(__NR_ftruncate, fd, length);
	if(que < 0) {
		errno = -que;
		return -1;
	}
	return que;
}
