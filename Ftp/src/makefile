all:../bin/ftp_server ../bin/ftp_client
../bin/ftp_server:server.c
	gcc -g server.c -o ../bin/ftp_server
../bin/ftp_client:client.c
	gcc -g client.c -o ../bin/ftp_client -lpthread
