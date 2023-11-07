// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

off_t lseek(int fd, off_t offset, int whence)
{
	if(fd < 0) {
		errno = EBADF;
		return -1;
	}
	if(offset < 0 || whence < 0) {
		errno = EINVAL;
		return -1;
	}
	off_t que = syscall(__NR_lseek, fd, offset, whence);
	if(que < 0) {
		errno = -que;
		return -1;
	}
	return que;
}
