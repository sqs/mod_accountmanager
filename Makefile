export CC=gcc
export CFLAGS=-Wall -Werror -g

export SRC=mod_accountmanager.c
export OBJ=$(SRC:.c=.o)
LDFLAGS=$(LIBS)

.PHONY: test clean restart

.SUFFIXES: .c

default: build

build: $(SRC)
	apxs2 $(INCLUDES) $(LIBS) $(LDFLAGS) -cia -Wc,-g $(SRC)

clean:
	rm -f *.o *.so *.slo *.lo *.la *.pyc
	rm -rf .libs/

buildre: build restart

restart:
	sudo /etc/init.d/apache2 restart

test:
	$(MAKE) -C tests

test_build:
	$(MAKE) -C tests build
