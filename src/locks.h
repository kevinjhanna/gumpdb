#ifndef LOCKS_H
#define LOCKS_H

#include <unistd.h>

#include <fcntl.h>
#include <sys/types.h>

bool set_lock(int fd, int cmd, int type, off_t offset, int whence, off_t len);

#endif
