/*
 * NXHelp - nxhelp.c
 *	by Lance Lovette
 *	Code and display copyright 1992, by Lance Lovette.
 *	All Rights Reserved
 */

#include "NXHelp.h"					/* Contains declarations for global functions */
#include "NXHelp.icn"					/* The icon */

#include <stdio.h>					/* Standard C include file */
#include <strings.h>					/* Include file for string operations */
#include <sys/stat.h>					/* For file stats */
#include <X11/Intrinsic.h>				/* Standard include file for the Xt Intrinsics */

#include <Xm/Form.h> 
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/MessageB.h>
#include <Xm/Separator.h>
#include <Xm/SelectioB.h>
#include <X11/cursorfont.h>				/* Include file for cursor fonts (watch, xterm) */
#include <Xm/Protocols.h>                               /* We want to communicate with MWM */

/*******************************************************/

#define	APP_NAME		"NXHelp"		/* Name of this application */
#define	HEADER			"[-NXHelpFile"		/* Key identifier to know if a file is a help file */
#define SPACE			"!"			/* The <key word> that indicates a blank space for selections */
#define dSPACE			102			/* Number associated with the keyword SPACE */
#define END			"END"			/* The <key word> that indicates end of the help file */
#define dEND			101			/* Number associated with keyword END */
#define NO_INDEX		'.'			/* A character that flags a keyword as not to be put in the index */
#define dNO_INDEX		103			/* Number associated with the keyword INDEX */
#define NO_TEXT			'-'			/* A character that flags a keyword associated with no text */
#define MAXWORD			300			/* Maximum index keyword */

Widget	helpWindow		=NULL;			/* The help window's toplevel */
Widget	scrolledTextArea	=NULL;			/* The scrolled text window */
Widget	noHelpDialog		=NULL;			/* The dialog for not finding a specified keyword */
Widget	indexBox		=NULL;			/* The index dialog */
static	Widget	parentWidget	=NULL;			/* The parent widget of the calling program */

XtAppContext context;					/* The help facilities application context */

static	Boolean	indexFlag	=False;			/* True if index has been pushed before, saves time,
							   by only getting keywords when index is first pushed */
static	Boolean	changeCursor	=False;			/* True if the application should change the cursor to a watch */
static	Boolean	helpInitialized	=False;			/* True if the help has been properly initialized */

static	Cursor	watchCursor	=NULL;			/* The watch cursor */

static  char	*helpFilename	=NULL;			/* The current help filename */
static  char	*windowTitle	=NULL;			/* The title of the window */
static  char	*iconTitle	=NULL;			/* The title of the icon */

/*******************************************************/

Boolean ValidateFile(char *filename)
{
  FILE		*file	=NULL;
  char		*string	=NULL;
  char		*header	=NULL;
  char		*title	=NULL;
  char		*ver	=NULL;
  char		*t	=NULL;
  char		*s	=NULL;
  Boolean	valid	=True;
  int		i;

 /*
  * This procedure takes a filename and returns True or False.
  * It returns True if the file specified is an XnHelpCB file.
  * It determines this from the first line of the file.
  * If the file is a valid help file, then some of the necessary
  * components from the first line are extracted, otherwise it
  * returns False.
  */

  valid=False;
  file=fopen(filename, "r");
  if( file )
   {
    valid=True;
    string=(char *) malloc(1000);
    fgets(string, 1000, file);

   /*
    * Determine if the file is valid by getting the first 11 characters
    * of the file and seeing if they match HEADER.
    */
    s=string;
    header=(char *) malloc(13);
    t=header;
    for(i=0; (i < 12) && ((*s) != '\0'); i++, s++, t++) (*t)=(*s);
    (*t)='\0';
    t=NULL;
    i=strcmp(header, HEADER);
    free(header);

    /* If the file is not valid, tell someone. */
    if( i != 0 ) valid=False;

    /* If the file is valid, extract the things we need from it. */
    if( valid )
     {
        /* First we get the title of the parent program. */
        s++;  /* Skip the space. */
        title=(char *) malloc(1000);
        t=title;
        /* Untill we hit a space or the '-' before the ']'. */
        for(i=0; ((*s) != ' ') && ((*s) != '-') && ((*s) != '\0'); i++, s++, t++) (*t)=(*s);
        (*t)='\0';
        t=NULL;
        windowTitle=(char *) malloc(50);
        sprintf(windowTitle, "%s", title);
        iconTitle=(char *) malloc(50);
        sprintf(iconTitle, "%s (Help)", title);
        free(title);
     }

    /* Close everything up and free strings. */
    fclose(file); 
    free(string);
    return(valid);
   }
  else fprintf(stderr, "%s: XnInitialize - Could not open the file %s.\n", APP_NAME, filename);

  return(valid);
}

/*******************************************************/

int CopyStr(char **dest, char *src)
{
  if( (!src) || (!dest) ) return(0);

 /*
  * Copys src into dest, but mallocs
  * dest to the required size first.
  */
 
  if( (*dest) ) free( (*dest) );

  (*dest)=(char *) malloc( strlen(src) + 1 );
  strcpy( (*dest), src);

  return(1);
}

/*******************************************************/

int XnInitializeHelp(XtAppContext con, Widget widget, char *filename, Boolean change)
{
  Boolean valid	=False;

 /*
  * Initializes the necessary variables for the correct
  * operation of the XnHelpCB.
  */

  if( !widget ) 
   {
    fprintf(stderr, "%s: XnInitialize - Widget is NULL.\n", APP_NAME); 
    return(0);
   }
  else parentWidget=widget;

  if( !con )
     {
    fprintf(stderr, "%s: XnInitialize - XtAppContext is NULL.\n", APP_NAME); 
    return(0);
   }
  else context=con;

  if( !filename )
   {
    fprintf(stderr, "%s: XnInitialize - Help Filename is NULL.\n", APP_NAME); 
    return(0);
   }
  else
   {
    CopyStr(&helpFilename, filename);
    valid=ValidateFile(filename);
   }

  if( !valid )
   {
    fprintf(stderr, "%s: XnInitialize - The file %s is not an %s file.\n", APP_NAME, filename, APP_NAME);
    return(0);
   }

  changeCursor=(Boolean)change;

  helpInitialized=True;
  return(1);
}

/*******************************************************/

char *CleanString2(char *tempStr)
{
  char *s	=NULL;
  char *t	=NULL;
  char *newStr	=NULL;

/*
 * Remove all leading spaces from the string tempStr
 * by copying the characters from tempStr to newStr.
 */

  s=tempStr;
  newStr=(char *) malloc( strlen(tempStr) + 1);
  t=newStr;
  tempStr=NULL;

  while( (*s) == ' ' ) s++;
  while( (*s) != '\0' ) { (*t)=(*s); s++; t++; }
  (*t)='\0';

  return(newStr);
}

/*******************************************************/

void CreateHelpIndex(char *filename)
{
  FILE		*file		=NULL; 
  char		*word		=NULL;
  char		*s		=NULL;
  Boolean	flag		=False; 
  Boolean	foundKey	=False;
  struct stat	statBuffer;
  Arg		al[10];
  int		ac;
  XmString	list[MAXWORD];
  int		i, j, k, fileLength, listNum, status;

/*
 * This function runs through the file
 * 'filename' and every keyword between
 * '<' and '>' is put into the selection dialog
 * box as a selection.  If the first character
 * of the keyword is NO_INDEX, then that keyword
 * is not put into the list.
 */

  XDefineCursor( XtDisplay(helpWindow), XtWindow(helpWindow), watchCursor);
  XFlush( XtDisplay(helpWindow) );

  file=fopen(filename, "r");
  if( file == NULL ) fprintf(stderr,"%s: CreateHelpIndex - Error opening the help file.\n", APP_NAME);
  else
    {
     if( stat(filename, &statBuffer) != (-1) ) fileLength=statBuffer.st_size;
     else fileLength=1000000;

     s=		(char *) XtMalloc(fileLength);
     word=	(char *) XtMalloc(MAXWORD);

     listNum=0;

     foundKey=False;
     while( foundKey == False )
      { 
       s[0]=' ';
       /* find a '<' - indicating a keyword */ 
       for(j=0; (j != EOF) && (s[0] != '<'); ) 
        {
         j=fgetc(file); s[0]=j;
        }

       /* get the keyword - between the '< >' */ 
       i=0; k=0; flag=False; j=0;
       while( (j != EOF) && (flag == False) )
        {
         j=fgetc(file); s[i]=j;
         if( s[i] != '>' )
          {
           word[k]=s[i]; k++; i++;
          } 
         else flag=True;
        } 
       word[k]='\0'; 

       /* find out if the word is special */
       status=0;
       status=strcmp(word, END);   if( status == 0 ) status=dEND;  else
       status=strcmp(word, SPACE); if( status == 0)  status=dSPACE; else
       if( word[0] == NO_INDEX ) status= dNO_INDEX;

       if( status == dSPACE )		word[0]= ' ';
       if( status == dEND )		foundKey=True; else
       if( status == dNO_INDEX )	;
       else
        {
         list[listNum]=XmStringCreate( word, XmSTRING_DEFAULT_CHARSET );
         listNum++;
        }
      }

     fclose(file);
     XtFree(word);  word=NULL;
     XtFree(s); s=NULL;

/*
 * Set the indexBox items to 'list' and
 * the list's item count to 'listNum'.
 */
     ac=0;
     XtSetArg(al[ac],      XmNlistItems,       list);   ac++;
     XtSetArg(al[ac],      XmNlistItemCount,   listNum);      ac++;
     XtSetValues(indexBox, al, ac);

     /* We created an index so set to True */
     indexFlag=True;
     XtManageChild(indexBox);
    }

  XDefineCursor( XtDisplay(helpWindow), XtWindow(helpWindow), None);
}

/*******************************************************/

char *GetHelpText(char *filename, char *keyword)
{
  FILE		*file		=NULL; 
  char		*word		=NULL;
  char		*s		=NULL;
  char		*text		=NULL;
  Boolean	flag		=False; 
  Boolean	foundKey	=False; 
  Boolean	other		=False;
  struct stat statBuffer;
  int i, j, k, fileLength;

/*
 * This function searches the file
 * 'filename' for the keyword 'keyword'
 * in between brackets '<>'. If it is found,
 * the text after the '>' and before the next
 * '<' is returned.  If the next character after
 * the '>' is an '=' then the rest of the line
 * (until newline) is used as a keyword and
 * the search is continued using the new keyword.
 * This way two keywords can reference the same text.
 */

  file=fopen(filename, "r");
  if( file == NULL ) fprintf(stderr,"%s: GetHelpText - Error opening the help file.\n", APP_NAME);
  else
   {
     if( stat(filename, &statBuffer) != (-1) ) fileLength=statBuffer.st_size;
     else fileLength=1000000;

     text=(char *) XtMalloc(fileLength);
     s=	  (char *) XtMalloc(fileLength);
     word=(char *) XtMalloc(300);

/*
 * Find the keyword 'keyword' in between 
 * '<' and '>'.
 */

     while( foundKey == False )
      { 
            /* find a '<' - indicating the beginning of a keyword */ 
            flag=False; j=0;
            while( (j != EOF) && (flag == False) ) 
             {
              j=fgetc(file); s[0]=j;
              if( s[0] == '<' ) flag=True;
             }

	    /* Remove leading spaces */
	    flag=False; j=0;
            while( (j != EOF) && (flag == False) )
            {
             j=fgetc(file); s[0]=j;
             if( s[0] == NO_INDEX ) s[0] = ' ';
	     if( s[0] != ' ' ) flag=True;
            }
	    word[0]=s[0];
     
            /* get the keyword - between the '< >' */ 
            k=1; flag=False; j=0;
            while( (j != EOF) && (flag == False) )
             {
              j=fgetc(file); s[0]=j;
              if( s[0] == '>' ) flag=True;
              else
               { word[k]=s[0]; k++; } 
             } 
            word[k]='\0';  

            /* compare word found with keyword searching for */
            if( (i=strcmp(word, keyword)) == 0 ) foundKey=True; else

            /* if not a keyword check to see if it is the end of the file */
            if( (i=strcmp(word, END)) == 0 ) foundKey=True; else
            if( j == EOF ) foundKey=True; 
      } 

/*
 * We either hit EOF, a keyword was found, or
 * we ran out of keywords.
 */

    /* If no keyword was found in the file return NULL */
    if( (i=strcmp(word, keyword)) != 0 ) text=NULL;
    else 
     {
           /* if there is an =, get the text for that word */
           s[0]=fgetc(file);
           if( s[0] == '=' )
            {
	     other=True;
             flag=False; i=0; k=0;
             while( ((j=fgetc(file)) != EOF) && (flag==False) )
              {
               s[i]=j;
               if( s[i] == '\n' ) flag=True;
               else
                { text[k]=s[i]; k++; } 
               i++;
              }
             text[k]='\0'; 
             strcat(text,"\0");
            }

           /* keyword was found, so return text associated with it */
           else 
            {
             other=False;
             flag=False; i=0; k=0;
             j=0; 
             while( ((j=fgetc(file)) != EOF) && (flag==False) )
              {
               s[i]=j;
               if( s[i] == '<' ) flag=True;
               else
                { text[k]=s[i]; k++; } 
               i++;
              }
             text[k]='\0'; 
             strcat(text,"\0");
            }

      fclose(file);

      XtFree(word);  word=NULL;
      XtFree(s); s=NULL;
     }
    }

/*  
 * If a keyword has an '=' for it's help text,
 * the text associated with the keyword following
 * the '=' is shown.  This allows multiple keywords
 * to display the same help text.
 */

  if( other ) GetHelpText(filename, text);
  else return( text ); 
} 

/*******************************************************/

void NoHelp(char *keyword)
{
  XmString	message		=NULL;
  char		*s		=NULL;
  Arg al[10];
  int ac;

/*
 * If the help callback is sent a keyword
 * that is not in the file, notify the user.
 */

  s=(char *) XtMalloc(300);
  sprintf(s,"No help on the topic '%s' was found.",keyword);

  ac=0;
  message=XmStringCreate(s,XmSTRING_DEFAULT_CHARSET);
  XtSetArg(al[ac],	XmNmessageString,	message);	ac++;
  XtSetValues( noHelpDialog, al, ac);
  XtManageChild( noHelpDialog );

  XtFree(s); s=NULL;
  XDefineCursor( XtDisplay(helpWindow), XtWindow(helpWindow), None);
}

/*******************************************************/

void GetHelp(char *keyword)
{
  char	*text	=NULL;
  int	i;
  
/*
 * This procedure determines what to do
 * with the variable 'keyword'.
 * If the keyword is 'Index' then it calls
 * CreateHelpIndex, otherwise it calls
 * GetHelpText and sets the helpWindow's text
 * to the text returned by GetHelpText.
 */

  XDefineCursor( XtDisplay(helpWindow), XtWindow(helpWindow), watchCursor);
  XFlush( XtDisplay(helpWindow) );

  i=strcmp(keyword, "Index");
    
  /* If the keyword is 'Index' create an index */
  if( i == 0 )
   if( indexFlag == False ) CreateHelpIndex(helpFilename);
   else XtManageChild(indexBox);

  /* Otherwise get the text associated with the keyword */
  else
   {
    text=GetHelpText(helpFilename, keyword);

    /* If no help for that keyword was found, tell user */
    if( text == NULL ) XtAppAddTimeOut(context, 1000, NoHelp, keyword);
    else
     {
      XmTextSetString( scrolledTextArea, text );
      XtFree(text); text=NULL;
     }
   }

  XDefineCursor( XtDisplay(helpWindow), XtWindow(helpWindow), None);
}

/*******************************************************/

void PushHelpCancelCB(Widget w, char *client_data, XmAnyCallbackStruct *call_data)
{

/*
 * When you push -Cancel- destroy the helpWindow
 * and set it to NULL.  Set everything just like it
 * was when you started.
 */

  XtDestroyWidget( helpWindow );
  helpWindow		=NULL;
  scrolledTextArea	=NULL;
  watchCursor		=NULL;
  indexFlag		=False;
}

/*******************************************************/

void IndexBoxOKCB(Widget w, char *client_data, XmAnyCallbackStruct *call_data)
{
  XmSelectionBoxCallbackStruct	*selection	=NULL;
  char				*s		=NULL;

/*
 * This is the callback for when the user
 * selects a keyword from the helpIndexBox.
 * Take that keyword and send it through the
 * XnHelpCB, just as if it was called by a callback.
 */

  XDefineCursor( XtDisplay(helpWindow), XtWindow(helpWindow), watchCursor);
  XFlush( XtDisplay(helpWindow) );

  selection=(XmSelectionBoxCallbackStruct *) call_data;
  XmStringGetLtoR(selection->value, XmSTRING_DEFAULT_CHARSET, &s);
 
  /* Make sure the user didn't select a blank selection */
  if( s != NULL )
   if( strlen(s) > 1 )  
    if( s[strlen(s)-1] != NO_TEXT )
     {
      XtUnmanageChild( indexBox );
      XnHelpCB(w, CleanString2(s), NULL);
     }

  XDefineCursor( XtDisplay(helpWindow), XtWindow(helpWindow), None);
}

/*******************************************************/

void IndexBoxCancelCB(Widget w, char *client_data, XmAnyCallbackStruct *call_data)
{
 
/* 
 * The indexBoxCancelCB. Just unmanage it.
 */

  XtUnmanageChild( indexBox );
}

/*******************************************************/

void CreateHelpTextArea(void)
{
  XmString	message	=NULL;
  Pixmap	pixmap	=NULL;
  Arg al[20];
  int ac;
  Widget helpForm, sep2, cancelButton, indexButton, textSW;
  Atom          WM_DELETE_WINDOW;

/*
 * This function creates the help window.  It is a
 * separate application shell with a scrolled
 * text area widget.  It is destroyed when it is
 * unmanaged, so we only need to create the window
 * when the helpWindow is NULL.
 */

  if( helpWindow == NULL )
  {

   if( changeCursor )
    {
     XDefineCursor( XtDisplay(parentWidget), XtWindow(parentWidget),
	 XCreateFontCursor( XtDisplay(parentWidget), XC_watch ));
     XFlush( XtDisplay(parentWidget) );
     XmUpdateDisplay(parentWidget);
    }

   ac=0;
   XtSetArg(al[ac],	XmNtitle,		windowTitle);		ac++;
   XtSetArg(al[ac],	XmNiconName,		iconTitle);		ac++;
   XtSetArg(al[ac],	XmNminWidth,		-1);			ac++;
   XtSetArg(al[ac],	XmNmaxWidth,		-1);			ac++;
   XtSetArg(al[ac],	XmNminHeight,		-1);			ac++;
   XtSetArg(al[ac],	XmNmaxHeight,		-1);			ac++;
   XtSetArg(al[ac],	XmNwidth,		550);			ac++;
   XtSetArg(al[ac],	XmNheight,		300);			ac++;
/******
   helpWindow=	XtCreateApplicationShell("HelpWindow", topLevelShellWidgetClass, al, ac);
********/
   helpWindow=XtAppCreateShell("HelpWindow","HelpWindow", topLevelShellWidgetClass,
        XtDisplay(parentWidget), al, ac);

   if( help_width != 0 )
    pixmap=XCreateBitmapFromData(XtDisplay(helpWindow), XtScreen(helpWindow)->root,
	help_bits, help_width, help_height);

   if( pixmap != NULL )
    {
     ac=0;
     XtSetArg(al[ac],	XmNiconPixmap,		pixmap);		ac++;
     XtSetValues(helpWindow, al, ac);
    }

   ac=0;
   helpForm=	XtCreateManagedWidget(	"helpForm", xmFormWidgetClass, helpWindow, al,ac);

   ac=0;
   message=XmStringCreate("Cancel",XmSTRING_DEFAULT_CHARSET);
   XtSetArg(al[ac],	XmNlabelString,		message);		ac++;
   XtSetArg(al[ac],	XmNbottomOffset,	3);			ac++;
   XtSetArg(al[ac],	XmNbottomAttachment,	XmATTACH_FORM);		ac++;
   XtSetArg(al[ac],	XmNleftAttachment,	XmATTACH_POSITION);	ac++;
   XtSetArg(al[ac],	XmNleftPosition,	40);			ac++;
   cancelButton=XtCreateManagedWidget("cancelButton", xmPushButtonWidgetClass, helpForm, al, ac);
   XtAddCallback(cancelButton, XmNactivateCallback, PushHelpCancelCB, NULL);

   ac=0;
   message=XmStringCreate("Index",XmSTRING_DEFAULT_CHARSET);
   XtSetArg(al[ac],	XmNlabelString,		message);		ac++;
   XtSetArg(al[ac],	XmNbottomOffset,	3);			ac++;
   XtSetArg(al[ac],	XmNbottomAttachment,	XmATTACH_FORM);		ac++;
   XtSetArg(al[ac],	XmNleftAttachment,	XmATTACH_POSITION);	ac++;
   XtSetArg(al[ac],	XmNleftPosition,	50);			ac++;
   indexButton=XtCreateManagedWidget("indexButton", xmPushButtonWidgetClass, helpForm, al, ac);
   XtAddCallback(indexButton, XmNactivateCallback, XnHelpCB, "Index");

   ac=0;
   XtSetArg(al[ac],	XmNbottomAttachment,	XmATTACH_WIDGET);		ac++;
   XtSetArg(al[ac],	XmNbottomWidget,	cancelButton);		ac++;
   XtSetArg(al[ac],	XmNleftAttachment,	XmATTACH_FORM);		ac++;
   XtSetArg(al[ac],	XmNrightAttachment,	XmATTACH_FORM);		ac++;
   sep2=XtCreateManagedWidget("sep2", xmSeparatorWidgetClass, helpForm, al, ac);

   ac=0;
   XtSetArg(al[ac],	XmNeditable, 		False); 		ac++;
   XtSetArg(al[ac],	XmNeditMode,		XmMULTI_LINE_EDIT); 	ac++;
   XtSetArg(al[ac],	XmNscrollVertical,	True); 			ac++;
   XtSetArg(al[ac],	XmNresizeWidth,	True);			ac++;
   scrolledTextArea=XmCreateScrolledText(helpForm, "scrolledText", al, ac);
   XtManageChild( scrolledTextArea );

   textSW=XtParent( scrolledTextArea );
   ac=0;
   XtSetArg(al[ac],	XmNtopAttachment,	XmATTACH_FORM);		ac++;
   XtSetArg(al[ac],	XmNbottomAttachment,	XmATTACH_WIDGET);	ac++;
   XtSetArg(al[ac],	XmNbottomWidget,	sep2);			ac++;
   XtSetArg(al[ac],	XmNleftAttachment,	XmATTACH_FORM);		ac++;
   XtSetArg(al[ac],	XmNrightAttachment,	XmATTACH_FORM);		ac++;
   XtSetValues( textSW, al, ac);

   XtRealizeWidget( helpWindow );

/*
 * Create the dialog boxes after the helpWindow has been
 * realized.
 */
   ac=0;
   message=XmStringCreate("No help for this topic was found.",XmSTRING_DEFAULT_CHARSET);
   XtSetArg(al[ac],	XmNmessageString,	message);	ac++;
   noHelpDialog=XmCreateInformationDialog(helpWindow,"noHelp",al,ac);
   message=XmStringCreateLtoR("No Help Dialog", XmSTRING_DEFAULT_CHARSET);
   XtSetArg(al[ac],	XmNdialogTitle,		message);	ac++;
   XtUnmanageChild(XmMessageBoxGetChild(noHelpDialog,XmDIALOG_CANCEL_BUTTON));
   XtUnmanageChild(XmMessageBoxGetChild(noHelpDialog,XmDIALOG_HELP_BUTTON));

   ac=0;
   message=XmStringCreate("Help Topics",XmSTRING_DEFAULT_CHARSET);
   XtSetArg(al[ac],	XmNlistLabelString,	message);	ac++;
   message=XmStringCreateLtoR("Help Index Dialog", XmSTRING_DEFAULT_CHARSET);
   XtSetArg(al[ac],	XmNdialogTitle,		message);	ac++;
   XtSetArg(al[ac],	XmNautoUnmanage,	False);		ac++;
   indexBox=XmCreateSelectionDialog(helpWindow,"indexBox",al,ac);
   XtUnmanageChild(XmSelectionBoxGetChild(indexBox,XmDIALOG_HELP_BUTTON));
   XtUnmanageChild(XmSelectionBoxGetChild(indexBox,XmDIALOG_APPLY_BUTTON));
   XtAddCallback(indexBox, XmNokCallback, IndexBoxOKCB, NULL);
   XtAddCallback(indexBox, XmNcancelCallback, IndexBoxCancelCB, NULL);

   if( watchCursor == NULL ) watchCursor=XCreateFontCursor( XtDisplay(helpWindow), XC_watch );
   XDefineCursor( XtDisplay(helpWindow), XtWindow(helpWindow), watchCursor);
   if( changeCursor ) XDefineCursor( XtDisplay(parentWidget), XtWindow(parentWidget), None);

  /*
   * We want the window manager to call PushHelpCancelCB when the user
   * closes the window using the 'Close' selection on the window frame menu.
   */
   WM_DELETE_WINDOW=XmInternAtom(XtDisplay(helpWindow), "WM_DELETE_WINDOW", False);
   XmAddWMProtocolCallback(helpWindow, WM_DELETE_WINDOW, PushHelpCancelCB, NULL);
  }
}

/*******************************************************/

void XnHelpCB(Widget w, char *client_data, XmAnyCallbackStruct *call_data)
{

/* 
 * This is the callback that is registered with 
 * the Xt Intrinsics for each button with a help
 * callback.  The client_data is a pointer to a
 * string which is the keyword to search the file
 * 'helpFilename' for.  If the help window is not
 * already created, it is created before anything
 * starts.
 */

  if( !helpInitialized )
   {
    fprintf(stderr, "%s:  XnHelpCB - XnHelpCB has not been initialized.\n", APP_NAME);
    return;
   }

  if( !helpFilename )
   {
    fprintf(stderr,"%s: XnHelpCB - No help file is defined.\n", APP_NAME);
    return;
   }

  if( client_data )
   if( helpWindow == NULL )
    {
     CreateHelpTextArea();
     XtAppAddTimeOut(context, 2000, GetHelp, client_data);
    }
   else GetHelp( client_data );
 else fprintf(stderr,"%s: XnHelpCB - Cannot search for a NULL keyword.\n", APP_NAME);
} 

/*******************************************************/
