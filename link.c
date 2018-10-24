/* Link module, Marshall Brain */

/* This module allows a program to form links to other seperately
   executing program and communicate with them. Links can be
   opened and closed, and the program using this unit can 
   write to and read from the other program over the link. */

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <stdlib.h>
#include "strings.h"

#include "link.h"
#include "ftp.h"
#include "global.h"

/* variables defined in ftp.c */
extern Widget ftp_output_text;
extern Boolean ftp_output_status;
extern XtAppContext context;

/* variables global to this file */
static XtIntervalId timer_id;
static Boolean connected;

/* this belongs in ftpcomm.c but placed here for convenience */
int check_for_timeout(caddr_t data, XtIntervalId *id)
{
  char s[100];

#ifdef DEBUG
printf("Timer Expired!!\n");
#endif

  if (!connected)
     return(0);

  link_write(&l,"pwd");
  link_read(&l,s);

  if (strstr(s, "421 "))
    {
    error("Connection closed by the site (time out due to inactivity).\nYou must log back in if you wish to continue.");
    disconnect();
    }
  else
    /* finish pwd read */
    while(link_input_waiting(&l))
      link_read(&l,s);
}

link_open(struct link_handle *lnk, char name[], char param[], char param2[], char param3[],
   char param4[])
{
  connected = True;
  timer_id = XtAppAddTimeOut(context, 240000, check_for_timeout, NULL);

  setbuf(stdout,NULL);
  pipe(lnk->pipefd1);
  pipe(lnk->pipefd2);
  if((lnk->pid=fork())==0)       /* child */
  {
    close(lnk->pipefd1[0]);
    close(1);
    dup(lnk->pipefd1[1]);
    close(2);		         /* 2 new lines */
    dup(lnk->pipefd1[1]);
    close(lnk->pipefd2[1]);
    close(0);
    dup(lnk->pipefd2[0]);
    execlp(name,name,param,param2,param3,param4,(char*)0); 
  }
  else
  {
    lnk->fpin=fdopen(lnk->pipefd1[0],"r");
    lnk->fpout=fdopen(lnk->pipefd2[1],"w");
    close(lnk->pipefd1[1]); 
    close(lnk->pipefd2[0]); 
   }
}

link_close(struct link_handle *lnk)
{
  wait((union wait*)0);
  close(lnk->pipefd1[1]);
  close(lnk->pipefd2[0]);
  fclose(lnk->fpin);
  fclose(lnk->fpout);
  lnk->pid=0;

  if (timer_id)
     XtRemoveTimeOut(timer_id);

  connected=False;
}

int link_read(struct link_handle *lnk, char s[])
{
  int eof_flag;

  *s = '\0';
  if (fgets(s,200,lnk->fpin)==NULL)
    eof_flag=1;
  else
    eof_flag=0;

#ifndef __hpux
 fflush(lnk->fpin);
#endif

if(feof(lnk->fpin)) fprintf(stderr, "End of input stream!\n");
  if (ftp_output_status)
    {
    XmTextInsert(ftp_output_text,
      XmTextGetCursorPosition(ftp_output_text),s);
    XmTextShowPosition(ftp_output_text, XmTextGetLastPosition(ftp_output_text));
    }

#ifdef DEBUG
printf("READ:  %s", s);
#endif

  return(eof_flag);
}

int link_input_waiting(struct link_handle *l)
{
  int num;

  ioctl(l->pipefd1[0],FIONREAD,&num);
  return(num);
}

link_write_char(struct link_handle *lnk,char c)
{
  fprintf(lnk->fpout,"%c",c);
  fflush(lnk->fpout);
}
  
link_write(struct link_handle *lnk,char s[])
{
  fprintf(lnk->fpout,"%s\n",s);
  fflush(lnk->fpout);

  /* set timer to check for time out */
  XtRemoveTimeOut(timer_id);
  timer_id = XtAppAddTimeOut(context, 240000, check_for_timeout, NULL);

#ifdef DEBUG
printf("SENT:  %s\n", s);
#endif
}

link_kill(struct link_handle *lnk)
{
  kill(lnk->pid,SIGKILL);
  link_close(lnk);
}
