/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef __TIME_H__
#define __TIME_H__	1

#include <internal/types.h>

struct timespec {
	time_t tv_sec;
	long int tv_nsec;
};

#endif
