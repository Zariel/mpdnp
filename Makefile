CFLAGS = -Wall -ggdb
LFLAGS = -lmpd
SOURCES = mpdnp.c

mpdnp:
	cc $(CFLAGS) $(LFLAGS) $(SOURCES) -o $@

clean:
	rm -f *.o mpdnp
