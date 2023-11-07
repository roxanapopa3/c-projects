// SPDX-License-Identifier: BSD-3-Clause

#include <sys/stat.h>
#include <errno.h>
#include <internal/syscall.h>

int fstat(int fd, struct stat *st)
{
	if (fd < 0) {
		errno = EBADF;
		return -1;
	}
	int que = syscall(__NR_fstat, fd, st);
	if(que < 0) {
		errno = -que;
		return -1;
	}
	return que;
}
