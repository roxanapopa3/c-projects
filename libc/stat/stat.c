// SPDX-License-Identifier: BSD-3-Clause

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <internal/syscall.h>

int stat(const char *restrict path, struct stat *restrict buf)
{
	int que = syscall(__NR_stat, path, buf);
	if(que < 0) {
		errno = -que;
		return -1;
	}
	return que;
}
