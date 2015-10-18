CC=/usr/bin/g++ -g

INCLOCLIST=-I.
LIBLOCLIST=

C_FILES=spell.cc
HEADERS=spell.h

O_FILES=$(C_FILES:.cc=.o)

BIN=spell

CFLAGS=
MOD_FLAGS=

CLEAN=$(BIN) *.o *~

%.o: %.cc $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(MOD_CFLAGS) $(INCLOCLIST)

link=$(CC) -o $(1) $(2) $(CFLAGS) $(LIBLOCLIST) $(LIBLIST)

# ============================================================================
build: $(BIN)
	@echo Build ok

$(BIN): $(O_FILES)
	$(call link, $(BIN), $(O_FILES))

clean:
	rm -rf $(CLEAN)
