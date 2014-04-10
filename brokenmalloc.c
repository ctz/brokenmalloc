/*
 * brokenmalloc.c
 *
 * See:
 *   https://github.com/ctz/brokenmalloc
 *
 * For documentation and license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <execinfo.h>
#include <assert.h>

static int report_fd = 4;
static uint32_t allocation_serial;

extern void *__libc_malloc(size_t size);

static int find_num(char *haystack, uint32_t needle)
{
    char *haystack_end = haystack + strlen(haystack);
    char *haystack_start = haystack;

    char needle_str[128];
    snprintf(needle_str, sizeof needle_str, "%lu", (unsigned long) needle);
    size_t needle_len = strlen(needle_str);

    while (1)
    {
        char * found = strstr(haystack, needle_str);
        char * found_end = found + needle_len;

        if (!found || found_end > haystack_end)
            return 0;
        if ((found == haystack_start || *(found - 1) == ',') &&
            (*found_end == '\0' || *found_end == ','))
            return 1;

        haystack = found + 1;
    }

    return 0;
}

static int should_fail_malloc(uint32_t serial)
{
    char * serials = getenv("MALLOC_FAILS");
    if (!serials)
        return 0;

    if (find_num(serials, serial))
        return 1;

    return 0;
}

void * malloc(size_t n)
{
    static int within_malloc = 0; /* not thread safe */

    if (n == 0)
        return NULL;

    if (within_malloc)
        goto complete;

    uint32_t this_malloc = __atomic_add_fetch(&allocation_serial, 1, __ATOMIC_RELEASE);
    if (should_fail_malloc(this_malloc))
    {
        printf("failed malloc %d by request\n", allocation_serial);
        return NULL;
    }

    if (getenv("MALLOC_REPORT"))
    {
        void *frames[32];
        within_malloc = 1;
        int nframes = backtrace(frames, 32);
        dprintf(report_fd, "MR: malloc(%zu) : %lu : ", n, (unsigned long) this_malloc);
        for (int i = 0; i < nframes; i++)
            dprintf(report_fd, "%p%s", frames[i], i == nframes - 1 ? "" : ",");
        dprintf(report_fd, "\n");
        within_malloc = 0;
    }

complete:
    return __libc_malloc(n);
}

