
all:
	gcc -o server server.c parse.c database.c network.c user.c `xml2-config --cflags --libs` -pthread -lsqlite3
	gcc client.c -o client

clean:
	rm -f *~ server client
