CFLAGS:= -std=c99 -Wall
smartsys: admsys.c usersys.c smartsys.c
	gcc admsys.c usersys.c smartsys.c -I/usr/local/mysql/include -L/usr/local/mysql/lib -lmysqlclient -o smartsys
