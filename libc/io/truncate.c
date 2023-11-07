// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

int truncate(const char *path, off_t length)
{
	int que = syscall(__NR_truncate, path, length);
	if(que < 0) {
		errno = -que;
		return -1;
	}
	return que;
}
