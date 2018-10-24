#  You may need to change the HELPFILE define in demo.c to the path
#  of the helpfile.  Other than that, all you need to do is type 'make'.
#  This Makefile will generate the help library libNXhelp.a and compile
#  a demo program using the help facility named 'demo'.

CC = cc
OBJS1 = nxhelp.o
OBJS2 = demo.o
PROGRAMS = libNXhelp.a demo
XLIBS = -lXm -lXt -lX11

all:: $(PROGRAMS)

libNXhelp.a: $(OBJS1)
	rm -f $@
	ar ruv $@ $(OBJS1)
	ranlib $@
	rm -f $(OBJS1)

demo: $(OBJS2)
	cc -o $@ $(OBJS2) libNXhelp.a $(XLIBS) 
