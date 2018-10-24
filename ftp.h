/*
 *  NXFTP - ftp.h
 *     Version 3.0
 *     by Eugene Hodges (ewhodges@eos.ncsu.edu)
 *
 *     Copyright 1992, 1993 by Eugene Hodges
 *     All Rights Reserved
 *
 *     Permission to use, copy, and distribute this software and text for
 *     non-commercial purposes and without fee is hereby granted, provided
 *     that this notice appears in all copies.
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

#include <Xm/Xm.h>

void display_quota();
void exit();
void disconnect();
void cd_site_up();
void cd_to_home_dir();
void cd_site_dir(char *dir);
void login_ok();
void anonymous_login();
void get_file(char *file_to_get);
void add_files();
void add_dirs();
void watch_cursor(Widget w);
void normal_cursor(Widget w);
void cd_local_dir(char *dir);
void read_file(char *file_to_get);
void error(char *s);
void files_select_CB(Widget w, int client_data, XmListCallbackStruct *call_data);
void dir_select_CB(Widget w, int client_data, XmListCallbackStruct *call_data);
