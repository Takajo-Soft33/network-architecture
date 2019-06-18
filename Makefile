
httpd:

all:lib httpd
cleanall:clean libclean

#httpd.nofunc http.only

httpd:httpd.c
	gcc -Wall httpd.c libhttpd.a -o httpd

httpd.only:httpd.c
	gcc -Wall httpd.c -o httpd.only

httpd.nofunc:httpd.nofunc.c libhttpd.a
	gcc -Wall httpd.nofunc.c libhttpd.a -o httpd.nofunc

lib:libhttpd.a

libhttpd.a:
	(cd ~/himitsu/server/lib;make)
	(cp ~/himitsu/server/lib/libhttpd.a .)
libclean:
	(cd ~/himitsu/server/lib;make clean)
	rm -f libhttpd.a
clean:
	rm -f *~  httpd httpd.only httpd.nofunc
