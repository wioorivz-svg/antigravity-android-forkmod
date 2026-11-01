// Antigravity Native Bionic Execution Hook
// Intercepts exec*() via LD_PRELOAD to seamlessly bypass Android noexec
// Zero external dependencies (-nostdlib -shared -fPIC).

#define AT_FDCWD -100

#define O_RDONLY    00000000
#define O_WRONLY    00000001
#define O_CREAT     00000100
#define O_TRUNC     00001000
#define O_CLOEXEC   02000000

#define SYS_close       57
#define SYS_read        63
#define SYS_write       64
#define SYS_fstat       80
#define SYS_fchmod      52
#define SYS_openat      56
#define SYS_sendfile    71
#define SYS_execve      221
#define SYS_mkdirat     34
#define SYS_newfstatat  79

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

static inline long sys_openat(int dirfd, const char *path, int flags, int mode) {
    register long x8 __asm__("x8") = SYS_openat;
    register long x0 __asm__("x0") = dirfd;
    register long x1 __asm__("x1") = (long)path;
    register long x2 __asm__("x2") = flags;
    register long x3 __asm__("x3") = mode;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0), "r"(x1), "r"(x2), "r"(x3) : "memory");
    return x0;
}

static inline long sys_close(int fd) {
    register long x8 __asm__("x8") = SYS_close;
    register long x0 __asm__("x0") = fd;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0) : "memory");
    return x0;
}

static inline long sys_read(int fd, void *buf, unsigned long count) {
    register long x8 __asm__("x8") = SYS_read;
    register long x0 __asm__("x0") = fd;
    register long x1 __asm__("x1") = (long)buf;
    register long x2 __asm__("x2") = count;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0), "r"(x1), "r"(x2) : "memory");
    return x0;
}

static inline long sys_fstat(int fd, struct kernel_stat *st) {
    register long x8 __asm__("x8") = SYS_fstat;
    register long x0 __asm__("x0") = fd;
    register long x1 __asm__("x1") = (long)st;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0), "r"(x1) : "memory");
    return x0;
}

static inline long sys_newfstatat(int dirfd, const char *path, struct kernel_stat *st, int flags) {
    register long x8 __asm__("x8") = SYS_newfstatat;
    register long x0 __asm__("x0") = dirfd;
    register long x1 __asm__("x1") = (long)path;
    register long x2 __asm__("x2") = (long)st;
    register long x3 __asm__("x3") = flags;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0), "r"(x1), "r"(x2), "r"(x3) : "memory");
    return x0;
}

static inline long sys_fchmod(int fd, int mode) {
    register long x8 __asm__("x8") = SYS_fchmod;
    register long x0 __asm__("x0") = fd;
    register long x1 __asm__("x1") = mode;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0), "r"(x1) : "memory");
    return x0;
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

static inline long sys_mkdirat(int dirfd, const char *path, int mode) {
    register long x8 __asm__("x8") = SYS_mkdirat;
    register long x0 __asm__("x0") = dirfd;
    register long x1 __asm__("x1") = (long)path;
    register long x2 __asm__("x2") = mode;
    __asm__ __volatile__("svc #0" : "=r"(x0) : "r"(x8), "r"(x0), "r"(x1), "r"(x2) : "memory");
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

static int str_cmp(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return (unsigned char)a[i] - (unsigned char)b[i];
}

static int str_ncmp(const char *a, const char *b, unsigned long n) {
    unsigned long i = 0;
    while (i < n && a[i] && b[i] && a[i] == b[i]) i++;
    if (i == n) return 0;
    return (unsigned char)a[i] - (unsigned char)b[i];
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

extern char **environ;

static inline char *const *resolve_envp(char *const envp[]) {
    return envp ? envp : environ;
}

static const char *find_env(char *const *envp, const char *key, unsigned long keylen) {
    envp = resolve_envp(envp);
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

// Stage any executable denied by kernel into private executable cache
static const char *stage_executable(const char *src_path, char *const *envp) {
    int src_fd = sys_openat(AT_FDCWD, src_path, O_RDONLY, 0);
    if (src_fd < 0) return 0;

    struct kernel_stat st;
    if (sys_fstat(src_fd, &st) < 0) {
        sys_close(src_fd);
        return 0;
    }

    const char *tmp = find_env(envp, "TMPDIR", 6);
    if (!tmp) tmp = "/data/user/0/com.agy/files/usr/tmp";

    sys_mkdirat(AT_FDCWD, tmp, 0700);

    const char *bname = get_basename(src_path);

    static char dest[512];
    int p = 0;
    while (tmp[p] && p < 240) { dest[p] = tmp[p]; p++; }
    dest[p++] = '/';
    dest[p++] = '.';
    dest[p++] = 'a';
    dest[p++] = 'g';
    dest[p++] = 'y';
    dest[p++] = '_';
    int b = 0;
    while (bname[b] && p < 500) { dest[p++] = bname[b++]; }
    dest[p] = '\0';

    // Cache check: reuse if size and mtime match
    struct kernel_stat dst_st;
    if (sys_newfstatat(AT_FDCWD, dest, &dst_st, 0) == 0) {
        if (dst_st.st_size == st.st_size && dst_st.st_mtime == st.st_mtime) {
            sys_close(src_fd);
            return dest;
        }
    }

    int dst_fd = sys_openat(AT_FDCWD, dest, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0755);
    if (dst_fd < 0) {
        const char fb[] = "/data/user/0/com.agy/cache/.agy_";
        p = 0;
        while (fb[p]) { dest[p] = fb[p]; p++; }
        b = 0;
        while (bname[b] && p < 500) { dest[p++] = bname[b++]; }
        dest[p] = '\0';
        dst_fd = sys_openat(AT_FDCWD, dest, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0755);
    }

    if (dst_fd < 0) {
        sys_close(src_fd);
        return 0;
    }

    sys_sendfile(dst_fd, src_fd, 0, (unsigned long)st.st_size);
    sys_fchmod(dst_fd, 0755);

    sys_close(src_fd);
    sys_close(dst_fd);

    return dest;
}

// Handle script shebangs (#!/bin/sh -> $PREFIX/bin/sh)
static int handle_shebang(const char *filename, char *const argv[], char *const envp[]) {
    int fd = sys_openat(AT_FDCWD, filename, O_RDONLY, 0);
    if (fd < 0) return 0;

    char buf[256];
    long bytes = sys_read(fd, buf, 255);
    sys_close(fd);

    if (bytes < 2 || buf[0] != '#' || buf[1] != '!') {
        return 0;
    }
    buf[bytes] = '\0';

    int end = 0;
    while (end < bytes && buf[end] != '\n' && buf[end] != '\r') end++;
    buf[end] = '\0';

    char *interp = buf + 2;
    while (*interp == ' ' || *interp == '\t') interp++;

    char *arg = 0;
    int s = 0;
    while (interp[s]) {
        if (interp[s] == ' ' || interp[s] == '\t') {
            interp[s] = '\0';
            arg = &interp[s + 1];
            while (*arg == ' ' || *arg == '\t') arg++;
            break;
        }
        s++;
    }

    const char *prefix = find_env(envp, "PREFIX", 6);
    if (!prefix) prefix = "/data/user/0/com.agy/files/usr";

    static char new_interp[512];
    int ip = 0;
    while (prefix[ip] && ip < 240) { new_interp[ip] = prefix[ip]; ip++; }

    if (str_cmp(interp, "/bin/sh") == 0 || str_cmp(interp, "/usr/bin/sh") == 0) {
        const char bin_sh[] = "/bin/sh";
        int k = 0; while (bin_sh[k]) { new_interp[ip++] = bin_sh[k++]; }
        new_interp[ip] = '\0';
    } else if (str_cmp(interp, "/bin/bash") == 0 || str_cmp(interp, "/usr/bin/bash") == 0) {
        const char bin_bash[] = "/bin/bash";
        int k = 0; while (bin_bash[k]) { new_interp[ip++] = bin_bash[k++]; }
        new_interp[ip] = '\0';
    } else if (str_cmp(interp, "/usr/bin/env") == 0) {
        const char bin_env[] = "/bin/env";
        int k = 0; while (bin_env[k]) { new_interp[ip++] = bin_env[k++]; }
        new_interp[ip] = '\0';
    } else if (str_ncmp(interp, "/usr/bin/", 9) == 0 || str_ncmp(interp, "/bin/", 5) == 0) {
        const char *tool = get_basename(interp);
        const char bin_slash[] = "/bin/";
        int k = 0; while (bin_slash[k]) { new_interp[ip++] = bin_slash[k++]; }
        k = 0; while (tool[k]) { new_interp[ip++] = tool[k++]; }
        new_interp[ip] = '\0';
    } else {
        return 0;
    }

    int argc = 0;
    while (argv && argv[argc]) argc++;

    char *new_argv[64];
    int na = 0;
    new_argv[na++] = new_interp;
    if (arg && *arg && na < 60) new_argv[na++] = arg;
    if (na < 60) new_argv[na++] = (char *)filename;
    for (int i = 1; i < argc && na < 60; i++) {
        new_argv[na++] = argv[i];
    }
    new_argv[na] = 0;

    return sys_execve(new_interp, new_argv, envp);
}

// Search PATH for a command without slash
static int search_and_exec(const char *file, char *const argv[], char *const envp[]) {
    char *const *ep = resolve_envp(envp);
    const char *path = find_env(ep, "PATH", 4);
    if (!path) path = "/data/user/0/com.agy/files/usr/bin:/system/bin";

    char candidate[512];
    int i = 0;
    while (path[i]) {
        int start = i;
        while (path[i] && path[i] != ':') i++;
        int len = i - start;

        if (len > 0 && len < 400) {
            int cp = 0;
            for (int k = 0; k < len; k++) candidate[cp++] = path[start + k];
            candidate[cp++] = '/';
            int f = 0;
            while (file[f] && cp < 510) candidate[cp++] = file[f++];
            candidate[cp] = '\0';

            // Check if file exists
            int fd = sys_openat(AT_FDCWD, candidate, O_RDONLY, 0);
            if (fd >= 0) {
                sys_close(fd);
                int ret = sys_execve(candidate, argv, ep);
                if (ret != -1) return ret;
                // If direct execve failed, try staging
                const char *staged = stage_executable(candidate, ep);
                if (staged) {
                    ret = sys_execve(staged, argv, ep);
                    if (ret != -1) return ret;
                }
            }
        }
        if (path[i] == ':') i++;
    }
    return -1;
}

__attribute__((visibility("default"))) int execve(const char *filename, char *const argv[], char *const envp[]) {
    if (!filename) return -1;
    char *const *ep = resolve_envp(envp);

    // If filename has no slashes, search PATH first!
    int has_slash = 0;
    for (int i = 0; filename[i]; i++) {
        if (filename[i] == '/') { has_slash = 1; break; }
    }
    if (!has_slash) {
        return search_and_exec(filename, argv, ep);
    }

    // Shebang handling
    handle_shebang(filename, argv, ep);

    // Direct execution
    int ret = sys_execve(filename, argv, ep);
    if (ret != -1) return ret;

    // Automatic transparent bypass: stage to executable partition and execute
    const char *staged = stage_executable(filename, ep);
    if (staged) {
        return sys_execve(staged, argv, ep);
    }

    return -1;
}

__attribute__((visibility("default"))) int execv(const char *path, char *const argv[]) {
    return execve(path, argv, environ);
}

__attribute__((visibility("default"))) int execvp(const char *file, char *const argv[]) {
    return execve(file, argv, environ);
}

__attribute__((visibility("default"))) int execvpe(const char *file, char *const argv[], char *const envp[]) {
    return execve(file, argv, resolve_envp(envp));
}
