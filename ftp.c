/*
 *  NXFTP - ftp.c
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

/* must be changed for each system accordingly */

/* #define SITE_LIST   "/opt/nxftp/lib/nxftp.list" */
/* #define SITE_LIST "/opt/tmp/nxftp/lib" */
#define SITE_LIST "/opt/archive/nxftp/nxftp-3.1a/anon_ftp_sites"

/* #define HELP_FILE  "/opt/nxftp/lib/nxftp.hlp" */
/* #define HELP_FILE  "/opt/tmp/nxftp/lib" */
#define HELP_FILE  "/opt/archive/nxftp/nxftp-3.1a/nxftp.hlp"

/* this macro must be changed so that the file .nxftprc is saved in
   the user's home directory

   at NCSU, for example, the homedir is /ncsu/loginid/

   Note:  the variables used are local variables to the functions that
          use them; you may change the function calls, etc. as necessary
          but do NOT change the names of the variables */
#define GetPath()    { char *homedir = getenv("HOME"); \
                       sprintf(nxftprc,"%s/.nxftprc",homedir); }

#include <stdio.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <strings.h>

#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/Separator.h>
#include <Xm/SeparatoG.h>
#include <Xm/MessageB.h>
#include <Xm/List.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/BulletinB.h>
#include <Xm/ToggleBG.h>
#include <Xm/SelectioB.h>
#include <Xm/FileSB.h>
#include <Xm/Protocols.h>

#include "ftpcomm.h"
#include "ftp.h"
#include "global.h"
#include "nxhelp/NXHelp.h"

/* bitmap for icon */
#include "nxftp.icon"
#include "nxftp_text.icon"

#define TITLE       "NXftp 3.1"

#define ERROR       0
#define LOGIN       1
#define DISCONNECT  2
#define QUIT        3
#define CANCEL      4
#define OK          5
#define ANONYMOUS   6
#define FOCUS       7
#define NOFOCUS     8
#define BINARY      9
#define ASCII      10
#define LOCALDIR   11
#define READTEXT   12
#define ABOUT      13
#define HELP       14
#define STATUS     15
#define EDIT       16
#define SELECT     17
#define ADD        18
#define REMOVE     19

XtAppContext context;
XmStringCharSet char_set=XmSTRING_DEFAULT_CHARSET;

static Widget toplevel, form, site_name, site_dir, local_dir, rem_disk_quota;

Widget site_info_label, site_name_label, site_dir_label, local_info_label,
       local_dir_label, rem_disk_quota_label, site_sep, local_sep, 
       files_recvd_label, site_files_label, site_dir_list_label, 
       site_files_list, site_dir_list, menu_bar, login_option, local_dir_option,
       disconnect_option, quit_option, login_dialog, login_label1,
       login_label2, login_label3,login_edit1, login_edit2, login_edit3,
       login_ok_button, login_cancel_button, login_anon_button,
       initial_login_info, initial_info, info_cancel_button, get_dialog,
       get_label1, get_filename, file_type_box, binary_button, ascii_button,
       get_ok_button, get_cancel_button, get_sep1, get_sep2, error_dialog, 
       login_sep1, login_sep2, files_recvd, local_dir_prompt,
       read_text_button, about_option, help_option, ftp_output_text=NULL,
       ftp_status_option, ftp_output_dialog, ftp_output_cancel_button, login_site_list,
       list_editor_dialog, list_editor_form, list_editor_add_text, list_editor_add_button, 
       list_editor_remove_button, list_editor_user_site_list, list_editor_general_site_text,
       list_editor_save_button, list_editor_cancel_button, info_shell;

static char site_dir_name[100], site_name_string[50], user_id[50], password[50];

static XmString *files_xmstr, *dirs_xmstr;

Boolean ftp_output_status=False;

extern Link_Handle l;

/* set fallback resources */
String fallback[] = {
                    "*fontList:              -adobe-helvetica-bold-r-normal--12-120-*-*-*-*-*-*",
                    "*list.fontList:         fixed",
                    "*info_text.fontList:    fixed",
                    NULL
                    };

void error (char *s)
    {
    Arg al[10];
    int ac;
    XmString err_msg;

    err_msg=XmStringCreateLtoR(s, char_set);

    ac = 0;
    XtSetArg(al[ac], XmNmessageString, err_msg);  ac++;
    XtSetValues(error_dialog,al,ac);
    XmStringFree(err_msg);

    XtManageChild(error_dialog);
    }

void error_CB(Widget w,int client_data,XmAnyCallbackStruct *call_data)
    {
    XtUnmanageChild(error_dialog);
    }

void create_error_dialog()
    {
    Arg al[10];
    int ac;

    ac=0;
    XtSetArg(al[ac],XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL); ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
        "NXftp:  Error",char_set)); ac++;
    error_dialog = XmCreateErrorDialog(toplevel,
        "error_dialog", al, ac);
    XtAddCallback(error_dialog, XmNokCallback, error_CB, OK);
    XtUnmanageChild(XmMessageBoxGetChild(error_dialog,
        XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(error_dialog,
        XmDIALOG_HELP_BUTTON));

    }

void create_text_and_labels()
    {
    Arg al[15];
    int ac;

    ac=0;
    XtSetArg(al[ac],XmNeditable,False); ac++;
    XtSetArg(al[ac],XmNcursorPositionVisible,False); ac++;
    site_name=XmCreateTextField(form,"site_name",al,ac);
    XtManageChild(site_name);

    ac=0;
    XtSetArg(al[ac],XmNeditable,False); ac++;
    XtSetArg(al[ac],XmNcursorPositionVisible,False); ac++;
    site_dir=XmCreateTextField(form,"site_dir",al,ac);
    XtManageChild(site_dir);

    ac=0;
    XtSetArg(al[ac],XmNeditable,False); ac++;
    XtSetArg(al[ac],XmNcursorPositionVisible,False); ac++;
    local_dir=XmCreateTextField(form,"local_dir",al,ac);
    XtManageChild(local_dir);

    ac=0;
    XtSetArg(al[ac],XmNeditable,False); ac++;
    XtSetArg(al[ac],XmNcursorPositionVisible,False); ac++;
    rem_disk_quota=XmCreateTextField(form,"rem_disk_quota",al,ac);
    XtManageChild(rem_disk_quota);

    /* create a label */
    ac=0;
    XtSetArg(al[ac],XmNlabelString,
        XmStringCreate("FTP Information",char_set)); ac++;
    site_info_label=XmCreateLabelGadget(form,"label1",al,ac);
    XtManageChild(site_info_label);

    /* create a separator */
    ac=0;
    XtSetArg(al[ac],XmNseparatorType,XmDOUBLE_LINE); ac++;
    XtSetArg(al[ac],XmNmargin,1); ac++;
    site_sep=XmCreateSeparatorGadget(form,"site_sep",al,ac);
    XtManageChild(site_sep);

    /* create a label */
    ac=0;
    XtSetArg(al[ac],XmNlabelString,
        XmStringCreate("Site Name",char_set)); ac++;
    site_name_label=XmCreateLabelGadget(form,"label2",al,ac);
    XtManageChild(site_name_label);

    /* create a label */
    ac=0;
    XtSetArg(al[ac],XmNlabelString,
        XmStringCreate("Current Site Directory",char_set)); ac++;
    site_dir_label=XmCreateLabelGadget(form,"label3",al,ac);
    XtManageChild(site_dir_label);

    /* create a label */
    ac=0;
    XtSetArg(al[ac],XmNlabelString,
        XmStringCreate("Current Download Directory",char_set)); ac++;
    local_dir_label=XmCreateLabelGadget(form,"label5",al,ac);
    XtManageChild(local_dir_label);

    /* create a label */
    ac=0;
    XtSetArg(al[ac],XmNlabelString,
        XmStringCreate("Download Free Space",char_set)); ac++;
    rem_disk_quota_label=XmCreateLabelGadget(form,"label6",al,ac);
    XtManageChild(rem_disk_quota_label);

    /* create a label */
    ac=0;
    XtSetArg(al[ac],XmNlabelString,
        XmStringCreate("Files Received",char_set)); ac++;
    files_recvd_label=XmCreateLabelGadget(form,"files_recvd_label",al,ac);
    XtManageChild(files_recvd_label);

    /* create a label */
    ac=0;
    XtSetArg(al[ac],XmNlabelString,
        XmStringCreate("Site Files",char_set)); ac++;
    site_files_label=XmCreateLabelGadget(form,"site_files_label",al,ac);
    XtManageChild(site_files_label);

    /* create a label */
    ac=0;
    XtSetArg(al[ac],XmNlabelString,
        XmStringCreate("Site Directories",char_set)); ac++;
    site_dir_list_label=XmCreateLabelGadget(form,"site_dir_list_label",al,ac);
    XtManageChild(site_dir_list_label);

    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, menu_bar); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 65); ac++; 
    XtSetValues(site_info_label,al,ac);

    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, site_info_label); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 65); ac++; 
    XtSetValues(site_sep,al,ac);

    ac=0;
    XtSetArg(al[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, site_sep); ac++; 
    XtSetArg(al[ac], XmNtopOffset, 5); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftOffset, 5); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 24); ac++;  
    XtSetValues(site_name_label,al,ac);

   /* attach text info widget site_name */
    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, site_name_label); ac++; 
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftOffset, 5); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 24); ac++;  
    XtSetValues(site_name,al,ac);

    ac=0;
    XtSetArg(al[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, site_sep); ac++; 
    XtSetArg(al[ac], XmNtopOffset, 5); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNleftPosition, 27); ac++; 
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 65); ac++;  
    XtSetValues(site_dir_label,al,ac);

   /* attach text info widget site_dir */
    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, site_dir_label); ac++; 
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNleftPosition, 27); ac++; 
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 65); ac++;  
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
    XtSetValues(site_dir,al,ac);

    ac=0;
    XtSetArg(al[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, site_name); ac++; 
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftOffset, 5); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 24); ac++;  
    XtSetValues(rem_disk_quota_label,al,ac);

    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, rem_disk_quota_label ); ac++; 
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftOffset, 5); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 24); ac++;  
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
    XtSetValues(rem_disk_quota,al,ac);

    ac=0;
    XtSetArg(al[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, site_dir); ac++; 
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNleftPosition, 27); ac++; 
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 65); ac++;  
    XtSetValues(local_dir_label,al,ac);

   /* attach text info widget local_dir */
    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, local_dir_label); ac++; 
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNleftPosition, 27); ac++; 
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 65); ac++;  
    XtSetValues(local_dir,al,ac);

    ac=0;
    XtSetArg(al[ac], XmNtopAttachment,   XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, rem_disk_quota); ac++;
    XtSetArg(al[ac], XmNtopOffset, 10); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftOffset, 5); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 30); ac++;  
    XtSetArg(al[ac], XmNrightOffset, 5); ac++;
    XtSetValues(files_recvd_label,al,ac);

    ac=0;
    XtSetArg(al[ac], XmNeditMode,XmMULTI_LINE_EDIT); ac++;
    XtSetArg(al[ac], XmNeditable,False); ac++;
    XtSetArg(al[ac], XmNcursorPositionVisible,False); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget,files_recvd_label ); ac++; 
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftOffset, 5); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 33); ac++;
    XtSetArg(al[ac], XmNrightOffset, 5); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomOffset, 5); ac++;
    files_recvd=XmCreateScrolledText(form,"list",al,ac);
    XtManageChild(files_recvd);

    ac=0;
    XtSetArg(al[ac], XmNtopAttachment,   XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, menu_bar); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNleftWidget, site_info_label); ac++;  
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNrightWidget, site_files_list); ac++;  
    XtSetArg(al[ac], XmNrightOffset, 5); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
    XtSetValues(site_files_label,al,ac);
    }

void create_list_widgets()
   {
   Arg al[12];
   int ac;

   files_xmstr=(XmString *) XtMalloc(sizeof(XmString) * 2000); 
   dirs_xmstr=(XmString *) XtMalloc(sizeof(XmString) * 1000); 

   ac=0;
   XtSetArg(al[ac], XmNlistSizePolicy, XmCONSTANT); ac++;
   XtSetArg(al[ac], XmNscrollBarDisplayPolicy, XmSTATIC); ac++;
/*   XtSetArg(al[ac], XmNselectionPolicy, XmEXTENDED_SELECT); ac++; */
   XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
   XtSetArg(al[ac], XmNtopWidget, site_files_label); ac++; 
   XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
   XtSetArg(al[ac], XmNrightOffset, 5); ac++;
   XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
   XtSetArg(al[ac], XmNleftWidget, site_sep); ac++;
   XtSetArg(al[ac], XmNleftOffset, 10); ac++;
   XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
   XtSetArg(al[ac], XmNbottomOffset, 5); ac++;
   site_files_list=XmCreateScrolledList(form,"list",al,ac);
   XtManageChild(site_files_list);
   XtAddCallback (site_files_list, XmNdefaultActionCallback, files_select_CB, NULL);

    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, local_dir); ac++;  
    XtSetArg(al[ac], XmNtopOffset, 10); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNleftWidget, files_recvd_label); ac++;  
    XtSetArg(al[ac], XmNleftOffset, 5); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNrightWidget, site_files_list); ac++;  
    XtSetArg(al[ac], XmNrightOffset, 5); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
    XtSetValues(site_dir_list_label,al,ac);

   ac=0;
   XtSetArg(al[ac], XmNlistSizePolicy, XmCONSTANT); ac++;
   XtSetArg(al[ac], XmNscrollBarDisplayPolicy, XmSTATIC); ac++;
   XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
   XtSetArg(al[ac], XmNtopWidget, site_dir_list_label); ac++; 
   XtSetArg(al[ac], XmNrightAttachment, XmATTACH_WIDGET); ac++;
   XtSetArg(al[ac], XmNrightWidget, site_files_list); ac++;
   XtSetArg(al[ac], XmNrightOffset, 5); ac++;
   XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
   XtSetArg(al[ac], XmNleftWidget, files_recvd); ac++;
   XtSetArg(al[ac], XmNleftOffset, 5); ac++;
   XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
   XtSetArg(al[ac], XmNbottomOffset, 5); ac++;
   site_dir_list=XmCreateScrolledList(form,"list",al,ac);
   XtManageChild(site_dir_list);
   XtAddCallback (site_dir_list, XmNdefaultActionCallback, dir_select_CB, NULL);
   }

void local_CB(Widget w, int client_data, XmSelectionBoxCallbackStruct *call_data)
   {
   char *path;
   int ac;
   Arg al[5];

   switch (client_data)
      {
      case OK:
         XmStringGetLtoR(call_data->value, char_set, &path);
         cd_local_dir(path);
         break;
      case CANCEL:
         break;
      }

   XtUnmanageChild(w);
   XmTextShowPosition(local_dir, XmTextGetLastPosition(local_dir));
   }

void create_prompt_dialog()
    {
    Arg al[10];
    int ac;

    /* create the change local directory prompt dialog. */
    ac = 0;
    XtSetArg(al[ac], XmNselectionLabelString, XmStringCreateLtoR
       ("Change download directory to ", char_set));  ac++;
    XtSetArg(al[ac],XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL); ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
       "NXftp:  Local Directory",char_set)); ac++;
    local_dir_prompt=XmCreatePromptDialog(toplevel,"local_dir_prompt",al,ac);
 
    XtAddCallback (local_dir_prompt, XmNokCallback, local_CB, OK);
    XtAddCallback (local_dir_prompt, XmNcancelCallback, local_CB, CANCEL);
    XtUnmanageChild(XmSelectionBoxGetChild(local_dir_prompt, XmDIALOG_HELP_BUTTON));
    }

int form_lists()
   {
   int i, j, ac; 
   Arg al[5];
   char s[201], *dir_ent_end, temp[100];   

   /* list the entries in the current directory and check the error codes;
   return a 0 if the ls -F is not successful */

   link_write(&l,"ls -lF");
   link_read(&l,s);

   if (strstr(s,"200 ")==NULL)
      {
      error("An error ocurred in reading the directory list.\nYou will probably need to \
disconnect from the site and try again.\nIt is also possible that this site is incompatible with this program.");
      return (0);
      }

   link_read(&l,s);
   if (strstr(s,"550 ")!=NULL)
      {
      error("An error occured in listing the directory chosen.\nYou are now back in the parent directory.");
      cd_site_up();
      return (0);
      }

   if (strstr(s,"150 ")==NULL)
      {
      error("An error ocurred in reading the directory list.\nYou will probably need to \
disconnect from the site and try again.\nIt is also possible that this site is incompatible with this program.");
      return (0);
      }

   link_read(&l,s);
   if (strstr(s,"total ")!=NULL)
      link_read(&l,s);

   i=0; j=0; 
   dirs_xmstr[j]=XmStringCreateLtoR("../",char_set); j++;
   for (; s[0] != '2' && s[1] != '2' && s[2] != '6'; link_read(&l,s))
      {
      if (strstr(s, "ermission denied"))
          continue;

      /* rip off newline */
      s[strlen(s) - 1] = '\0';
                    
      /* take the appropriate action based on the first character
            d = directory
            l = link
            - = file
         anything else is discarded */
      switch(s[0])
         {
         case 'd':
            {
            int cnt;

            cnt = sscanf(s, "%*s %*s %*s %*s %*s %*s %*s %*s %s\n", temp);
            if (cnt != EOF)
               dirs_xmstr[j++]=XmStringCreateLtoR(temp, char_set);
            else
               {
               cnt = sscanf(s, "%*s %*s %*s %*s %*s %*s %*s %s\n", temp);
               if (cnt != EOF)
                  dirs_xmstr[j++]=XmStringCreateLtoR(temp, char_set);
               }

            break;
            }

         case 'l':
            {
            int cnt;
            char junk[100];

            if (s[strlen(s)-1] == '/')       /* directory */
               {
               cnt = sscanf(s, "%*s %*s %*s %*s %*s %*s %*s %*s %s %*s %s\n", temp, junk);
               if (cnt == 2)
                  {
                  temp[strlen(temp)+1] = '\0';
                  temp[strlen(temp)] = '@';
                  dirs_xmstr[j++]=XmStringCreateLtoR(temp, char_set);
                  }
               else
                  {
                  cnt = sscanf(s, "%*s %*s %*s %*s %*s %*s %*s %s %*s %s\n", temp, junk);
                  if (cnt == 2)
                     dirs_xmstr[j++]=XmStringCreateLtoR(temp, char_set);
                  }
               }
            else                             /* file */
               {
               char name[50];

               cnt = sscanf(s, "%*s %*s %*s %*s %*s %*s %*s %*s %s %*s %s\n", name, junk);
               if (cnt == 2)
                  {
                  if(name[strlen(name)-1] != '@')
                     sprintf(s, "      %s@", name);
                  else
                     sprintf(s, "      %s", name);
                  files_xmstr[i++]=XmStringCreateLtoR(s,char_set);
                  }
               else
                  {
                  cnt = sscanf(s, "%*s %*s %*s %*s %*s %*s %*s %s %*s %s\n", name, junk);
                  if (cnt == 2)
                     {
                     if(name[strlen(name)-1] != '@')
                        sprintf(s, "      %s@", name);
                     else
                        sprintf(s, "      %s", name);
                     files_xmstr[i++]=XmStringCreateLtoR(s,char_set);
                     }
                  }
               }
            }
            break;

         case '-':
            {
            int cnt;
            char name[50];
            int size;

            cnt = sscanf(s, "%*s %*s %*s %*s %*s %*s %*s %*s %s\n", name);
            if (cnt != EOF)
               {
               sscanf(s, "%*s %*s %*s %*s %d", &size);
               size /= 1000;
               if (!size) size++;
               sprintf(s, "%5d %s", size, name);
               files_xmstr[i++]=XmStringCreateLtoR(s,char_set);
               }
            else
               {
               cnt = sscanf(s, "%*s %*s %*s %*s %*s %*s %*s %s\n", name);
               if (cnt != EOF)
                  {
                  sscanf(s, "%*s %*s %*s %d", &size);
                  size /= 1000;
                  if (!size) size++;
                  sprintf(s, "%5d %s", size, name);
                  files_xmstr[i++]=XmStringCreateLtoR(s,char_set);
                  }
               }

            break;
            }
         }
      }

   XmListAddItems(site_files_list, files_xmstr, i, 0);
   XmListAddItems(site_dir_list, dirs_xmstr, j, 0);

   /* flush the pipes by sending a command (pwd) which has no effect on the
   current settings of ftp (the last line from the file listing cannot be read
   from the pipe otherwise) and read all information currently in the pipe,
   discarding that which is not needed */

   link_write(&l,"pwd");
   link_read(&l,s);

/* -- What's the point in having these? -- rik
   link_read(&l,s);
   link_read(&l,s);
*/
   while(link_input_waiting(&l))
       link_read(&l,s);
   link_write(&l,"reset");
   }

void files_select_CB(Widget w, int client_data, XmListCallbackStruct *call_data)
   {
   char temp[100];
   int total = call_data->selected_item_count, cnt;
   char *file_to_get;

   XmStringGetLtoR(call_data->item, char_set, &file_to_get);

   XmTextSetString(get_filename, file_to_get);
   XmListDeselectItem(w, call_data->item);
   XtManageChild(get_dialog);
   }

void dir_select_CB(Widget w, int client_data, XmListCallbackStruct *call_data)
   {
   char *dir, temp[100];

   XmStringGetLtoR(call_data->item,char_set,&dir);

   if ((dir[3] >= '0') && (dir[3] <= '9') && (dir[4] == ' '))
       {
       sscanf(dir,"%*d %s",temp);
       strcpy(dir,temp);
       }

   XmListDeselectItem(w, call_data->item);
   if (strstr(dir,"..")!=NULL)
      cd_site_up();
   else
      cd_site_dir(dir);
   }

void change_menu_sensitivity(Boolean login_state)
/* changes the menu sensitivities between logged in and not logged in states. */
    {
    XtSetSensitive(login_option,!login_state);
    XtSetSensitive(disconnect_option,login_state);
    XtSetSensitive(local_dir_option,login_state);
    }

void ftp_output_CB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {
    ftp_output_status=!ftp_output_status;
    }

void create_ftp_output_dialog()
    {
    Arg al[10];
    int ac;

    ac=0;
    XtSetArg(al[ac],XmNheight,330); ac++;
    XtSetArg(al[ac],XmNwidth,520); ac++;
    XtSetArg(al[ac],XmNverticalSpacing,3); ac++;
    XtSetArg(al[ac],XmNdialogStyle,XmDIALOG_MODELESS); ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
        "NXftp:  FTP Status Information",char_set)); ac++;
    ftp_output_dialog=XmCreateFormDialog(toplevel,
        "ftp_output_dialog",al,ac);

    ac=0;
    XtSetArg(al[ac],XmNeditMode,XmMULTI_LINE_EDIT); ac++;
    XtSetArg(al[ac],XmNeditable,False); ac++;
    XtSetArg(al[ac],XmNcolumns,80); ac++;
    XtSetArg(al[ac],XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac],XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac],XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac],XmNbottomAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac],XmNbottomPosition, 90); ac++;
    ftp_output_text=XmCreateScrolledText(ftp_output_dialog,"ftp_output_text",al,ac);
    XtManageChild(ftp_output_text);

    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR(" Dismiss ", char_set)); ac++;
    ftp_output_cancel_button=XmCreatePushButtonGadget(ftp_output_dialog,
        "ftp_output_cancel_button",al,ac);
    XtManageChild(ftp_output_cancel_button);
    XtAddCallback(ftp_output_cancel_button, XmNactivateCallback, ftp_output_CB, CANCEL);

    ac=0;
    XtSetArg(al[ac],XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac],XmNtopWidget, ftp_output_text); ac++;
    XtSetArg(al[ac],XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac],XmNrightPosition, 55); ac++;
    XtSetArg(al[ac],XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac],XmNleftPosition, 45); ac++;
    XtSetArg(al[ac],XmNbottomAttachment, XmATTACH_NONE); ac++;
    XtSetValues(ftp_output_cancel_button,al,ac);

    ac=0;
    XtSetArg(al[ac],XmNcancelButton,ftp_output_cancel_button); ac++;
    XtSetValues(ftp_output_dialog,al,ac);   
    }

void menuCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
/* handles menu options. */
    {
    Arg al[10];
    int ac, height;

    switch (client_data)
       {
       case LOGIN:
          XtManageChild(login_dialog);
          XmProcessTraversal(login_edit1,XmTRAVERSE_CURRENT);
          XmProcessTraversal(login_edit1,XmTRAVERSE_CURRENT);
          break;

       case DISCONNECT:
          disconnect();
          break;

       case LOCALDIR:
          ac=0;
          XtSetArg(al[ac], XmNtextString, XmStringCreateLtoR(
             curr_local_dir, char_set)); ac++;
          XtSetValues(local_dir_prompt, al, ac);
          XtManageChild(local_dir_prompt);

          XmProcessTraversal((struct _WidgetRec *)XmSelectionBoxGetChild(local_dir_prompt,
             XmDIALOG_TEXT),XmTRAVERSE_CURRENT);
          XmProcessTraversal((struct _WidgetRec *)XmSelectionBoxGetChild(local_dir_prompt,
             XmDIALOG_TEXT),XmTRAVERSE_CURRENT);
          break;

       case STATUS:
          ftp_output_status=!ftp_output_status;
          XtManageChild(ftp_output_dialog);
          break;

       case QUIT:
          ftp_exit();
          XtFree(files_xmstr);
          XtFree(dirs_xmstr);
          exit(0);
          break;
       }
    }

/* adds a separator into the menu */
void make_menu_separator(Widget menu)
    {
    XtManageChild(XmCreateSeparatorGadget(menu,"sep",NULL,0));
    }

Widget make_help_menu(char *menu_name, KeySym mnemonic, Widget menu_bar)
/* Creates a new menu on the menu bar. */
    {
    int ac;
    Arg al[10];
    Widget menu, cascade;

    ac = 0;
    menu = XmCreatePulldownMenu (menu_bar, menu_name, al, ac);

    ac = 0;
    XtSetArg (al[ac], XmNsubMenuId, menu); ac++;
    XtSetArg (al[ac], XmNmnemonic, mnemonic); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR(menu_name, char_set)); ac++;
    cascade = XmCreateCascadeButton (menu_bar, menu_name, al, ac);
    XtManageChild (cascade); 

    ac=0;
    XtSetArg(al[ac],XmNmenuHelpWidget,cascade); ac++;
    XtSetValues(menu_bar,al,ac);

    return(menu);
    }

void add_accelerator(Widget w, char *acc_text, char *key)
/* adds an accelerator to a menu option. */
    {
    int ac;
    Arg al[10];

    ac=0;
    XtSetArg(al[ac],XmNacceleratorText,
        XmStringCreate(acc_text,char_set)); ac++;
    XtSetArg(al[ac],XmNaccelerator,key); ac++;
    XtSetValues(w,al,ac);
    }

Widget make_menu_option(char *option_name, KeySym mnemonic, int client_data, Widget menu)
/* Adds an option in a menu. */
    {
    int ac;
    Arg al[10];
    Widget b;

    ac = 0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR(option_name,
        char_set)); ac++;
    XtSetArg (al[ac], XmNmnemonic, mnemonic); ac++;
    b=XtCreateManagedWidget(option_name,xmPushButtonWidgetClass,menu,al,ac);
    XtAddCallback(b, XmNactivateCallback, menuCB, client_data);
    return(b);
    }

Widget make_menu_toggle(char *option_name, int client_data, Widget menu)
/* Adds a toggle in a menu. */
    {
    int ac;
    Arg al[10];
    Widget b;

    ac = 0;
    XtSetArg(al[ac], XmNlabelString,XmStringCreateLtoR(option_name,
        char_set)); ac++;
    XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
    b=XtCreateManagedWidget(option_name,xmToggleButtonGadgetClass,menu,al,ac);
    XtAddCallback(b, XmNvalueChangedCallback, menuCB, client_data);
    return(b);
    }

Widget make_menu(char *menu_name, KeySym mnemonic, Widget menu_bar)
/* Creates a new menu on the menu bar. */
    {
    int ac;
    Arg al[10];
    Widget menu, cascade;

    ac = 0;
    menu = XmCreatePulldownMenu (menu_bar, menu_name, al, ac);

    ac = 0;
    XtSetArg (al[ac], XmNsubMenuId, menu); ac++;
    XtSetArg (al[ac], XmNmnemonic, mnemonic); ac++;
    XtSetArg(al[ac], XmNlabelString,
             XmStringCreateLtoR(menu_name, char_set)); ac++;
    cascade = XmCreateCascadeButton (menu_bar, menu_name, al, ac);
    XtManageChild (cascade); 

    return(menu);
    }

void create_menus(Widget menu_bar)
    {
    int ac;
    Arg al[10];
    Widget menu;

    menu=make_menu("Control",'C',menu_bar);
    login_option = make_menu_option("Connect...",'C',LOGIN,menu);
    add_accelerator(login_option,"meta+c","Meta<Key>c:");
    disconnect_option = make_menu_option("Disconnect",'D',DISCONNECT,menu);
    add_accelerator(disconnect_option,"meta+d","Meta<Key>d:");
    local_dir_option = make_menu_option("Change Download Dir...",'C',LOCALDIR,menu);
    add_accelerator(local_dir_option,"meta+c","Meta<Key>c:");
    ftp_status_option = make_menu_option("Verbose",'V',STATUS,menu);
    add_accelerator(ftp_status_option,"meta+v","Meta<Key>v:");
    make_menu_separator(menu);
    quit_option = make_menu_option("Exit",'E',QUIT,menu);
    add_accelerator(quit_option,"meta+q","Meta<Key>q:");

    menu=make_help_menu("Help",'H',menu_bar);
    help_option=make_menu_option("Help",'H',HELP,menu);
    add_accelerator(help_option,"meta+h","Meta<Key>h:");
    XtAddCallback(help_option,XmNactivateCallback,XnHelpCB,"Index");
    }

void watch_cursor(Widget w)
/* change the cursor to a wrist watch shape. */
    {
    Cursor c1;

    c1 = XCreateFontCursor(XtDisplay(w),XC_watch);
    XDefineCursor(XtDisplay(w),XtWindow(w),c1);
    XFlush(XtDisplay(w));
    }

/* a simple bubble sort for the user's custom list (since qsort
   doesn't like arrays of pointers) */
void sort(char **list, int count)
   {
   int i, j;
   char *temp;

   for(i=0; i < count; i++)
       for(j = 0; j < (count-i); j++)
          if((j < (count-1)) && (strcmp(list[j+1],list[j]) < 0))
              {
              temp=list[j+1];
              list[j+1]=list[j];
              list[j]=temp;
              }
   }

void list_editor_CB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {
    Arg al[5];
    int ac, count, i;
    static XmString site_xmstr;
    XmString xmstr=NULL, *table;
    static Boolean selected=False, file_bool=False;
    FILE *f;
    static char nxftprc[100], logname[10];
    char *temp, temp2[100], **list;
    struct passwd *pwent;

    switch (client_data)
        {
        case OK:   /* save */
           if(site_xmstr)
               {
               XmStringFree(site_xmstr);
               site_xmstr=NULL;
               }

           if (!file_bool)
               {
               pwent = getpwuid(getuid());
               strcpy(logname, pwent->pw_name);

               /* This is a macro defined at the beginning of this file. */
               GetPath();
               file_bool=True;
               }

            if ((f=fopen(nxftprc,"w+")) == NULL)
               {
               error("Error in writing to custom site list file.  File not saved.\n");
               return;
               }

            ac=0;
            XtSetArg(al[ac], XmNitemCount, &count); ac++;
            XtGetValues(list_editor_user_site_list, al, ac);

            for(i=1; i <= count; i++)
                {
                XmListSelectPos(list_editor_user_site_list, i, False);
                
                XtSetArg(al[0], XmNselectedItems, &table);
                XtGetValues(list_editor_user_site_list, al, 1);

                XmStringGetLtoR(table[0], char_set, &temp);
                fprintf(f, "%s\n", temp);
                XmStringFree(table[0]);
                XtFree(temp);
                }
            fflush(f);
            fclose(f);

            XtSetSensitive(list_editor_save_button, False);
            break;

        case SELECT:   /* from list */
            if(site_xmstr)
                XmStringFree(site_xmstr);

            site_xmstr = XmStringCopy (((XmListCallbackStruct *)call_data)->item);
            XmStringGetLtoR(site_xmstr,char_set,&temp);
            selected = True;
            break;

        case ADD:
            /* get the entry to add */
            temp=XmTextGetString(list_editor_add_text);
            if (!strlen(temp))
                return;
    
            ac=0;
            XtSetArg(al[ac], XmNitemCount, &count); ac++;
            XtGetValues(list_editor_user_site_list, al, ac);
            count++;

            /* allocate an array of char *'s */
            list = (char **)malloc(sizeof(char *) * count);

            /* get the entry to add */
            XmTextSetString(list_editor_add_text, "");
            sscanf(temp,"%s", temp2);   /* in case there are spaces in there */
            if(!strlen(temp2))
               return;
            list[0] = (char *)XtMalloc(sizeof(char) * strlen(temp2));
            strcpy(list[0], temp2);

            if(site_xmstr)
                {
                XmStringFree(site_xmstr);
                site_xmstr=NULL;
                }

            /* get all current entries */
            for(i=1; i < count; i++)
                {
                XmListSelectPos(list_editor_user_site_list, i, False);
                
                XtSetArg(al[0], XmNselectedItems, &table);
                XtGetValues(list_editor_user_site_list, al, 1);

                XmStringGetLtoR(table[0], char_set, &temp);
                list[i] = temp;
                XmStringFree(table[0]);
                }

            /* sort list */
            sort(list, count);

            /* put sorted list in list widgets */
            XmListDeleteAllItems(list_editor_user_site_list);
            for(i=0; i < count; i++)
                {
                xmstr=XmStringCreateLtoR(list[i], char_set);
                XmListAddItemUnselected(list_editor_user_site_list,xmstr,0);
                XmStringFree(xmstr);
                XtFree(list[i]);
                }
            free(list);

            XtSetSensitive(list_editor_save_button, True);
            break;

        case REMOVE:
            if (selected)
                {
                XmListDeleteItem(list_editor_user_site_list,site_xmstr);
                XmStringFree(site_xmstr);
                site_xmstr=NULL;
                XtSetSensitive(list_editor_save_button, True);
                selected=False;
                }
            break;

        case CANCEL:
            XtUnmanageChild(list_editor_dialog);

            if(site_xmstr)
                {
                XmStringFree(site_xmstr);
                site_xmstr=NULL;
                }

            XmListDeselectAllItems(list_editor_user_site_list);
            XmListDeleteAllItems(login_site_list);

            ac=0;
            XtSetArg(al[ac], XmNitemCount, &count); ac++;
            XtGetValues(list_editor_user_site_list, al, ac);

            /* get all current entries and put in login dialog list */
            for(i=1; i <= count; i++)
                {
                XmListSelectPos(list_editor_user_site_list, i, False);
                
                XtSetArg(al[0], XmNselectedItems, &table);
                XtGetValues(list_editor_user_site_list, al, 1);

                XmListAddItemUnselected(login_site_list, table[0], 0);
                XmStringFree(table[0]);
                }

            break;
        }
    }

void create_list_editor_dialog()
    {
    Arg al[15];
    int ac;
    static Widget label, sep;
    char *file_contents, temp[100], logname[10], nxftprc[200];
    FILE *f;
    int file_length;
    struct stat stat_val;
    XmString xmstr;
    struct passwd *pwent;

    ac=0;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR("NXftp: List Editor",char_set)); ac++;
    XtSetArg(al[ac], XmNautoUnmanage, False); ac++;
    list_editor_dialog=XmCreatePromptDialog(toplevel,"list_editor_dialog",al,ac);

    XtUnmanageChild(XmSelectionBoxGetChild(list_editor_dialog, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(list_editor_dialog, XmDIALOG_PROMPT_LABEL));
    XtUnmanageChild(XmSelectionBoxGetChild(list_editor_dialog, XmDIALOG_HELP_BUTTON));

    ac=0;
    list_editor_form=XmCreateForm(list_editor_dialog, "list_editor_dialog", al, ac);
    XtManageChild(list_editor_form);

    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR("Site To Add", char_set));    ac++;
    label=XmCreateLabelGadget(list_editor_form,"label",al,ac);
    XtManageChild(label);

    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, label); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    list_editor_add_text=XmCreateText(list_editor_form,"list_editor_add_text",al,ac);
    XtManageChild(list_editor_add_text);

    ac=0;
    XtSetArg(al[ac], XmNtopOffset, 10); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, list_editor_add_text); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR(" Add to List ", char_set)); ac++;
    list_editor_add_button=XmCreatePushButtonGadget(list_editor_form, "list_editor_add_button",
        al, ac);
    XtManageChild(list_editor_add_button);
    XtAddCallback(list_editor_add_button, XmNactivateCallback, list_editor_CB, ADD);

    ac=0;
    XtSetArg(al[ac], XmNtopOffset, 10); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, list_editor_add_button); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR(" Remove from List ", char_set)); ac++;
    list_editor_remove_button=XmCreatePushButtonGadget(list_editor_form, 
        "list_editor_remove_button", al, ac);
    XtManageChild(list_editor_remove_button);
    XtAddCallback(list_editor_remove_button, XmNactivateCallback, list_editor_CB, REMOVE);

    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNleftPosition, 60); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR("User Site List", char_set)); ac++;
    label=XmCreateLabelGadget(list_editor_form,"label",al,ac);
    XtManageChild(label);

    ac=0;
    XtSetArg(al[ac], XmNheight, 150); ac++;
    XtSetArg(al[ac], XmNwidth, 175); ac++;
    XtSetArg(al[ac], XmNlistSizePolicy, XmCONSTANT); ac++;
    XtSetArg(al[ac], XmNselectionPolicy, XmSINGLE_SELECT); ac++;
    XtSetArg(al[ac], XmNscrollBarDisplayPolicy, XmSTATIC); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, label); ac++;
    XtSetArg(al[ac], XmNleftOffset, 25); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNleftWidget, list_editor_add_text); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    list_editor_user_site_list=XmCreateScrolledList(list_editor_form,
        "list_editor_user_site_list",al,ac);
    XtManageChild(list_editor_user_site_list);
    XtAddCallback(list_editor_user_site_list, XmNsingleSelectionCallback, list_editor_CB,
        SELECT);

    ac=0;
    XtSetArg(al[ac], XmNtopOffset, 10); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, list_editor_user_site_list); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sep=XmCreateSeparatorGadget(list_editor_form, "sep", al, ac);
    XtManageChild(sep);

    ac=0;
    XtSetArg(al[ac], XmNtopOffset, 5); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, sep); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR("General Site List With Descriptions", char_set)); ac++;
    label=XmCreateLabelGadget(list_editor_form,"label",al,ac);
    XtManageChild(label);

    ac=0;
    XtSetArg(al[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
    XtSetArg(al[ac], XmNeditable, False); ac++;
    XtSetArg(al[ac], XmNrows, 10); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, label); ac++; 
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    list_editor_general_site_text=XmCreateScrolledText(list_editor_form,
        "list",al,ac);
    XtManageChild(list_editor_general_site_text);

    list_editor_save_button=XmSelectionBoxGetChild(list_editor_dialog, XmDIALOG_OK_BUTTON);
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR("Save", char_set)); ac++;
    XtSetValues(list_editor_save_button, al, ac);
    XtAddCallback(list_editor_dialog, XmNokCallback, list_editor_CB, OK);
    XtSetSensitive(list_editor_save_button, False);

    list_editor_cancel_button=XmSelectionBoxGetChild(list_editor_dialog, XmDIALOG_CANCEL_BUTTON);     
    XtAddCallback(list_editor_dialog, XmNcancelCallback, list_editor_CB, CANCEL);

    /*load anonymous ftp site list*/
    
    if (stat(SITE_LIST, &stat_val) == 0)
        {
        file_length=stat_val.st_size;

        /* try to open file in "r" mode. if OK then read it. */
        
        if ((f=fopen(SITE_LIST,"r")) == NULL)
           {
           error("NXFtp Error:  General site list file could not be opened.\n");
           normal_cursor(toplevel);
           return;
           }

        /* malloc a place for the string to be read to. */
        file_contents=(char *) XtMalloc(file_length+1);
        *file_contents='\0';

        /* read the file string */
        fread(file_contents, sizeof(char), file_length, f);
        fclose(f);
        file_contents[file_length]='\0';

        /* give the string to the text widget. */
        XmTextSetString(list_editor_general_site_text,file_contents);

        XtFree(file_contents);
        }

    /* put the custom site files from ~/.nxftprc in the two user site list widgets;
       this is a macro defined at the top of this file */
    pwent = getpwuid(getuid());
    strcpy(logname, pwent->pw_name);
    GetPath();

    if ((f=fopen(nxftprc,"r")) == NULL)
        return;

    while (fscanf(f,"%s",temp) != EOF)
        {
        xmstr=XmStringCreateLtoR(temp, char_set);
        XmListAddItemUnselected(login_site_list,xmstr,0);
        XmListAddItemUnselected(list_editor_user_site_list,xmstr,0);
        XmStringFree(xmstr);
        }
    fclose(f);
    }

void login_CB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
/* callback for any button in the find dialog box. */
    {
    char *temp, mail_id[25], s[50];
    Link_Handle temp_link;

    switch (client_data)
        {
        case OK:
            login_ok();
            break;
        case ANONYMOUS:
            watch_cursor(toplevel);
            watch_cursor(login_dialog);
            XmTextSetString(login_edit2, "anonymous");
            XmTextSetString(login_edit3, eos_adr());
            normal_cursor(login_dialog);
            normal_cursor(toplevel);
            login_ok();
            break;
        case SELECT:
            XmStringGetLtoR(((XmListCallbackStruct *)call_data)->item,char_set,&temp);
            XmTextSetString(login_edit1, temp);
            XtFree(temp);
            break;
        case EDIT:
            XtManageChild(list_editor_dialog);
            break;
        case CANCEL:
            XtUnmanageChild(login_dialog);
            break;
        }
    }

void create_login_dialog()
/* creates all of the widgets in the login dialog box. */
    {
    static char translations1[]=
         "<Key>Return: next-tab-group()";
    static char translations2[]=
         "<Key>Return: next-tab-group()next-tab-group()";
    XtTranslations trans_table1,trans_table2;
    XFontStruct *font;
    XmFontList password_font = NULL;

    Arg al[10];
    int ac, background;
    static Widget login_dialog_bb, login_edit_button, label;

    ac=0;
    XtSetArg(al[ac], XmNdialogTitle,XmStringCreateLtoR("NXftp: Connect", char_set)); ac++;
    XtSetArg(al[ac], XmNautoUnmanage, False); ac++;
    login_dialog=XmCreatePromptDialog(toplevel,"login_dialog",al,ac);

    XtUnmanageChild(XmSelectionBoxGetChild(login_dialog, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(login_dialog, XmDIALOG_PROMPT_LABEL));

    ac=0;
    XtSetArg(al[ac], XmNheight, 285); ac++;
/*    XtSetArg(al[ac], XmNwidth, 300); ac++; */  
    XtSetArg(al[ac], XmNautoUnmanage, False); ac++;
    login_dialog_bb=XmCreateBulletinBoard(login_dialog,"login_dialog_bb",al,ac);
    XtManageChild(login_dialog_bb);

    /* create and manage the three labels. */
    ac=0;
    XtSetArg(al[ac], XmNx, 10); ac++;
    XtSetArg(al[ac], XmNy, 10); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR("FTP Site Name", char_set));    ac++;
    login_label1=XmCreateLabelGadget(login_dialog_bb,"login_label1",al,ac);
    XtManageChild(login_label1);
     
    ac=0;
    XtSetArg(al[ac], XmNx, 10); ac++;
    XtSetArg(al[ac], XmNy, 30); ac++;
    XtSetArg(al[ac], XmNmaxLength, 35); ac++;
    XtSetArg(al[ac], XmNwidth, 148); ac++;
    login_edit1=XmCreateText(login_dialog_bb,"login_edit1",al,ac);
    XtManageChild(login_edit1);

    ac=0;
    XtSetArg(al[ac], XmNx, 10); ac++;
    XtSetArg(al[ac], XmNy, 70); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR(" Anonymous Login ", char_set));    ac++;
    login_anon_button=XmCreatePushButtonGadget(login_dialog_bb,"login_anon_button",al,ac);
    XtManageChild(login_anon_button);
    XtAddCallback (login_anon_button, XmNactivateCallback, login_CB, ANONYMOUS);
     
    /* create a separator */
    ac=0;
    XtSetArg(al[ac], XmNx, 10); ac++;
    XtSetArg(al[ac], XmNy, 110); ac++;
    XtSetArg(al[ac], XmNwidth, 148); ac++;
    XtSetArg(al[ac], XmNmargin, 1); ac++;
    login_sep1=XmCreateSeparatorGadget(login_dialog_bb,"login_sep1",al,ac);
    XtManageChild(login_sep1);

    ac=0;
    XtSetArg(al[ac], XmNx, 10); ac++;
    XtSetArg(al[ac], XmNy, 115); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR("User ID", char_set));    ac++;
    login_label2=XmCreateLabelGadget(login_dialog_bb,"login_label2",al,ac);
    XtManageChild(login_label2);
     
    ac=0;
    XtSetArg(al[ac], XmNx, 10); ac++;
    XtSetArg(al[ac], XmNy, 135); ac++;
    XtSetArg(al[ac], XmNwidth, 148); ac++;
    login_edit2=XmCreateText(login_dialog_bb,"login_edit2",al,ac);
    XtManageChild(login_edit2);
     
    ac=0;
    XtSetArg(al[ac], XmNx, 10); ac++;
    XtSetArg(al[ac], XmNy, 175); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR("Password", char_set));    ac++;
    login_label3=XmCreateLabelGadget(login_dialog_bb,"login_label3",al,ac);
    XtManageChild(login_label3);

    font = XLoadQueryFont(XtDisplay(toplevel), "nil2");
    ac=0;
    if (font)
        {
        password_font = XmFontListCreate(font, char_set);

        XtSetArg(al[ac], XmNfontList, password_font); ac++;
        XtSetArg(al[ac], XmNheight, 31); ac++;
        }
    XtSetArg(al[ac], XmNwidth, 148); ac++;
    XtSetArg(al[ac], XmNx, 10); ac++;
    XtSetArg(al[ac], XmNy, 195); ac++;
    login_edit3=XmCreateText(login_dialog_bb,"login_edit3",al,ac);
    XtManageChild(login_edit3);

    login_ok_button=XmSelectionBoxGetChild(login_dialog, XmDIALOG_OK_BUTTON);
    XtAddCallback(login_dialog, XmNokCallback, login_CB, OK);

    login_cancel_button=XmSelectionBoxGetChild(login_dialog, XmDIALOG_CANCEL_BUTTON);
    XtAddCallback(login_dialog, XmNcancelCallback, login_CB, CANCEL);

/*-----*/
    login_edit_button=XmSelectionBoxGetChild(login_dialog, XmDIALOG_HELP_BUTTON);
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR("Edit List", char_set)); ac++;
    XtSetValues(login_edit_button, al, ac);
    XtAddCallback(login_edit_button, XmNhelpCallback, login_CB, EDIT);

    ac=0;
    XtSetArg(al[ac], XmNx, 175); ac++;
    XtSetArg(al[ac], XmNy, 10); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR("User Site List", char_set));    ac++;
    label=XmCreateLabelGadget(login_dialog_bb,"label",al,ac);
    XtManageChild(label);

    ac=0;
    XtSetArg(al[ac], XmNx, 175); ac++;
    XtSetArg(al[ac], XmNy, 31); ac++;
    XtSetArg(al[ac], XmNheight, 193); ac++;
    XtSetArg(al[ac], XmNwidth, 160); ac++;
    XtSetArg(al[ac], XmNselectionPolicy, XmSINGLE_SELECT); ac++;
    XtSetArg(al[ac], XmNlistSizePolicy, XmCONSTANT); ac++;
    login_site_list=XmCreateScrolledList(login_dialog_bb,"login_site_list",al,ac);
    XtManageChild(login_site_list);
    XtAddCallback (login_site_list, XmNsingleSelectionCallback, login_CB, SELECT);

   /* change translation so that <return> moves to next text field */
    trans_table1=XtParseTranslationTable(translations1);
    trans_table2=XtParseTranslationTable(translations2);
    XtOverrideTranslations(login_edit1, trans_table1);
    XtOverrideTranslations(login_edit2, trans_table1);
    XtOverrideTranslations(login_edit3, trans_table2);
 
    XtSetSensitive(login_edit1, True);
    }

void normal_cursor(Widget w)
/* return the cursor to its normal shape. */
    {
    XUndefineCursor(XtDisplay(w),XtWindow(w));
    XFlush(XtDisplay(w));
    }

void info_CB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
/* callback for Cancel button in the initial_info dialog box. */
    {
    switch (client_data)
        {
        case CANCEL:
            XtDestroyWidget(XtParent(XtParent(w)));
            break;
        default:
            XtDestroyWidget(w);   /* handles 'Close' from the window manager frame menu */
            break;
        }
    }

void show_info(char *title, char *icon_name, char *msg)
    {
    Arg al[10];
    int ac;
    Atom WM_DELETE_WINDOW;
    Pixmap pixmap;

    ac=0;
    XtSetArg(al[ac], XmNheight, 330); ac++;
    XtSetArg(al[ac], XmNwidth, 520); ac++;
    XtSetArg(al[ac], XmNtitle, title); ac++;
    XtSetArg(al[ac], XmNiconName, icon_name); ac++;
    info_shell=XtAppCreateShell("NXftp", "NXftp", 
        topLevelShellWidgetClass, XtDisplay(toplevel), al, ac);

    /* Create the icon pixmap for the toplevel. */
    pixmap=XCreateBitmapFromData(XtDisplay(toplevel), XtScreen(toplevel)->root,
        nxftp_text_bits, nxftp_text_width, nxftp_text_height);

    /* Set the icon pixmap for the info_shell. */
    if(pixmap != None)
        {
        ac=0;
        XtSetArg(al[ac], XmNiconPixmap, pixmap); ac++;
        XtSetValues(info_shell, al, ac);
        }

    ac=0;
    XtSetArg(al[ac], XmNautoUnmanage, False); ac++;
    XtSetArg(al[ac], XmNhorizontalSpacing, 2); ac++;
    XtSetArg(al[ac], XmNverticalSpacing, 2); ac++;
    initial_login_info=XmCreateForm(info_shell, "initial_login_info", al, ac);
    XtManageChild(initial_login_info);

    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR(" Dismiss ", char_set)); ac++;
    XtSetArg(al[ac], XmNshowAsDefault, True); ac++;
    info_cancel_button=XmCreatePushButtonGadget(initial_login_info,"info_cancel_button",
        al,ac);
    XtManageChild(info_cancel_button);
    XtAddCallback (info_cancel_button, XmNactivateCallback, info_CB, 
        CANCEL);

    /* attachments for info_cancel_button */
    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_NONE); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 58); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNleftPosition, 42); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomOffset, 10); ac++;
    XtSetValues(info_cancel_button,al,ac);

    ac=0;
    XtSetArg(al[ac], XmNvalue, msg); ac++;
    XtSetArg(al[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
    XtSetArg(al[ac], XmNeditable, False); ac++;
    XtSetArg(al[ac], XmNcolumns, 80); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNbottomWidget, info_cancel_button); ac++;
    XtSetArg(al[ac], XmNbottomOffset, 10); ac++;
    initial_info = XmCreateScrolledText(initial_login_info, "info_text", al, ac);
    XtManageChild(initial_info);

    XtRealizeWidget(info_shell);

   /* call info_CB when the user closes the window using the 'Close'
      selection on the window frame menu */
    WM_DELETE_WINDOW=XmInternAtom(XtDisplay(info_shell), "WM_DELETE_WINDOW", False);
    XmAddWMProtocolCallback(info_shell, WM_DELETE_WINDOW, info_CB, NULL);
    }

void get_dialog_CB(Widget w, int client_data, XmSelectionBoxCallbackStruct *call_data)
/* callback for any button in the get dialog box. */
    {
    char *file_to_get, name[50];

    switch (client_data)
       {
       case BINARY:
          binary();
          break;
       case ASCII:
          ascii();
          break;
       case OK:
          file_to_get = XmTextGetString(get_filename);
          if (sscanf(file_to_get, "%*d %s", name) != 1)
             sscanf(file_to_get, "%s", name);
          get_file(name);
          XtFree(file_to_get);
          break;
       case CANCEL:
          XtUnmanageChild(w);
          break;
       case READTEXT:
          file_to_get = XmTextGetString(get_filename);
          if (sscanf(file_to_get, "%*d %s", name) != 1)
             sscanf(file_to_get, "%s", name);
          read_file(name);
          XtFree(file_to_get);
          break;
       }
    }

void create_get_dialog()
    {
    Arg al[10];
    int ac;
    char temp[20];
    static Widget get_dialog_form;

    ac=0;
    XtSetArg(al[ac], XmNdialogTitle, XmStringCreateLtoR("NXftp: Get File",char_set)); ac++;
    XtSetArg(al[ac], XmNautoUnmanage, False); ac++;
    get_dialog=XmCreatePromptDialog(toplevel,"get_dialog",al,ac);

    XtUnmanageChild(XmSelectionBoxGetChild(get_dialog, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(get_dialog, XmDIALOG_PROMPT_LABEL));

    ac=0;
    get_dialog_form=XmCreateForm(get_dialog,"get_dialog_form",al,ac);
    XtManageChild(get_dialog_form);

    /* create and manage the label */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR("File to Get", char_set)); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    get_label1=XmCreateLabelGadget(get_dialog_form,"get_label1",al,ac);
    XtManageChild(get_label1);

    /* Create and manage the text widget */
    ac=0;
    XtSetArg(al[ac], XmNeditable, False); ac++;
    XtSetArg(al[ac], XmNtopOffset, 5); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, get_label1); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    get_filename=XmCreateText(get_dialog_form,"get_filename",al,ac);
    XtManageChild(get_filename);

    ac=0;
    XtSetArg(al[ac], XmNorientation,XmVERTICAL); ac++;
    XtSetArg(al[ac], XmNpacking,XmPACK_COLUMN); ac++;
    XtSetArg(al[ac], XmNnumColumns,1); ac++;
    XtSetArg(al[ac], XmNtopOffset, 5); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, get_filename); ac++;
    XtSetArg(al[ac], XmNleftOffset, 85); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    file_type_box=XmCreateRadioBox(get_dialog_form, "file_type_box", al, ac);
    XtManageChild(file_type_box);

    ac=0;
    XtSetArg(al[ac],XmNlabelString,XmStringCreateLtoR("Binary",char_set)); ac++;
    binary_button=XmCreateToggleButtonGadget(file_type_box,"binary_button",al,ac);
    XtManageChild(binary_button);
    XtAddCallback(binary_button,XmNvalueChangedCallback,get_dialog_CB,BINARY);

    ac=0;
    XtSetArg(al[ac],XmNset,True); ac++;
    XtSetArg(al[ac],XmNlabelString,XmStringCreateLtoR("ASCII",char_set)); ac++;
    ascii_button=XmCreateToggleButtonGadget(file_type_box,"ascii_button",al,ac);
    XtManageChild(ascii_button);
    XtAddCallback(ascii_button, XmNvalueChangedCallback,get_dialog_CB,ASCII);

    /* resources of buttons */
    get_ok_button=XmSelectionBoxGetChild(get_dialog, XmDIALOG_OK_BUTTON);
    XtAddCallback(get_dialog, XmNokCallback, get_dialog_CB, OK);

    get_cancel_button=XmSelectionBoxGetChild(get_dialog, XmDIALOG_CANCEL_BUTTON);
    XtAddCallback(get_dialog, XmNcancelCallback, get_dialog_CB, CANCEL);

    sprintf(temp,"Read Text\nFile");
    read_text_button=XmSelectionBoxGetChild(get_dialog, XmDIALOG_HELP_BUTTON);
    ac=0;
    XtSetArg(al[ac],XmNlabelString,XmStringCreateLtoR(temp,char_set)); ac++;
    XtSetValues(read_text_button, al, ac);
    XtAddCallback(get_dialog, XmNhelpCallback, get_dialog_CB, READTEXT);
    }

/*----------------------------------------------------------------------------*/

void update_quota(caddr_t data, XtIntervalId *id)
   {
   display_quota();

   XtAppAddTimeOut(context, 60000, update_quota, NULL);

   XmUpdateDisplay(rem_disk_quota);
   }

void display_quota()
   {
   char quota_str[20];
   Boolean once = False;

   strcpy(quota_str, quota());
   XmTextSetString(rem_disk_quota, quota_str);
   
   if (*quota_str == '-')
      {
      /* It is negative. */
      if (!once)
         error("You have exhausted your quota.");
      once = True;
      }
   else
      once = False;
   }

void disconnect()
   {
   Arg al[10];
   int ac;

   ftp_exit();

   /* reset the interface */
   XmTextSetString(site_name,"");
   XmTextSetString(site_dir,"");
   XmListDeleteAllItems(site_files_list);   
   XmListDeleteAllItems(site_dir_list);
   XmTextInsert(files_recvd,XmTextGetCursorPosition(files_recvd),"\n");

   ac=0;
   XtSetArg(al[ac], XmNset, False);ac++;
   XtSetValues(binary_button,al,ac);     
   
   ac = 0;
   XtSetArg(al[ac],XmNset,True); ac++;
   XtSetValues(ascii_button,al,ac);

   change_menu_sensitivity(False);

   ac=0;
   XtSetArg(al[ac], XmNiconName, "(no site)"); ac++;
   XtSetArg(al[ac], XmNtitle, TITLE); ac++;
   XtSetValues(toplevel, al, ac);
   }   

void cd_site_up()
   {
   Arg al[10];
   int ac, cnt;
   char temp[25];

   watch_cursor(toplevel);
   if(strcmp(site_dir_name, "/") == NULL)
      error("Root Directory--cannot go up.");
   else
      { 
      strcpy(site_dir_name,cd_remote(".."));
      if(site_dir_name == NULL) 
         error("Cannot change to parent directory.");
      else
         {
         XmTextSetString(site_dir,site_dir_name);
         XmTextShowPosition(site_dir, XmTextGetLastPosition(site_dir));
         XmListDeleteAllItems(site_files_list);   
         XmListDeleteAllItems(site_dir_list);
         form_lists();
         }
      }
   normal_cursor(toplevel);
   }

void cd_local_dir(char *dir)
   {
   char temp2[200];
   
   strcpy(temp2, cd_local(dir));
   if (*temp2 != '\0')
      {
      strcpy(curr_local_dir, temp2);
      XmTextSetString(local_dir, curr_local_dir);
      XmTextShowPosition(local_dir, strlen(curr_local_dir) - 1);
      display_quota();
      }
   else
      {
      sprintf(temp2, "Cannot change to %s.", dir);
      error(temp2);
      }      
   }

void cd_site_dir(char *dir)
   {
   Arg al[10];
   int ac, cnt;
   char temp[200], temp2[200];

   watch_cursor(toplevel);

   strcpy(temp, cd_remote(dir));
   if (strlen(temp))
      {
      strcpy(site_dir_name,temp);        
      XmTextSetString(site_dir,site_dir_name);
      XmTextShowPosition(site_dir, XmTextGetLastPosition(site_dir));

      XmListDeleteAllItems(site_files_list);   
      XmListDeleteAllItems(site_dir_list);
      form_lists();   
      }
   else
      {
      sprintf(temp2, "Permission denied:  %s", dir);
      error(temp2);
      }

   normal_cursor(toplevel);
   }

void login_ok()
   {
   Arg al[10];
   int ac, cnt;
   int  i, cmp_results, log_err;
   char s[200], local_pwd[200], display[3000], temp[50], local_dir_name[25],
        temp2[300];

   strcpy(site_name_string, XmTextGetString(login_edit1));
   strcpy(user_id, XmTextGetString(login_edit2));
   strcpy(password, XmTextGetString(login_edit3));

   /* check to see if information is in each field of the login dialog */
   if (site_name_string[0]=='\0')
      {
      XmProcessTraversal(login_edit1,XmTRAVERSE_CURRENT);
      XmProcessTraversal(login_edit1,XmTRAVERSE_CURRENT);
      return;
      }

   if (user_id[0]=='\0')
      {
      XmProcessTraversal(login_edit2,XmTRAVERSE_CURRENT);
      XmProcessTraversal(login_edit2,XmTRAVERSE_CURRENT);
      return;
      }

   if (password[0]=='\0')
      {
      XmProcessTraversal(login_edit3,XmTRAVERSE_CURRENT);
      XmProcessTraversal(login_edit3,XmTRAVERSE_CURRENT);
      return;
      }
  
   XtUnmanageChild(login_dialog);
   XmUpdateDisplay(form);
   XFlush(XtDisplay(form));

   XmTextSetString(site_name, site_name_string);
   watch_cursor(toplevel);

   if (ftp_connect_manual(site_name_string,user_id,password,local_pwd)==1)
      { 
      if ((log_err=initial_login_msg(s))!=NULL)
         {
         if (log_err==2)
            {
            error("Login attempt was unsuccessful.\nAccess denied.");
            time_out(0);
            disconnect();
            return;
            }

         if (strstr(s,"530 ")!=NULL)
           {
           /* close link if unsuccessful login */
           link_kill(&l);   
           error(s);
           time_out(0);
           disconnect();
           normal_cursor(toplevel);
           return;
           }
          
         *display='\0';
         strcat(display,s); 
         for (;(log_err=initial_login_msg(s));)
            {
            strcat(display,s);
            if (log_err == 2)
               {
               show_info("NXFtp:  Initial Connection Information", "NXFtp: INFO", display);
               error("Login attempt was unsuccessful.\nAccess denied.");
               disconnect();
               normal_cursor(toplevel);
               return;
               }
            }            

         show_info("NXFtp:  Initial Connection Information", "Initial Info", display);
         }

      change_menu_sensitivity(True);

      strcpy(site_dir_name, "/");
      XmTextSetString(site_dir, "/");
               
      form_lists();
      sprintf(temp, "Site:  %s\n", site_name_string);
      XmTextInsert(files_recvd,XmTextGetCursorPosition(files_recvd),temp);
      normal_cursor(toplevel);

      /* maintain local download directory if it has been set */
      if (strlen(curr_local_dir))
          cd_local_dir(curr_local_dir);
      else
          {
          strcpy(curr_local_dir, local_pwd);
          XmTextSetString(local_dir, local_pwd);
          }

      XmTextShowPosition(local_dir, XmTextGetLastPosition(local_dir));

      /* change title and icon name to reflect site connection */
      sprintf(temp2, "%s:  %s", TITLE, site_name_string);
      ac=0;
      XtSetArg(al[ac], XmNiconName, site_name_string); ac++;
      XtSetArg(al[ac], XmNtitle, temp2); ac++;
      XtSetValues(toplevel, al, ac);
      }
   else
      {
      XtUnmanageChild(login_dialog);
      disconnect();
      normal_cursor(toplevel);
      }                
   }

void get_file(char *file_to_get)
   { 
   char temp[100];

   watch_cursor(get_dialog);
   watch_cursor(toplevel);

   XtUnmanageChild(get_dialog);

   if(get(file_to_get))
      {   
      display_quota();
      strcat(file_to_get,"\n");
      XmTextInsert(files_recvd,XmTextGetCursorPosition(files_recvd),file_to_get);
      } 

   normal_cursor(get_dialog);
   normal_cursor(toplevel);
   }

void read_file(char *file_to_get)
   {
   Arg al[10];
   int ac, size;
   char temp[200], *bad_char, *file_contents;
   FILE *f;
   int file_length;
   struct stat stat_val;

   XtUnmanageChild(get_dialog);

   watch_cursor(toplevel);

   bad_char=strstr(file_to_get,"@");
   if (bad_char != (char *)NULL)
      *bad_char = '\0';

   bad_char=strstr(file_to_get,"*");
   if (bad_char != (char *)NULL)
      *bad_char = '\0';

   sprintf(temp, "%s /tmp/%s", file_to_get, file_to_get);

   if(get(temp)==1)
      {
      bad_char=strstr(file_to_get,".Z");
      if (bad_char != (char *)NULL)
         {
         sprintf(temp, "/usr/ucb/uncompress /tmp/\"%s\"", file_to_get);
         system(temp);
         *bad_char='\0';
         }

      sprintf(temp, "/tmp/%s", file_to_get);

      if (stat(temp, &stat_val) == 0)
        {
        file_length = stat_val.st_size;
        /* try to open file in "r" mode. if OK then read it. */
        if ((f=fopen(temp,"r"))==NULL)
            {
            error("File cannot be opened for reading.");

            sprintf(temp, "/bin/rm /tmp/\"%s\"", file_to_get);
            system(temp);     
            normal_cursor(toplevel);
            return;
            }

        /* malloc a place for the string to be read to. */
        file_contents = (char *) XtMalloc(file_length+1);
        *file_contents = '\0';
        /* read the file string */
        fread(file_contents, sizeof(char), file_length, f);
        fclose(f);
        file_contents[file_length]='\0';

        /* give the string to the text widget. */
        sprintf(temp, "NXFtp Read Text File:  %s", file_to_get);
        show_info(temp, file_to_get, file_contents);

        XtFree(file_contents);
        }

      sprintf(temp,"/bin/rm /tmp/\"%s\"",file_to_get);
      system(temp);     
      normal_cursor(toplevel);
      }

   else
      normal_cursor(toplevel);

   }

void main(int argc, char *argv)
    {
    Arg al[10];
    int ac;
    Pixmap        pixmap;

    setbuf(stdout,NULL);

    /* create the toplevel shell */
    
    toplevel = XtAppInitialize(&context,"NXftp",NULL,0,&argc,argv,fallback,NULL,0);

    XnInitializeHelp(context,toplevel,HELP_FILE,True);

    ac=0;
    XtSetArg(al[ac], XmNtitle, TITLE); ac++;
    XtSetArg(al[ac], XmNiconName, "(no site)"); ac++;
    XtSetArg(al[ac], XmNheight, 450); ac++;
    XtSetArg(al[ac], XmNwidth, 605); ac++;
    XtSetValues(toplevel,al,ac);

    /* Create the icon pixmap for the toplevel. */
    pixmap=XCreateBitmapFromData(XtDisplay(toplevel), XtScreen(toplevel)->root,
        nxftp_bits, nxftp_width, nxftp_height);

    /* Set the icon pixmap for the toplevel. */
    if( pixmap != None )
        {
        ac=0;
        XtSetArg(al[ac], XmNiconPixmap, pixmap); ac++;
        XtSetValues(toplevel, al, ac);
        }

    /* create a form to hold the other widgets */
    ac=0;
    XtSetArg(al[ac],XmNhorizontalSpacing,2); ac++;
    XtSetArg(al[ac],XmNverticalSpacing,3); ac++; 
    form=XmCreateForm(toplevel,"form",al,ac);
    XtManageChild(form);

    /* create a menu bar and attach it to the form. */
    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    menu_bar=XmCreateMenuBar(form,"menu_bar",al,ac);
    XtManageChild(menu_bar);

    create_menus(menu_bar);
    create_text_and_labels();
    create_list_widgets();
    create_get_dialog();
    create_error_dialog();
    create_ftp_output_dialog();
    create_prompt_dialog();

    XtRealizeWidget(toplevel);

    create_login_dialog();

    change_menu_sensitivity(False);

    strcpy(curr_local_dir, ".");
    display_quota();

    create_list_editor_dialog();

    /* don't start checking quota if not supported by the system */
    XtAppAddTimeOut(context, 60000, update_quota, NULL);

    XtAppMainLoop(context);
    }
