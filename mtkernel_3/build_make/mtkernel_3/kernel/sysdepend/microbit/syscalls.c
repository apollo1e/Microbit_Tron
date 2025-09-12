#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int _write(int file, char *ptr, int len) {
    // Implement this if you're using UART
    return len;
}

int _read(int file, char *ptr, int len) {
    return 0;
}

int _close(int file) {
    return -1;
}

int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) {
    return 1;
}

int _lseek(int file, int ptr, int dir) {
    return 0;
}

void *_sbrk(ptrdiff_t incr) {
    extern char end;       // defined by the linker
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &end;
    }

    prev_heap_end = heap_end;
    heap_end += incr;
    return (void *)prev_heap_end;
}
