# xv6_operating_system
# CS 333 Introduction to Operating Systems

**Course Coordinator:** Jesse Chaney 

## Course Description:

Introduction to the principles of operating systems and concurrent programming. Operating system services, file systems, resource management, synchronization. The concept of a process; process cooperation and interference. Introduction to networks, and protection and security. Examples drawn from one or more modern operating systems. Programming projects, including concurrent programming.

**Prerequisites:** CS 302

## Goals:

- To expose students to the fundamental architectural and algorithmic concepts involved in the design and implementation of modern operating systems. This will include the design and coding of concurrent programs.
- Upon the successful completion of this class, students will be able to:
  - Read technical material that refers to operating system concepts.
  - Write system-level code.
  - Apply synchronization-related concerns when implementing code.
  - Continue on to study advanced operating systems.
  - Describe observed application runtime behavior.

## Textbooks:

- **Operating Systems: Three Easy Pieces**, *Remzi Arpaci-Dusseau and Andrea Arpaci Dusseau*, 2016
- **Xv6: a simple, Unix-like teaching operating system**, *Russ Cox*, 2014

## References:

- **The C Programming Language, 2nd ed.**, *Kernighan and Ritchie*, Prentice Hall, 1998. ISBN 0131103628
- **Practical Guide to Linux Commands, Editors, and Shell Programming, A (2nd Edition)** ISBN 0131367366

## Major Topics:

- Processes
- Inter-process communication
- Concurrent Programming
- Scheduling
- Memory Management
- File Systems
- Synchronization
- Device Management
- Introductions to Protection & Security and to Networking

## Problem Analysis:

Students complete a variety of projects that require problem analysis, design, coding, and testing. For example, students might design a shell program based on knowledge of desired shell behavior, or design and add synchronization to a multithreaded code. Specific projects vary for each course offering.

## Solution Design:

One or more projects require solution design, for example, changing existing code to remove a race condition.


-----------------------------------------------------------------

# Write functions in xv6

- `vikalloc_set_min(size_t size)`: Sets the minimum memory allocation size and returns the current minimum size.

- `vikalloc_set_algorithm(vikalloc_fit_algorithm_t algorithm)`: Configures the memory allocation algorithm and logs the choice in verbose mode.

- `vikalloc_set_verbose(uint8_t verbosity)`: Enables or disables verbose mode for logging messages.

- `vikalloc_set_log(FILE *stream)`: Sets the log stream for message output.

- `vikalloc(size_t size)`: Allocates memory using various allocation algorithms (e.g., FIRST_FIT, BEST_FIT), reusing or creating blocks as needed and handling block splitting.

- `coalesce(mem_block_t *curr)`: Combines adjacent free memory blocks into larger blocks through coalescing.

- `vikfree(void *ptr)`: Frees previously allocated memory blocks, coalescing adjacent free blocks if necessary.

- `vikalloc_reset(void)`: Deallocates all allocated memory, effectively resetting vikalloc.

- `vikcalloc(size_t nmemb, size_t size)`: Allocates memory for arrays, initializing elements to zero.

- `vikrealloc(void *ptr, size_t size)`: Reallocates memory for a previously allocated block, extending it or allocating a new block and copying data.

- `vikstrdup(const char *s)`: Allocates memory for a duplicated string, copying the input string and returning a pointer to the duplicate.

