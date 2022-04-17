# NEU-CS5600-CMPSYS
A crucial component of the course is the lab assignments. Five labs covering Setup and tools, C review, Shell, Virtual Mem, and File System.
## [Labs overview](https://naizhengtan.github.io/22spring/labs/)
### [Lab0:Setup and tools](https://naizhengtan.github.io/22spring/labtutorials/lab0/)
This lab will let you heavily use gcc that is a widely used compiler, make, and gdb. 
+ See a quick introduction of gcc [here](https://courses.cs.washington.edu/courses/cse451/99wi/Section/gccintro.html).
+ Make is a system to organize compilation. When you run make, the compilation system will look for a file named Makefile and executes the rules within. Here is a [quick introduction](https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/).
+ GDB is the GNU Debugger, a tool that every programmer should know how to use. Let see [tutorial](http://www.unknownroad.com/rtfm/gdbtut/gdbtoc.html).
### [Lab1:C review](https://naizhengtan.github.io/22spring/labtutorials/lab1/)
This lab will give you practice in the C programming language. For those who know C, this will reinforce your prior experience; for others, this is a good opportunity for you to learn C. The overall comfort you’ll gain (or reinforce) with C language and the Linux computing environment will be helpful for future labs.

Lab1 has three main sections:

    + Implementing a simple cipher.
    + Implementing a linked queue.
    + Implementing a ciphered queue using the cipher and the queue.
### [Lab2:Shell](https://naizhengtan.github.io/22spring/labtutorials/lab2/)
In this lab, you will learn how a shell is built. You will improve (or reinforce) your shell-using skills. You will also gain experience with C programming (you will interact with critical constructs, such as pointers and data structures, as well as standard idioms like linked lists). Along the way, you will use the fork(), a system call that we’ve intensively discussed in lectures.  
You may be interested in [a reasonable tutorial for Unix shells](https://linuxcommand.org/lc3_learning_the_shell.php).
### [lab3:Concurrent KV-store](https://naizhengtan.github.io/22spring/labtutorials/lab3/)
In this lab, you will write a simple concurrent Key-Value store (short as KV-store). A KV-store is a data storage that saves and retrieves data by keys.

Lab3 designs a toy KV-store that

    + contains at most a fixed number of keys (the number is defined by MAX_TABLE in lab3.h).
    + the key is a string.
    + each key has an associated value (another string).
    + the KV-store has three operations: read, write, and delete.
    + writes can insert new key-value pairs to the KV-store.
You will leverage multi-threading to accelerate the KV-store server. The server has a console thread (or main thread), a network listener thread, and multiple worker threads. we covered [monitors](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-monitors.pdf) from OSTEP.
### [lab4:cs5600 File System](https://naizhengtan.github.io/22spring/labtutorials/lab4/)
In Lab4, you will implement a simplified Unix-like file system, called fs5600 (CS5600 File System).

fs5600 uses the FUSE (File system in User SpacE) library, which allows us to implement a file system in the user space. This makes the development much easier than in the kernel space. FUSE exposes a few file system function interfaces that need to be instantiated and your task is to fill in these function bodies.

The file system that we will build is implemented as a user-level process. This file system’s storage will be a file (which is created by us called test.img) that lives in the normal file system of your devbox. Much of your code will treat this file as if it were a disk.

This entire arrangement (file system implemented in user space with arbitrary choice of storage) is due to software called [FUSE (File system in User SpacE)](https://github.com/libfuse/libfuse).

The FUSE driver registers a set of callbacks with the FUSE system (via libfuse and ultimately the FUSE kernel module); these callbacks are things like read, write, etc. A FUSE driver is associated with a particular directory, or mount point. The concept of mounting was explained in [OSTEP 39](https://pages.cs.wisc.edu/~remzi/OSTEP/file-intro.pdf) (see 39.17). Any I/O operations requested on files and directories under this mount point are dispatched by the kernel (via VFS, the FUSE kernel module, and libfuse) to the callbacks registered by the FUSE driver.

To recap all of the above: the file system user interacts with the file system roughly in this fashion:

+ When the file system user, Process A, makes a request to the system, such as listing all files in a directory via ls, the ls process issues one or more system calls (stat(), read(), etc.).

+ The kernel hands the system call to VFS.

+ VFS finds that the system call is referencing a file or directory that is managed by FUSE.

+ VFS then dispatches the request to FUSE, which dispatches it to the corresponding FUSE driver (which is where you will write your code).

+ The FUSE driver handles the request by interacting with the “disk”, which is implemented as an ordinary file. The FUSE driver then responds, and the responses go back through the chain.

![lab4-fuse-diagram](https://github.com/puppy-milo/NEU-CS5600-CMPSYS/blob/main/Photos/lab4-fuse-diagram.svg)




