CFLAGS = -Wall -ggdb
LFLAGS = -lmpd
SOURCES = mpdnp.c

mpdnp:
	$(CLAGS) $(LFLAGS) $(SOURCES)

clean:
	rm -f *.o mpdnp
