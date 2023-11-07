#include <time.h>
#include <internal/syscall.h>
#include <unistd.h>

int nanosleep(const struct timespec *req, struct timespec *rem){
    return syscall(__NR_clock_nanosleep, 0, 0, req, rem);
}
