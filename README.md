# nxftp
NXftp is an X/Motif wrapper around the ftp command line that I wrote that started as a class project in 1992.
----


NOTE:  Even if you don't want to read anything, READ the INSTRUCTIONS
       section at the end because it will tell you how to set up the 
       program to use the name of your network and the help and site
       list files.

<pre>
**********************************************************************
   OO    O   OOOOO   OOOO   O    O     The North Carolina State
   O O   O  O       O       O    O     University Motif Group
   O  O  O  O        OOOO   O    O     NCSU Box 8206
   O   O O  O            O  O    O     Raleigh, NC 2695-8206
   O    OO   OOOOO   OOOO    OOOO

   O     O  OOOO  OOOOOOO O  OOOOO      OOOO  OOOO   OOO  O    O OOOO
   OO   OO O    O    O    O  O         O    O O   O O   O O    O O   O
   O O O O O    O    O    O  OOOOO     O      OOOO  O   O O    O OOOO
   O  O  O O    O    O    O  O         O  OOO O O   O   O O    O O
   O     O O    O    O    O  O         O    O O  O  O   O O    O O
   O     O  OOOO     O    O  O          OOOO  O   O  OOO   OOOO  O
 **********************************************************************

   Title: nxftp

   File names: ftp.c ftpcomm.c link.c ./nxhelp/nxhelp.c
   File Type : Code

   Lead Programmer: Eugene Hodges (ewhodges@eos.ncsu.edu)

   Testing by: Eugene Hodges, many users on EOS at NCSU

   Version: 3.1
   Date   : 6/8/93

   Description:
	NXftp is a Motif front end for the standard ftp command in Unix. 
        It allows the user to double click on a directory name to change
        directories, on a file to download or read it online if it is text,
        and change the directory in which files are placed.  This is a much
        needed change for the ftp user.  Furthermore, it allows the user to 
        easily create custom site lists to prevent having to re-enter the site
        names commonly used.

        New features:

           *  Interface has been reorganized -- better display of info!!
           *  The custom user list is sorted
           *  Much better stability and compatibility with sites
           *  Symbolic links in the site dir are properly separated into files
              and directories
           *  Remaining disk space function works with both AFS and non-AFS
              Unix systems now
           *  Improved error messages

   Author's notes:
	This program is one of those hard-to-make-always-work programs since
        it relies on the hope of a little uniformity among ftp sites.  This
        hope sometimes gets smashed.  This program works on EOS to a great
        extent.  When an ftp site does something that is not known to the 
        program, however, the program may crash.  Thankfully, the program 
        seems to work on the vast majority of ftp anonymous sites and is 
        generally a pleasure to use.

        One more thing, this program doesn't always work with all ftp 
        programs.  In other words, it may not work with the ftp program on
        your system.  If this is the case, you may need to modify the code
        to make NXftp work.  I hope this will not be the case, though.

   System Information:
        This text and code is based on the OSF/Motif widget set version
        1.1 and X11R4.

   About the NCSU Motif Group:
        The NCSU Motif Group is a group of students at North Carolina
        State University who meet once a week to discuss Motif programming.
        Individual projects are used to help students learn Motif.
        These projects are then released onto the network to help others.
        All released code is available at our anonymous FTP site:
        osl.csc.ncsu.edu, in the directory pub/ncsu_motif.  Questions
        and comments should be directed to the lead programmer for the
        project in question, or to Marshall Brain (brain@eos.ncsu.edu).

   Copyright Information:
        Copyright 1992, 1993 by Eugene Hodges
        All Rights Reserved

        Permission to use, copy, and distribute this software and text for
        NON-commercial purposes and without fee is hereby granted, provided
        that this notice appears in all copies and that modified or
        incomplete versions are not distributed.

        The author disclaims all warranties with regard to the software or
        text including all implied warranties of merchantability and fitness.

        In no event shall the author or NCSU be liable for any special,
        indirect or cosequential damages or any damages whatsoever
        resulting from loss of use, data or profits, whether in an
        action of contract, negligence or other tortious action,
        arising out of or in connection with the use or performance
        of this software or text.


DEBUGGING

        Some people have had trouble with the function link_input_waiting()
        hanging the program.  If you find this to be the case (or if the
        program is hanging and you don't know what else to try), replace the
        link_input_waiting() function in link. with the following:

        int link_input_waiting(struct link_handle *l)
        {
        return(0);  /* effectively removes the function, since it will always
                       return 0 */
        }

        Removing this function call this way should not cause any problems
        with nxftp in general, as the code that uses it is only there to try
        catch trash from any weird sites.

        Note:  This change was necessary for at least one person using 
               Openwin 3 on a SPARC ELC station to get nxftp to work.

        Feel free to send me email if it hangs with you and you don't know
        why, and I will give you whatever help that I can.  Hopefully the code
        is commented well enough to help on most problems.


INSTRUCTIONS

        For this program to function properly, change 5 things:

       *  1.  The #define SITE_LIST in ftp.c to the location it will have on
              your system
       *  2.  The #define HELP_FILE in ftp.c to the location it will have on
              your system
       *  3.  The #define GetPath() (a macro) in ftp.c must be changed to 
              something that will determine the path for the custom site list
              that each user can have (must be in the user's directory).
       *  4.  The #define EMAIL_ID in ftpcomm.c to the name of your system 
              (email address).
       *  5.  The DEFINES = -DNO_AFS should be uncommented for systems without
              AFS (otherwise, nxftp will attempt to execute "fs lq").

	Use the Imakefile to compile the program by first typing 'xmkmf' to
        build a Makefile for your system and then type 'make' to compile.  You
        will need to have the Motif widget set available on your system.  Make
        the appropriate changes to Imakefile if it does not accurately reflect
        your system (hopefully, this will not be a problem).

	Running the program should be quite straightforward.  Simply enter the
        name of the site you wish to log into and hit the anonymous button if 
	it is an "anonymous" ftp site.  To select files or directories, double 
        click on them.  In the case of text files or compressed text files, the
        option Read Text File in the Get File dialog will pull up the text file
	in a separate window.  The Change Download Dir option in the Control
        menu will allow you to change the directory in which files received will
        be placed.  The Edit List button in the Connect dialog allows you to
        create a custom site list.

	Good luck and enjoy!

        Buck. (ewhodges@eos.ncsu.edu)

        Note:  After August 1993, I will be a graduate student at Univ. of
               Ill. at Urbana-Champaign, so the email address is not valid
               for much longer.
</pre>
