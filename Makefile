#  
#  Created by: Abdulfatai Popoola
#  Copyright (c) 2012 Bitcoin Project Team
#

CFLAGS=-g -Wall -Wextra -DNDEBUG -pedantic -ansi $(OPTFLAGS)
LIBS=-lcrypto -lcunit -pthread $(OPTLIBS)
PREFIX?=/usr/local

SOURCES=$(shell find src/ -type f -name '*.c')
OBJECTS=$(patsubst src/%.c,bin/%.o,$(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=build/libbitcoin.a

# The Target Build
all: $(TARGET) tests

dev: CFLAGS=-g -Wall -Wextra -pedantic -ansi $(OPTFLAGS)
dev: all


$(TARGET) : build $(OBJECTS)
	ar rcs $@ $(OBJECTS)
	ranlib $@
	gcc -o"MIBitcoin" $(OBJECTS) $(LIBS)
	
build:
	@mkdir -p build
	@mkdir -p bin
	@mkdir -p tests/valgrind-logs

# Unit tests and moving compiled files to the bin directory
.PHONY: all tests
all: $(OBJECTS)

bin/%: src/%
	@mkdir -p "$(@D)"
	@echo moving "$<" to "$@"
	@mv $< $@

tests: $(TESTS)
	sh ./tests/runtests.sh

$(TESTS): $(TARGET)
	gcc $(CFLAGS) $@.c $(TARGET) $(LIBS) -o"$@"

valgrind:
		VALGRIND="valgrind --log-file=tests/valgrind-logs/valgrind-%p.log" $(MAKE)

# The Cleaner
clean:
		rm -rf MIBitcoin
		rm -rf build $(OBJECTS) $(TESTS) bin
		rm -f tests/tests.log
		rm -rf tests/valgrind-logs
		find . -name "*.gc*" -exec rm {} \;
		rm -rf 'find . -name "*.dSYM" -print'

# The Install
install: all
		install -d $(DESTDIR)/$(PREFIX)/lib/
		install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/

# The Checker
BADFUNCS='[^_.>a-zA-Z0-9] (str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)'
check:
		@echo Files with potentially dangerous functions
		@egrep $(BADFUNCS) $(SOURCES) || true
