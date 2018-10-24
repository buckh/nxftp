/* Link module, Marshall Brain */

/* This module allows a program to form links to other seperately
   executing programs and communicate with them. Links can be
   opened and closed, and the program using this unit can 
   write to and read from the other program over the link. */
/* The structure declaration was modified by E. W. Hodges to be a typedef.
   All else (including comments) are those of Marshall Brain. */

#include <stdio.h>
#include <strings.h>
#include <signal.h>
#include <sys/ioctl.h>

#ifndef LINK_H
#define LINK_H

/* This structure was written by Marshall Brain.  The structure includes two
pipe file descriptors, one which will be used to read and one to write, the
process id of the child, and two FILE streams, one in and one out.  The pipe
file descriptors and the FILE streams will be connected in the program so as
to establish a link (read-write) between the parent and child processes.  */
typedef struct link_handle {
                           int pipefd1[2],pipefd2[2];
                           int pid;
                           FILE *fpin,*fpout;
                           }Link_Handle;
#endif

extern link_open(struct link_handle *lnk, char name[], char param[], char param2[], 
   char param3[], char param4[]);
/* open a link to another program named name, passing up to 3 params
   to the program if desired. This routine will execute name
   in parallel and you can start communicating with it with
   link_read and link_write.*/

extern link_close(struct link_handle *lnk);
/* Close the link to a program that has terminated. Use link_kill
   if the program needs to be terminated as well.*/

extern link_read(struct link_handle *lnk,char s[]);
/* read from the program started with link_open.*/

extern link_input_waiting(struct link_handle *l);
/* returns the number of bytes waiting in the input buffer; if 0, then 
   link_read will block if it is called */

extern link_write_char(struct link_handle *lnk,char c);
/* write a char, without a newline, to the program.*/
  
extern link_write(struct link_handle *lnk,char s[]);
/* write a string to the program, with a newline.*/

extern link_kill(struct link_handle *lnk);
/*kill the program and close the link. If the program has terminated
  on its own use link_close instead.*/
