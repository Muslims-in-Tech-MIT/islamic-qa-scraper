CC=g++
CFLAGS=`xml2-config --cflags --libs` -lcurl -std=c++11
LIBS=`xml2-config --libs`

main: main.cpp
	$(CC) -o main main.cpp $(CFLAGS) $(LIBS)

clean:
	rm -f main
