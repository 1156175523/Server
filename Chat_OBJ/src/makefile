server_tcp:
	gcc -g -o ../bin/server_tcp main.c func_handler.c server_tcp.c mmysql.h mmysql.c public.c\
		-I ./ -lpthread -I ~/ThirdInc/mysql/ -L ~/ThirdLib/mysql -lmysqlclient

clean:
	rm ../bin/server_tcp
