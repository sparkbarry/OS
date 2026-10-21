# Lab 5

Initials: C, Y. Initial value: `67 + 89 / 10.0 = 75.9`.

## Compile and run

On Linux (kernel 4.x or newer), or for local macOS checks:

```sh
pwd
uname -sr
cc -std=c11 -Wall -Wextra -Wpedantic lab5_a.c -o lab5_a
cc -std=c11 -Wall -Wextra -Wpedantic lab5_b.c -o lab5_b
./lab5_a 7 2.5
./lab5_b 7 2.5
```

Run these commands from the `lab5` directory. Each run can take up to roughly
42 seconds for seven values because the six intervals are independently chosen
from 0 through 6999 milliseconds. Expected printed values:

```text
75.900002
78.400002
80.900002
83.400002
85.900002
88.400002
90.900002
```

The small difference from decimal 75.9 is normal: the assignment requires
float elements, even though Part A stores them in a double buffer.

## Part A

The parent creates an anonymous POSIX shared mapping of exactly
`BUF_SZ * sizeof(double)` bytes, where `BUF_SZ` is 5. A separate shared mapping
contains two lock-free atomic counters. There is no named shared-memory file,
so no unlink is needed. Both processes unmap their mappings before exiting.

The child inherits n and d through fork, produces each float, and stores it in
slot `produced % BUF_SZ`. The parent reads slot `consumed % BUF_SZ`, prints the
value, and flushes stdout. Monotonic counters distinguish a full queue from an
empty one and allow all five slots to be used. The producer waits while
`produced - consumed == BUF_SZ`; the consumer waits while the counters match.

Release/acquire atomic operations ensure a slot is written before the consumer
reads it, and read before the producer reuses it. They are individual loads and
stores, not spin locks. Each wait sleeps for 1 ms, so arrival detection can have
about 1 ms of polling delay plus operating-system scheduling delay. No semaphore,
mutex, or spin lock is used. The program rejects platforms whose unsigned
atomics are not lock-free.

## Part B

An ordinary pipe carries one float at a time. Each process closes the unused
pipe end. The parent blocks in read until data arrives, then prints and flushes
it immediately. The transfer routine handles interrupted and partial reads and
writes. The parent waits for the child only after receiving all n elements.

## Verification and submission

The source files were compiled and run on macOS; `local_validation.json` records
actual output and arrival times. Tests cover seven elements (including buffer
wraparound), a negative increment, a zero increment, and invalid inputs.

`lab5.pdf` is a local report draft. **It does not yet contain the required Linux
terminal screenshots.** Before submission, run the commands above on Linux
kernel 4.x or newer and capture the actual terminal showing the directory,
compile/run commands, and output for both parts. Add those screenshots to the
PDF, and review the course's rules concerning outside assistance.

Submit `lab5_a.c`, `lab5_b.c`, and the completed `lab5.pdf` individually.
No Makefile is required for these user-space programs.
