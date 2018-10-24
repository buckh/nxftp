/*
 *  NXFTP - global.h
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

#include "link.h"

/* declare external the variable Link_Handle l so that both ftpcomm.c and
list_handler.c can use the same l; l is used as the main link, the link
between ftpshell and ftp */
 
Link_Handle l;

char curr_local_dir[200];
