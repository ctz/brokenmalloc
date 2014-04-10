brokenmalloc
============

brokenmalloc is a simple malloc wrapper which can be made to fail on demand.

Building
--------

A trivial makefile is supplied.  It may only be compatible with glibc, I do not know if
other libcs provide `__libc_malloc` hooks.

Usage
-----

Say your program is `./prog` and you have built brokenmalloc as `./brokenmalloc.so`.
First, get a log of all allocations a run of your program does:

    $ LD_PRELOAD="./brokenmalloc.so" MALLOC_REPORT=yes ./prog 4> malloc.log

`MALLOC_REPORT` makes brokenmalloc write a line to file descriptor 4 for each malloc
call.  An example line is:

    MR: malloc(104) : 1 : 0x7f9cc96edb6c,0x403f17,0x404576,0x40cfda,0x40bd84,0x401dd1...

Here, `MR: ` is a header, `malloc` is the allocator function which was called (realloc
may appear in a future version), `104` is the size in bytes of the allocation, `1` is
the serial number of the allocation, and `0x7f9cc96edb6c,...` is a stack trace of the
call in the form of a list of addresses.

You ask gdb (or `addr2line`) to tell you about this address:

    (gdb) info symbol 0x403f17
    default_malloc_ex + 35 in section .text

`lookup-syms.py` is a python script which automates this.  Give it the name of your
binary and pipe in your `malloc.log`:

    $ python lookup-syms.py ./prog < malloclog 
    malloc(104) 1 ?? @ ??:0 -> default_malloc_ex @ /home/jbp/openssl/crypto/mem.c:79
	                        -> CRYPTO_malloc @ /home/jbp/openssl/crypto/mem.c:307
							-> EVP_DigestInit_ex @ /home/jbp/openssl/crypto/evp/digest.c:206
							-> ssleay_rand_add @ /home/jbp/openssl/crypto/rand/md_rand.c:280
							-> RAND_add @ /home/jbp/openssl/crypto/rand/rand_lib.c:158
							-> RAND_poll @ /home/jbp/openssl/crypto/rand/rand_unix.c:394
							-> ssleay_rand_bytes @ /home/jbp/openssl/crypto/rand/md_rand.c:439
							-> ssleay_rand_pseudo_bytes @ /home/jbp/openssl/crypto/rand/md_rand.c:617
							-> RAND_pseudo_bytes @ /home/jbp/openssl/crypto/rand/rand_lib.c:173
							-> main @ /home/jbp/openssl/hax/simple-randtest.c:7
							-> ?? @ ??:0
							-> _start @ ??:?
    (...)

Now: to fail an allocation.  brokenmalloc will look for an environment variable `MALLOC_FAILS`
which should contain a comma-separated list of allocations to fail, by serial number.  So
`MALLOC_FAILS=1` always fails the first allocation, `MALLOC_FAILS=3,4` fails the third and
fourth, etc.

    $ LD_PRELOAD=./brokenmalloc.so MALLOC_FAILS=57 ./prog
    failed malloc 57 by request
    weak randomness: 00000000000000000000000000000000000000000000000000000000


