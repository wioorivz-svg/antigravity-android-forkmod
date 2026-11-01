// Antigravity Freestanding Binary Executor for Android (AArch64)
// Bypasses Android's noexec mount restriction on /storage/emulated/0/
// Zero external dependencies (-nostdlib -static).

#define AT_FDCWD -100
#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_CREAT  00000100
#define O_TRUNC  00001000
#define O_CLOEXEC 02000000

#define SYS_write 64
#define SYS_close 57
#define SYS_fstat 80
#define SYS_fchmod 52
#define SYS_openat 56
#define SYS_sendfile 71
#define SYS_execve 221
#define SYS_exit 93

struct kernel_stat {
    unsigned long st_dev;
    unsigned long st_ino;
    unsigned int st_mode;
    unsigned int st_nlink;
    unsigned int st_uid;
    unsigned int st_gid;
    unsigned long st_rdev;
    unsigned long __pad1;
    long st_size;
    int st_blksize;
    int __pad2;
    long st_blocks;
    long st_atime;
    unsigned long st_atime_nsec;
    long st_mtime;
    unsigned long st_mtime_nsec;
    long st_ctime;
    unsigned long st_ctime_nsec;
    unsigned int __unused4;
    unsigned int __unused5;
};

static inline long sys_write(int fd, const void *buf, unsigned long count) {
    register long x8 __asm__("x8") = SYS_write;
    register long x0 __asm__("x0") = fd;
    register long x1 __asm__("x1") = (long)buf;
    register long x2 __asm__("x2") = count;
    __asm__ __volatile__("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");
    return x0;
}

static inline void sys_exit(int code) {
    register long x8 __asm__("x8") = SYS_exit;
    register long x0 __asm__("x0") = code;
    __asm__ __volatile__("svc #0" :: "r"(x8), "r"(x0) : "memory");
    while(1) {}
}

static inline int sys_openat(int dirfd, const char *path, int flags, int mode) {
    register long x8 __asm__("x8") = SYS_openat;
    register long x0 __asm__("x0") = dirfd;
    register long x1 __asm__("x1") = (long)path;
    register long x2 __asm__("x2") = flags;
    register long x3 __asm__("x3") = mode;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0), "r"(x1), "r"(x2), "r"(x3) : "memory");
    return (int)x0;
}

static inline int sys_fstat(int fd, struct kernel_stat *st) {
    register long x8 __asm__("x8") = SYS_fstat;
    register long x0 __asm__("x0") = fd;
    register long x1 __asm__("x1") = (long)st;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0), "r"(x1) : "memory");
    return (int)x0;
}

static inline int sys_fchmod(int fd, int mode) {
    register long x8 __asm__("x8") = SYS_fchmod;
    register long x0 __asm__("x0") = fd;
    register long x1 __asm__("x1") = mode;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0), "r"(x1) : "memory");
    return (int)x0;
}

static inline int sys_close(int fd) {
    register long x8 __asm__("x8") = SYS_close;
    register long x0 __asm__("x0") = fd;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0) : "memory");
    return (int)x0;
}

static inline long sys_sendfile(int out_fd, int in_fd, long *offset, unsigned long count) {
    register long x8 __asm__("x8") = SYS_sendfile;
    register long x0 __asm__("x0") = out_fd;
    register long x1 __asm__("x1") = in_fd;
    register long x2 __asm__("x2") = (long)offset;
    register long x3 __asm__("x3") = count;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0), "r"(x1), "r"(x2), "r"(x3) : "memory");
    return x0;
}

static inline int sys_execve(const char *path, char *const argv[], char *const envp[]) {
    register long x8 __asm__("x8") = SYS_execve;
    register long x0 __asm__("x0") = (long)path;
    register long x1 __asm__("x1") = (long)argv;
    register long x2 __asm__("x2") = (long)envp;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0), "r"(x1), "r"(x2) : "memory");
    return (int)x0;
}

static unsigned long str_len(const char *s) {
    unsigned long n = 0;
    while (s && s[n]) n++;
    return n;
}

static void print_err(const char *msg) {
    sys_write(2, msg, str_len(msg));
}

static const char *get_basename(const char *path) {
    const char *last = path;
    int i = 0;
    while (path && path[i]) {
        if (path[i] == '/') last = &path[i + 1];
        i++;
    }
    return last;
}

static const char *find_env(char **envp, const char *key, unsigned long keylen) {
    if (!envp) return 0;
    int i = 0;
    while (envp[i]) {
        const char *e = envp[i];
        unsigned long k = 0;
        while (k < keylen && e[k] && e[k] == key[k]) k++;
        if (k == keylen && e[k] == '=') return &e[k + 1];
        i++;
    }
    return 0;
}

void agy_main(unsigned long *sp) {
    int argc = (int)sp[0];
    char **argv = (char **)&sp[1];
    char **envp = (char **)&sp[argc + 2];

    if (argc < 2) {
        print_err("Antigravity Binary Executor (freestanding AArch64)\n");
        print_err("Usage: agy-exec <binary_path> [arguments...]\n");
        sys_exit(1);
    }

    const char *target = argv[1];

    // Try direct execution first. If permitted by kernel/mount, execve never returns.
    sys_execve(target, &argv[1], envp);

    // If execve returned, the filesystem is mounted noexec or direct execution is denied.
    // Proceed to stage the binary to an executable location.
    int src = sys_openat(AT_FDCWD, target, O_RDONLY, 0);
    if (src < 0) {
        print_err("agy-exec: cannot open target file\n");
        sys_exit(127);
    }

    struct kernel_stat st;
    if (sys_fstat(src, &st) < 0) {
        print_err("agy-exec: fstat failed\n");
        sys_close(src);
        sys_exit(127);
    }

    const char *candidates[5];
    int ncand = 0;
    const char *e_tmp = find_env(envp, "TMPDIR", 6);
    if (e_tmp) candidates[ncand++] = e_tmp;
    candidates[ncand++] = "/tmp";
    const char *e_home = find_env(envp, "HOME", 4);
    if (e_home) candidates[ncand++] = e_home;
    candidates[ncand++] = "/data/local/tmp";

    const char *bname = get_basename(target);
    char dest[256];
    int dst = -1;

    for (int c = 0; c < ncand; c++) {
        const char *base = candidates[c];
        int p = 0;
        while (base[p] && p < 200) { dest[p] = base[p]; p++; }
        dest[p++] = '/';
        dest[p++] = '.';
        dest[p++] = 'a';
        dest[p++] = 'g';
        dest[p++] = 'y';
        dest[p++] = '_';
        int b = 0;
        while (bname[b] && p < 254) { dest[p++] = bname[b++]; }
        dest[p] = '\0';

        dst = sys_openat(AT_FDCWD, dest, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0755);
        if (dst >= 0) break;
    }

    if (dst < 0) {
        print_err("agy-exec: failed to create executable staging file\n");
        sys_close(src);
        sys_exit(126);
    }

    sys_sendfile(dst, src, 0, (unsigned long)st.st_size);
    sys_fchmod(dst, 0755);
    sys_close(src);
    sys_close(dst);

    // Execute from executable partition with target arguments
    sys_execve(dest, &argv[1], envp);

    print_err("agy-exec: final execve failed\n");
    sys_exit(126);
}

__asm__(
".globl _start\n"
"_start:\n"
"    mov x0, sp\n"
"    b agy_main\n"
);
