/*
 *  NXFTP - ftpcomm.h
 *     Version 3.1
 *     by Eugene Hodges (ewhodges@eos.ncsu.edu)
 *
 *     Copyright 1992, 1993 by Eugene Hodges
 *     All Rights Reserved
 *
 *     Permission to use, copy, and distribute this software and text for
 *     non-commercial purposes and without fee is hereby granted, provided
 *     that this notice appears in all copies and that modified or
 *     incomplete versions are not distributed.
 *
 *     The author disclaims all warranties with regard to the software or
 *     text including all implied warranties of merchantability and fitness.
 *
 *     In no event shall the author or NCSU be liable for any special,
 *     indirect or cosequential damages or any damages whatsoever
 *     resulting from loss of use, data or profits, whether in an
 *     action of contract, negligence or other tortious action,
 *     arising out of or in connection with the use or performance
 *     of this software or text.
 *
 */


/* Kills ftp and closes the link in timer seconds if it is not
reset to zero.  Uses the SIGALARM routine from the signal library to
accomplish the "alarm clock" effect. */

void time_out(int timer);


/* The function calls ftp and connects to the site, checking for
error codes indicating successful/unsuccessful login.  It logs the user in by
issuing the ftp command 'user login_id passwd,' with the variables containing
the appropriate information.  After issuing this command it checks for a
successful login, returning a 0 if somehow unsuccessful.  The link (and ftp)
are killed if the login is unsuccessful. If the login takes too long (in other
words, if something goes wrong at the site), the time_out function, which is
called in several places, kills the link and the program.  This keeps the
program from just hanging.  If the login is successful, the current local
directory is determined by opening a second link and reading the listing from
the pipe.  The local directory is returned through the use of the variable
local_pwd. */

int ftp_connect_manual (char *site_name, char *login_id, char *passwd, char *local_pwd);


/* Reads the initial message printed each time by sites when
users log in.  The message is returned by use of the msg variable, one line at
a time, until the end of the message is reached.  When the message is
completely read, the function returns a 0 through its name on the next call to
read a line of the message. */

int initial_login_msg(char *msg);


/* The function builds the user's eos e-mail address by using
the logname command to obtain their login id.  Concatenate @eos.ncsu.edu to
the login id and the e-mail address is complete. */

char *eos_adr ();


/* This function changes ftp so that all gets (file transfers) 
will be sent to the user's home directory. */
 
char *cd_home_dir();


/* This function changes site directories.  It changes to the
directory named in dir_name.  It sends the command to ftp and checks for
error codes.  If the change in directories cannot be made, a NULL string is
returned.  Otherwise, the new pathname is returned.  The command 'pwd' is used
to flush the last of the output in the pipe so that it can be successfully
read. */

char *cd_remote(char *dir_name);


/* from K&R page 62 */
/* reverse:  reverse string s in place */
void reverse(char s[]);


/* from K&R page 64 */
/* itoa:  convert n to characters in s */
void itoa(int n, char s[]);


/* Returns a string containing the number indicating the number
of kilobytes free in the user's quota */ 

char *quota();


/* This function gets the file specified in file_name from the
site.  It checks for error codes and returns a 0 if the get is for some reason
unsuccessful.  It returns 1 if successful. */

int get(char *file_name);


/* sets the file transfer type to binary and returns a 1 if successful or 0
otherwise through the function name */

int binary();


/* sets the file transfer type to ascii (text) and returns a 1 if successful 
or 0 otherwise through the function name */

int ascii();

/* This function checks to see if ftp is connected to a site. 
If it is, it calls clear_lists to free both of the doubly linked lists.  It
then sends ftp the 'quit' command and closes the link.  The variable
connected, which keeps track of whether or not ftp is connected to a site, is
set to not connected (0). */

void ftp_exit();

char *cd_local(char *path);

void toggle_hash();
