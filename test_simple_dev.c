#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

static void hexdump(unsigned int addr, unsigned char *p, int len)
{
    char ascii[20];
    char ch;
    int ix;

    while (len > 0)
    {
        printf("%08X:", addr);
        memset(ascii, 0, sizeof(ascii));
        for (ix = 0; ix < 16 && len > 0; ix++)
        {
            printf(" %02X", *p);
            ch = *p & 0x7F;
            ascii[ix] = ((ch < ' ') || (0x7F == ch)) ? '.' : ch;
            p++;
            len--;
            addr++;
        }
        printf(" [%-16s]\n", ascii);
    }
}

static void dump_file(int fd, size_t size)
{
    char *bfr;
    ssize_t cnt;

    bfr = (char *)malloc(size);
    if (!bfr)
    {
        printf("No buffer for %d bytes\n", size);
        close(fd);
        return;
    }

    lseek(fd, 0, SEEK_SET);
    cnt = read(fd, bfr, size);
    if (cnt < 0)
    {
        printf("Failed to read %d bytes: %s\n", size, strerror(errno));
        free(bfr);
        return;
    }
    // let's limit the output of the hexdump to the first 256 bytes
    if (size > 256)
    {
        size = 256;
    }
    hexdump(0, bfr, size);
    free(bfr);
}

int main(int argc, char *argv[])
{
    int fd;
    const char *simple_path = "/dev/simple";
    off_t pos;
    size_t fsize;
    const char *wdata = "Something else to write";
    size_t wlen = strlen(wdata) + 1;
    ssize_t cnt;
    int rc;

    fd = open(simple_path, O_RDWR);
    if (fd < 0)
    {
        printf("open(%s) failed: %s\n", simple_path, strerror(errno));
        return -1;
    }

    pos = lseek(fd, 0, SEEK_END);
    if (pos < 0)
    {
        printf("lseek(0, SEEK_END) failed: %s\n", strerror(errno));
    }

    fsize = pos;
    printf("File size of %s is %d\n", simple_path, fsize);

    printf("Initial file content:\n");
    dump_file(fd, fsize);


    lseek(fd, 0, SEEK_SET);
    cnt = write(fd, wdata, wlen);
    if (cnt < 0)
    {
        printf("write(%d bytes) failed: %s\n", wlen, strerror(errno));
        close(fd);
        return -1;
    }

    printf("After writing \"%s\" over the existing data:\n", wdata);
    dump_file(fd, fsize);

    printf("Press ENTER to exit\n");
    fgetc(stdin);
    if (close(fd))
    {
        printf("close(%d) failed: %s\n", fd, strerror(errno));
        return -1;
    }
    return 0;
}
