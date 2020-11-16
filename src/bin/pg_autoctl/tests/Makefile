# Made with help from https://www.ccs.neu.edu/home/skotthe/classes/cs5600/fall/2015/labs/intro-check.html

PG_CONFIG ?= pg_config
BINDIR    ?= $(shell $(PG_CONFIG) --bindir)
TEST_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SRC_DIR := ${TEST_DIR}../..

DEFAULT_CFLAGS = -std=c99 -D_GNU_SOURCE -g
DEFAULT_CFLAGS += -I $(shell $(PG_CONFIG) --includedir)
DEFAULT_CFLAGS += -I $(shell $(PG_CONFIG) --includedir-server)
DEFAULT_CFLAGS += -I $(shell $(PG_CONFIG) --pkgincludedir)/internal
DEFAULT_CFLAGS += $(shell $(PG_CONFIG) --cflags)

COMMON_LIBS = -I${SRC_DIR}/lib/parson/
COMMON_LIBS += -I${SRC_DIR}/pg_autoctl/

DEFAULT_CFLAGS += $(COMMON_LIBS)

override CFLAGS := $(DEFAULT_CFLAGS) $(CFLAGS)

LIBS  = -L $(shell $(PG_CONFIG) --pkglibdir)
LIBS += $(shell $(PG_CONFIG) --ldflags)
LIBS += $(shell $(PG_CONFIG) --libs)
LIBS += -lpq
LIBS += -lcheck


CC = $(shell $(PG_CONFIG) --cc)

# We have to link against the original pg_autoctl objects. Build the list of
# OBJS, ignoring main.c. (XXX Because this is in a different directory and this
# project uses recursive Make, you must manually build the pg_autoctl source
# before running this recipe.)
SRC   = $(filter-out %/main.c,$(wildcard ${SRC_DIR}/pg_autoctl/*.c))
SRC_LIBS = lib-log.o lib-commandline.o lib-parson.o lib-snprintf.o lib-strerror.o

OBJS  = $(patsubst %.c,%.o,$(SRC))
OBJS += $(addprefix $(SRC_DIR)/pg_autoctl/,$(SRC_LIBS))

# Additionally build (and link) all objects from this test directory.
TEST_OBJS = $(patsubst ${TEST_DIR}/%.c,%.o,$(wildcard ${TEST_DIR}/*.c))

build_src:
	make -C ${SRC_DIR}/pg_autoctl

build_test: build_src $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) $(OBJS) $(LIBS) -o run_tests

test: clean build_test
	./run_tests

clean:
	rm -f run_tests *.o
