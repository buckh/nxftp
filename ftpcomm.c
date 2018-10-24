/*
 *  NXFTP - ftpcomm.c
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

/* must be changed for each system accordingly (just the site name, leave the
   %s alone) */
/* #define EMAIL_ID   "%s@your.host.name" */

#define EMAIL_ID   "%s@csc.liv.ac.uk"
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include "ftpcomm.h"
#include "global.h"

#ifdef __hpux
#define getwd(arg)      getcwd(arg, (size_t) 199)
#endif

/* keeps track of whether or not ftp is connected to a site */
static int connected;

static char *error_msg[100];

/*
 * Function:  void time_out(int timer)
 *              
 * Description:  Kills ftp and closes the link in timer seconds if it is not
 * reset to zero.  Uses the SIGALARM routine from the signal library to
 * accomplish the "alarm clock" effect.
 *           
 * Inputs:  The number of seconds (timer) in which the link should be killed.
 *
 * Outputs:  None.
 *
 */
void time_out(int timer)
   {
   static int i=0;  /* counts the number of times the function is called; used
                    so that the link is not killed the first time the function
                    is called */
   signal(SIGALRM, time_out);
   alarm(timer);

   /* if alarm clock reset by a call with timer==0, reset counter (i) to 0 */   
   if (timer==0)
      {
      i=0;
      return;
      }

   /* if alarm clock expires, kill the link */
   if (i==1)
      {
      link_kill(&l);
      error(error_msg);
      i=0;
      }
   i++;		
   }

/*
 * Function: int ftp_connect_manual (char *site_name, char *login_id, char 
 *                                   *passwd, char *local_pwd) 
 *              
 * Description:  The function calls ftp and connects to the site, checking for
 * error codes indicating successful/unsuccessful login.  It logs the user in by
 * issuing the ftp command 'user login_id passwd,' with the variables containing
 * the appropriate information.  After issuing this command it checks for a
 * successful login, returning a 0 if somehow unsuccessful.  The link (and ftp)
 * are killed if the login is unsuccessful. If the login takes too long (in other
 * words, if something goes wrong at the site), the time_out function, which is
 * called in several places, kills the link and the program.  This keeps the
 * program from just hanging.  If the login is successful, the current local
 * directory is determined by opening a second link and reading the listing from
 * the pipe.  The local directory is returned through the use of the variable
 * local_pwd.  
 *           
 * Inputs:  site_name is the name of the site for connection
 *          login_id is the user's login for the site
 *          passwd is the user's password for the site
 *          local_pwd is used to store the pathname for the current local dir
 *
 * Outputs:  local_pwd contains the pathname for the current local dir
 *           returns 0 or 1 if the login is unsuccessful or successful,
 *           respectively
 *
 * Restrictions:  The site_name, login_id, and passwd variables need to
 *                contain some sort of string other than the empty string.
 *
 */
int ftp_connect_manual (char *site_name, char *login_id, char *passwd, char *local_pwd)
 {
   char s[200],         /* holds text read from link */
        jnk[200],       /* holds text read from link that is not needed */
        login[50],      /* holds the user login password command */
        err_msg[1000],  /* holds error message */
        *temp;
   int i;               /* loop counter */

   /* initialize as not connected */
   connected=0;

   /* set buffer to 0 to prevent problems writing to the link */
   setbuf(stdout,NULL);

   /* set the timer and attempt to connect to the site */
   time_out (25);
   link_open(&l,"ftp","-v","-n",site_name,(char *)NULL);
   link_read(&l,s);

   strcpy(error_msg, "Unable to connect to site or site unknown.\n");
   /* clear the timer and check the error codes for unsuccessful login; return
   0 in that case */
   time_out (0);
   if ((strstr(s,"Connected")==NULL)||(strstr(s,"unreachable")!=NULL))
      {
      link_close(&l);
      error("Unable to connect to site or site unknown.\n");
      return (0);
      }

   link_read(&l,s);

   if (strstr(s,"421 ")!=NULL)
      {
      link_kill(&l);
      error("Service not available, remote server has closed connection.\n");
      return (0);
      }

   if (strstr(s,"220-")!=NULL)
      {
      while (strstr(s,"220-")!=NULL)
         link_read(&l,s);

      while (strstr(s,"220 ")==NULL)
         link_read(&l,s);
      }
   
   if (strstr(s,"220 ")==NULL)
      {
      link_kill(&l);
      error("An error has occured in reading the site's opening message.\n");
      return (0);
      }

   /* build the user login password command string */
   sprintf(login, "user %s %s", login_id, passwd);

   /* set timer and attempt to login the user to the site */
   time_out(25);
   link_write(&l,login);
   link_read(&l,s);

   /* clear the timer and check the error codes for login condition */
   time_out(0);

   /* extra line(s) after the '220 ' */
   if ((strstr(s,"530 ")==NULL)&&(strstr(s,"331 ")==NULL)) 
      link_read(&l,s);

   if ((strstr(s,"530 ")==NULL)&&(strstr(s,"331 ")==NULL)) 
      link_read(&l,s);

   if (strstr(s,"530 ")!=NULL)
      {
      /* close link if unsuccessful login */
      error(s);
      link_kill(&l);   
      return (0);
      }
      
   if (strstr(s,"530-")!=NULL)
      {
      /* close link if unsuccessful login */
      strcat(err_msg, s);
      while (strstr(s,"530-")!=NULL)
         {
         link_read(&l,s);
         strcat(err_msg, s);
         }
      error(err_msg);
      link_kill(&l);   
      return (0);
      }

   if (strstr(s,"331 ")!=NULL) 
      {
      /* login was successful */
      time_out(0);
      connected=1;

      getwd(local_pwd);

      return (1);
      }
   else 
      {
      /* if unknown error code, close the link and return a 0 to indicate
      unsuccessful login */
      error("An unknown error has occured.\n");
      link_write(&l,"quit");
      link_close(&l);
      return (0);
      }

   }

/*
 * Function:  int initial_login_msg(char *msg)
 *              
 * Description:  Reads the initial message printed each time by sites when
 * users log in.  The message is returned by use of the msg variable, one line at
 * a time, until the end of the message is reached.  When the message is
 * completely read, the function returns a 0 through its name on the next call to
 * read a line of the message.
 *           
 * Inputs:  msg is a pointer to some fixed array of at least length 80 which
 *          is used to store a line of the initial message
 *
 * Outputs:  The line of text is output through the use of the array msg
 *           points to.
 *
 * Restrictions:  The array pointed to by msg must be at least 80 characters
 *                long.
 *
 */
int initial_login_msg(char *msg)
   {
   static msg_chk=1;    /* used to give the status of the next line of the
                        initial message */

   /* if there is another line, read it */
   if (msg_chk)
      {
      /* check for error message */
      link_read(&l,msg);
      if ((strstr(msg,"421 ")!=NULL) && (strstr(msg,"421-")!=NULL))
         return (2);

      /* if msg contains '230-,' there is at least one more line; if '230 ,'
         that is the last line */
      if (strstr(msg,"230-")!=NULL)
         return(1);
      if (strstr(msg,"550-")!=NULL)
         return(1);
      if (strstr(msg,"550 ")!=NULL)
         return(2);
      if (strstr(msg,"230 ")!=NULL)
         {
         msg_chk=0;
         return(1);
         }
      else
         return(1);
      }

   /* since there is not another line, return in msg the empty string */
   *msg='\0';
   msg_chk=1;
   return(0);
   }

/*
 * Function:  char *eos_adr()
 *              
 * Description:  The function builds the user's eos e-mail address by using
 *               the logname command to obtain their login id.  Concatenate 
 *               @eos.ncsu.edu to the login id and the e-mail address is 
 *               complete. 
 *           
 * Inputs:  None.
 *
 * Outputs:  The eos e-mail address is returned through the name of the
 *           function.
 *
 * Restrictions:  The user must be logged into an eos terminal for the e-mail
 *                address to be valid.
 */
char *eos_adr()
   {
   /* contains the e-mail address for the user in
      the form login@wherever, where login is the
      user's login id */
   static char mail_id[25];
   static struct passwd *pwent = NULL; 

   if (!pwent)
      {
      pwent = getpwuid(getuid());
      sprintf(mail_id, EMAIL_ID, pwent->pw_name);
      }

   /* return mail_id to the caller */
   return (mail_id);
   }

/*
 * Function:  char *cd_home_dir()
 *              
 * Description:  This function changes ftp so that all gets (file transfers) 
 * will be sent to the user's home directory.
 *           
 * Inputs:  None.
 *
 * Outputs:  The local pathname to the user's home directory is returned
 *           through the function name.
 *
 */
char *cd_home_dir()
   {
   char s[200],         /* holds output of the pwd command, which is not
                        needed */
        dir_msg[200],   /* holds the string containing the local pathname to
                        the user's home directory */
        *dir_path;      /* the local pathname */

   int i=0;

#ifdef DEBUG
printf("entering cd_home_dir()\n");
#endif 

   /* get rid of any trash still hanging around */
   while(link_input_waiting(&l))
       link_read(&l,s);

   /* send the command 'lcd ' to ftp and then 'pwd' to flush the ouput from
   lcd out of the pipe; read all output from the pipes */
   link_write(&l,"lcd");
   link_write(&l,"pwd");
/*   link_write(&l,"pwd"); */
   link_read(&l,dir_msg);
/* -- What's going on here? -- Rik 
   link_read(&l,s);
*/

   while(link_input_waiting(&l))
       link_read(&l,s);

   /* modify the string to contain only the local pathname and return it
   through the function name */
   dir_path=strstr(dir_msg,"/");

#ifdef DEBUG
printf("leaving cd_home_dir()\n");
#endif 

   return (dir_path);
   }

char *cd_local(char *path)
   {
   char s[200],         /* holds output of the pwd command, which is not
                        needed */
        dir_msg[200],   /* holds the string containing the local pathname to
                        the user's home directory */
        dir_path[200];      /* the local pathname */
   int i=0;

#ifdef DEBUG
printf("entering cd_local_dir()\n");
#endif 

   /* get rid of any trash still hanging around */
   while(link_input_waiting(&l))
       link_read(&l,s);

   sprintf(dir_path, "lcd %s", path);

   /* send the command 'lcd ' to ftp and then 'pwd' to flush the ouput from
   lcd out of the pipe; read all output from the pipes */
   link_write(&l,dir_path);
   link_write(&l,"pwd");
   link_read(&l,dir_msg);
   link_read(&l,s);
   if ((strstr(dir_msg,"/")==NULL)||(strstr(dir_msg,"No such")!=NULL)
      ||(strstr(dir_msg,"Permission denied")!=NULL))
      return ("\0");

   while(link_input_waiting(&l))
       link_read(&l,s);

#ifdef DEBUG
printf("leaving cd_local_dir()\n");
#endif 

   /* kill newline */
   dir_msg[strlen(dir_msg)-1] = '\0';
 
   /* modify the string to contain only the local pathname and return it
   through the function name */
   return (strstr(dir_msg,"/"));
   }

/*
 * Function:  char *cd_remote(char *dir_name)
 *              
 * Description:  This function changes site directories.  It changes to the
 * directory named in dir_name.  It sends the command to ftp and checks for
 * error codes.  If the change in directories cannot be made, a NULL string is
 * returned.  Otherwise, the new pathname is returned.  The command 'pwd' is used
 * to flush the last of the output in the pipe so that it can be successfully
 * read.
 *           
 * Inputs:  dir_name is the name of the directory to be changed to 
 *
 * Outputs:  It returns the new site path to the current site directory if
 *           successful and returns a NULL string if unsuccessful.
 *
 */
char *cd_remote(char *dir_name)
   {
   char cd_name[30],       /* the name of the dir to change into */
        s[200],            /* output read from the pipe */
        *bad_char,*temp;   /* the position of undesired characters in the dir
                           name */
   int i=0;

#ifdef DEBUG
printf("entering cd_remote_dir()\n");
#endif 

   /* get rid of any trash still hanging around */
   while(link_input_waiting(&l))
       link_read(&l,s);

   /* remove unwanted characters, such as  / and @, from the end of
   the directory name */
   bad_char=strstr(dir_name,"/");
   if (bad_char != (char *)NULL)
      *bad_char='\0';

   bad_char=strstr(dir_name,"@");
   if (bad_char != (char *)NULL)
      *bad_char='\0';

   /* if it is a 'cd ..', check to see if the current site directory is the root;
   if so, return a NULL string */
   if (strstr(dir_name,"..") != NULL)
      {
fprintf(stderr, "FTPCOMM: %d\n", __LINE__);
      link_write(&l,"pwd");
      link_read(&l,s);
      if (strstr(s,"\"/\"") != NULL)
         return ("");
      }

   /* build command to change directories and put in cd_name */
   sprintf(cd_name,"cd %s",dir_name);

   /* send the command to ftp */ 
   link_write(&l,cd_name);
   link_read(&l,s);

   /* get rid of any comments that may appear with the cd command and check
   for successful code */
   while ((strstr(s,"250 CWD")==NULL) && (strstr(s,"550 ")==NULL))
      link_read(&l,s);

   if (strstr(s,"250 ")==NULL)
      return ("");

   /* send the 'pwd' command to ftp to get the new path to the current site
   directory; the path is simply part of the output for pwd; form a list of
   the new directory */
   link_write(&l,"pwd");
   link_read(&l,s);

   temp=strstr(s,"\" ");
   *temp='\0';
   temp=strstr(s,"/");

   while(link_input_waiting(&l))
       link_read(&l,s);

#ifdef DEBUG
printf("leaving cd_remote_dir()\n");
#endif 

   return (temp);
   }

/*
 * Function:  char *quota() 
 *              
 * Description:  Returns a string containing the number indicating the number
 *               of kilobytes free in the user's quota  
 *           
 * Inputs:  None.
 *
 * Outputs:  The user's quota remaining is returned in string format through
 *           the function name. 
 */

char *quota()
   {
   static char s[100];
   FILE *f;
   int quota_total, quota_used;

#ifndef NO_AFS
   sprintf(s, "fs lq %s 2>&1", curr_local_dir);
   f=popen(s, "r");
   
   if (!f)
      return("Could not determine disk quota.");
   
   fgets(s, 100, f);

   if (strstr(s, "fs: "))
      {
      if (strstr(s, "You don't have the required access rights"))
          {
          /* AFS, the user did not have the required rights */
          sprintf(s, "Insufficient Access Rights.");
          pclose(f);
          return(s);
          }
      else if (strstr(s, "not in AFS"))
         {
         static char free_space[15];

         /* Normal Unix */
         pclose(f);
#ifdef __hpux
         sprintf(s, "/usr/bin/bdf %s", curr_local_dir);
#else
         sprintf(s, "/bin/df %s", curr_local_dir);
#endif
         f=popen(s, "r");

         if (!f)
            return("Could not determine disk quota.");
   
         fgets(s, 100, f);
         fgets(s, 100, f);
         fgets(s, 100, f);
         pclose(f);
         sscanf(s,"%*s %*s %*s %s", free_space);
         return(free_space);
         }
      else
         {
         /* The message was neither of the above and did not say 'not
            in AFS,' so I don't know what it is */
         pclose(f);
         sprintf(s, "Unreadable");
         return(s);
         }
      }

   fgets(s, 100, f);
   pclose(f);

   if (!strstr(s, "fs: "))
      {
      /* AFS, successful */
      sscanf(s,"%*s %d %d",&quota_total,&quota_used);
      sprintf(s,"%d",quota_total-quota_used);
      return(s);
      }
   else if (strstr(s, "You don't have the required access rights"))
      {
      /* AFS, the user did not have the required rights */
      sprintf(s, "Insufficient Access Rights.");
      return(s);
      }
   else if (strstr(s, "not in AFS"))
      {
      static char free_space[15];

      /* Normal Unix */
#ifdef __hpux
      sprintf(s, "/usr/bin/bdf %s", curr_local_dir);
#else
      sprintf(s, "/bin/df %s", curr_local_dir);
#endif
      f=popen(s, "r");

      if (!f)
         return("Could not determine disk quota.");
   
      fgets(s, 100, f);
      fgets(s, 100, f);
      fgets(s, 100, f);
      pclose(f);
      sscanf(s,"%*s %*s %*s %s", free_space);
      return(free_space);
      }
   else 
      {
      /* The message was neither of the above and did not say 'not
         in AFS,' so I don't know what it is */
      sprintf(s, "Unreadable");
      return(s);
      }

#else  /* Unix, no AFS */

   static char free_space[15];

   /* Here is the output the following code is expecting:

Filesystem   Total    kbytes   kbytes   %
node         kbytes   used     free     used  Mounted on
/dev/rz0a     151903   58937   77776    43%   /

   If you are having trouble with this function, type 'df .' and compare
   the output to that above.  For example, you get the same output except
   missing the second line, rip out one of the fgets() calls.  */

#ifdef __hpux
   sprintf(s, "/usr/bin/bdf %s", curr_local_dir);
#else
   sprintf(s, "/bin/df %s", curr_local_dir);
#endif
   f=popen(s, "r");

   if (!f)
      return("Could not determine disk quota.");
   
   fgets(s, 100, f);
   fgets(s, 100, f);
   fgets(s, 100, f);
   pclose(f);
   sscanf(s,"%*s %*s %*s %s", free_space);
   return(free_space);

#endif  /* NO_AFS */
   }


/*
 * Function:  int get(char *file_name)
 *              
 * Description:  This function gets the file specified in file_name from the
 * site.  It checks for error codes and returns a 0 if the get is for some reason
 * unsuccessful.  It returns 1 if successful.
 *           
 * Inputs:  file_name is the name of the file to be gotten from the site
 *
 * Outputs:  Returns a zero if get unsuccessful and a 1 if successful.
 *
 */

int get(char *file_name)
   {
   char s[200],              /* output read from the pipe */
        get_file[100],       /* the command 'cd ' and file name to send to ftp */
        *bad_char,           /* pointer used to get rid of unwanted characters
                             in file names */
        *temp;               /* temporary variable also used in sending the
                             proper commands to ftp */
   int i=0;

#ifdef DEBUG
printf("entering get()\n");
#endif 

   /* get rid of any trash still hanging around */
   while(link_input_waiting(&l))
       link_read(&l,s);

   /* remove unwanted chars from the file name */
   bad_char=strstr(file_name,"\n");
   if (bad_char != (char *)NULL)
      *bad_char='\0';
   
   bad_char=strstr(file_name,"*");
   if (bad_char != (char *)NULL)
      *bad_char='\0';

   bad_char=strstr(file_name,"@");
   if (bad_char != (char *)NULL)
      *bad_char='\0';

   sprintf(get_file, "get %s", file_name);

   /* send command to ftp */
   temp=strstr(get_file,"get ");
   link_write(&l,temp);
   link_read(&l,s);

   /* check error codes, return 0 if error occurs*/
   if (strstr(s,"200 ")==NULL)
      {
      error(s);
      return (0);
      }
   
   link_read(&l,s);
   if (strstr(s,"550 ")!=NULL)
      {
      error(s);
      return (0);
      }
   
   if (strstr(s,"150 ")!=NULL)
      {
      link_read(&l,s);

      if (strstr(s,"521 ")!=NULL)
         {
         error(s);
         return (0);
         }

      /*send pwd to ftp to flush rest of output in pipe so that it can be read*/
      link_write(&l,"pwd");
      link_read(&l,s);
/* -- More commenting out! -- Rik
      link_read(&l,s);
      link_read(&l,s);
*/
      }

   while(link_input_waiting(&l))
       link_read(&l,s);

#ifdef DEBUG
printf("leaving get()\n");
#endif 

   /* successful execution, return a 1 */
   return (1);
   }

/*
 * Function:  int binary()
 *              
 * Description:  This function sets the file transfer type to binary.
 *           
 * Inputs:  None.
 *
 * Outputs:  Returns 1 if successful, 0 otherwise.  
 *
 * Restrictions:  None. 
 *
 */
int binary()
   {
   char s[200];      /* holds the output from the pipe */
   int i=0;

#ifdef DEBUG
printf("entering binary()\n");
#endif 

   /* get rid of any trash still hanging around */
   while(link_input_waiting(&l))
       link_read(&l,s);

   /* send the binary command to ftp */
   link_write(&l,"binary");     
   link_read(&l,s);

   /* check for success */
   if (strstr(s,"200 Type set to I.")!=NULL)
      return (1);

#ifdef DEBUG
printf("leaving binary()\n");
#endif 

   return (0);
   }

/*
 * Function:  int ascii()
 *              
 * Description:  This function sets the file transfer type to ascii (text).
 *           
 * Inputs:  None.
 *
 * Outputs:  Returns 1 if successful, 0 otherwise.
 *
 */
int ascii()
   {
   char s[200];      /* holds the output from the pipe */
   int i=0;

#ifdef DEBUG
printf("entering ascii()\n");
#endif 

   /* get rid of any trash still hanging around */
   while(link_input_waiting(&l))
       link_read(&l,s);

   /* send the ascii command to ftp */
   link_write(&l,"ascii");
   link_read(&l,s);

   /* check for success */
   if (strstr(s,"200 Type set to A.")!=NULL)
      return (1);

#ifdef DEBUG
printf("leaving ascii()\n");
#endif 

   return (0);
   }

/*
 * Function:  void ftp_exit()
 *              
 * Description:  This function checks to see if ftp is connected to a site. 
 * If it is, it calls clear_lists to free both of the doubly linked lists.  It
 * then sends ftp the 'quit' command and closes the link.  The variable
 * connected, which keeps track of whether or not ftp is connected to a site, is
 * set to not connected (0).
 *           
 * Inputs:  None.
 *
 * Outputs:  None.
 *
 */
void ftp_exit()
   {
   if (connected)
      {
      link_write(&l,"quit");
      link_close(&l);
      connected=0;
      }
   }

/*
 * Function:  int hash_state(int state)
 *              
 * Description:  This function turns the hash mark function on or off.
 *           
 * Inputs:  None.
 *
 * Outputs:  Success returns 1, failure returns 0.
 *
 */
void toggle_hash()
   {
   char s[200];      /* holds the output from the pipe */

   /* send the hash command to ftp */
   link_write(&l,"hash");
   link_write(&l,"pwd");
   link_read(&l,s);
/* -- Get rid of this too :-) -- Rik
   link_read(&l,s);
*/

   while(link_input_waiting(&l))
       link_read(&l,s);
   }
