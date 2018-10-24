/*
 * NXHelp - demo.c
 *	by Lance Lovette
 *	Code and display copyright 1992, by Lance Lovette.
 *	All Rights Reserved
 *
 * This code demonstrates the creation of a help menu 
 * on the menu bar using the help facility described in the README.
*/

#include <Xm/Xm.h>
#include <Xm/Label.h> 
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>

#include "NXHelp.h"		/* Include file for help callbacks. */

#define	HELPFILE	"demo.hlp"

XtAppContext context;
XmStringCharSet char_set=XmSTRING_DEFAULT_CHARSET;

Widget toplevel, form, label, menu_bar;
Widget file_menu;
Widget quit_item;
Widget help_menu;
Widget overviewButton;
Widget aboutButton;
Widget deferredButton;
Widget hiddenButton;
Widget indexButton;

void menuCB(w,client_data,call_data)
    Widget w;
    char *client_data;
    XmAnyCallbackStruct *call_data;
/* callback routine used for all menus */
{
    if (strcmp(client_data,"Quit")==0) /* if quit seen, then exit */
        exit(0);
}

Widget make_menu_item(item_name,client_data,menu)
    char *item_name;
    caddr_t client_data;
    Widget menu;
/* adds an item into a menu. */ 
{
    int ac;
    Arg al[10];
    Widget item;

    ac = 0;
    XtSetArg(al[ac],XmNlabelString,
        XmStringCreateLtoR(item_name,char_set)); ac++;  
    item=XmCreatePushButton(menu,item_name,al,ac);
    XtManageChild(item);
    XtAddCallback(item,XmNactivateCallback,menuCB,client_data);  
    XtSetSensitive(item,True);
    return(item);
}

Widget make_menu(menu_name,menu_bar)
    char *menu_name; 
    Widget menu_bar;
/* creates a menu on the menu bar */
{
    int ac;
    Arg al[10];
    Widget menu, cascade;

    menu=XmCreatePulldownMenu(menu_bar,menu_name,NULL,0);
    ac=0;
    XtSetArg (al[ac],XmNsubMenuId, menu); ac++;
    XtSetArg(al[ac],XmNlabelString,
        XmStringCreateLtoR(menu_name,char_set)); ac++;
    cascade=XmCreateCascadeButton(menu_bar,menu_name,al,ac);  
    XtManageChild(cascade); 
    return(menu);
}

Widget make_help_menu(menu_name, menu_bar)
    char *menu_name;
    Widget menu_bar;
/* Creates a new menu on the menu bar. */
{
    int ac;
    Arg al[10];
    Widget menu, cascade;

    ac = 0;
    menu = XmCreatePulldownMenu (menu_bar, menu_name, al, ac);

    ac = 0;
    XtSetArg (al[ac], XmNsubMenuId, menu); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR(menu_name, XmSTRING_DEFAULT_CHARSET)); ac++;
    cascade = XmCreateCascadeButton (menu_bar, menu_name, al, ac);
    XtManageChild (cascade); 

    /* Wire the help menu into the rowcol widget's help menu resource. */
    ac=0;
    XtSetArg(al[ac],XmNmenuHelpWidget,cascade); ac++;
    XtSetValues(menu_bar,al,ac);

    return(menu);
}
void create_menus(menu_bar)
    Widget menu_bar;
/* creates all the menus for this program */
{
    /* create the file menu */
    file_menu=make_menu("File",menu_bar);
    quit_item=make_menu_item("Quit","Quit",file_menu);

    /* Create the help menu. */
    help_menu=make_help_menu("Help",menu_bar);

  /* Create some buttons and register their help callbacks. */
    aboutButton=XmCreatePushButton(help_menu, "About", NULL, 0);
    XtManageChild(aboutButton);
    XtAddCallback(aboutButton, XmNactivateCallback, XnHelpCB, "About");

    indexButton=XmCreatePushButton(help_menu, "Index", NULL, 0);
    XtManageChild(indexButton);
    XtAddCallback(indexButton, XmNactivateCallback, XnHelpCB, "Index");

    overviewButton=XmCreatePushButton(help_menu, "Overview", NULL, 0);
    XtManageChild(overviewButton);
    XtAddCallback(overviewButton, XmNactivateCallback, XnHelpCB, "Overview");

    deferredButton=XmCreatePushButton(help_menu, "Deferred", NULL, 0);
    XtManageChild(deferredButton);
    XtAddCallback(deferredButton, XmNactivateCallback, XnHelpCB, "Deferred");

    hiddenButton=XmCreatePushButton(help_menu, "Hidden", NULL, 0);
    XtManageChild(hiddenButton);
    XtAddCallback(hiddenButton, XmNactivateCallback, XnHelpCB, "Hidden");
}

void main(argc,argv)
    int argc; 
    char *argv[];
{
    Arg al[10];
    int ac;

    /* create the toplevel shell */
    toplevel = XtAppInitialize(&context,"",NULL,0,&argc,argv,NULL,NULL,0);

    /* Initialize the XnHelpCB callback. */
     XnInitializeHelp(context, toplevel, HELPFILE, True);

    /* resize the window */
    ac=0;
    XtSetArg(al[ac],XmNheight,200); ac++;
    XtSetArg(al[ac],XmNwidth,200); ac++;
    XtSetValues(toplevel,al,ac);

    /* create a form widget */
    ac=0;
    form=XmCreateForm(toplevel,"form",al,ac);
    XtManageChild(form);

    /* create a label widget */
    ac=0;
    XtSetArg(al[ac],XmNlabelString,
        XmStringCreate("I'm a label", char_set)); ac++;
    label=XmCreateLabel(form,"label",al,ac);
    XtManageChild(label);

    /* create the menu bar */
    ac=0;
    menu_bar=XmCreateMenuBar(form,"menu_bar",al,ac);
    XtManageChild(menu_bar);

    /* attach the menu bar to the form */
    ac=0;
    XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM); ac++;
    XtSetArg(al[ac],XmNrightAttachment,XmATTACH_FORM); ac++;
    XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM); ac++;
    XtSetValues(menu_bar,al,ac);

    /* attach the label to the form */
    ac=0;
    XtSetArg(al[ac],XmNtopAttachment,XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac],XmNtopWidget,menu_bar); ac++;
    XtSetArg(al[ac],XmNrightAttachment,XmATTACH_FORM); ac++;
    XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM); ac++;
    XtSetArg(al[ac],XmNbottomAttachment,XmATTACH_FORM); ac++;
    XtSetValues(label,al,ac);

    create_menus(menu_bar);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(context);
}
