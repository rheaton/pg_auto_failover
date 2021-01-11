# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the PostgreSQL License.

CONTAINER_NAME = pg_auto_failover
TEST_CONTAINER_NAME = pg_auto_failover_test
DOCKER_RUN_OPTS = --privileged  -ti --rm

NOSETESTS = $(shell which nosetests3 || which nosetests)

# Tests for multiple standbys
MULTI_SB_TESTS  = $(basename $(notdir $(wildcard tests/test*_multi*)))
MULTI_SB_TESTS += $(basename $(notdir $(wildcard tests/test*_disabled*)))

# TEST indicates the testfile to run
TEST ?=
ifeq ($(TEST),)
	TEST_ARGUMENT = --where=tests
else ifeq ($(TEST),multi)
	TEST_ARGUMENT = --where=tests --tests=$(MULTI_SB_TESTS)
else ifeq ($(TEST),single)
	TEST_ARGUMENT = --where=tests --exclude='_multi_' --exclude='disabled'
else
	TEST_ARGUMENT = $(TEST:%=tests/%.py)
endif

# Documentation and images
FSM = docs/fsm.png
PDF = ./docs/_build/latex/pg_auto_failover.pdf

# Command line with DEBUG facilities
PG_AUTOCTL = PG_AUTOCTL_DEBUG=1 ./src/bin/pg_autoctl/pg_autoctl

# make cluster arguments
NODES ?= 2
NODES_ASYNC ?= 0
NODES_SYNC_SB ?= -1
FIRST_PGPORT ?= 5500

TMUX_EXTRA_COMMANDS ?= ""
TMUX_LAYOUT ?= even-vertical	# could be "tiled"
TMUX_TOP_DIR = ./tmux
TMUX_SCRIPT = ./tmux/script-$(FIRST_PGPORT).tmux

# make azcluster arguments
AZURE_PREFIX ?= ha-demo-$(shell whoami)
AZURE_REGION ?= paris
AZURE_LOCATION ?= francecentral

# Pick a version of Postgres and pg_auto_failover packages to install
# in our target Azure VMs when provisionning
#
#  sudo apt-get install -q -y postgresql-13-auto-failover-1.4=1.4.1
#  postgresql-${AZ_PG_VERSION}-auto-failover-${AZ_PGAF_DEB_VERSION}=${AZ_PGAF_VERSION}
AZ_PG_VERSION ?= 13
AZ_PGAF_DEB_VERSION ?= 1.4
AZ_PGAF_DEB_REVISION ?= 1.4.1-1

export AZ_PG_VERSION
export AZ_PGAF_DEB_VERSION
export AZ_PGAF_DEB_REVISION

all: monitor bin ;

install: install-monitor install-bin ;
clean: clean-monitor clean-bin ;
check: check-monitor ;

monitor:
	$(MAKE) -C src/monitor/ all

clean-monitor:
	$(MAKE) -C src/monitor/ clean

install-monitor: monitor
	$(MAKE) -C src/monitor/ install

check-monitor: install-monitor
	$(MAKE) -C src/monitor/ installcheck

bin:
	$(MAKE) -C src/bin/ all

clean-bin:
	$(MAKE) -C src/bin/ clean

install-bin: bin
	$(MAKE) -C src/bin/ install

test:
	sudo -E env "PATH=${PATH}" USER=$(shell whoami) \
		$(NOSETESTS)			\
		--verbose				\
		--nocapture				\
		--stop					\
		${TEST_ARGUMENT}

test_unit:
	$(MAKE) -C src/bin/pg_autoctl test

indent:
	citus_indent
	black .

docs: $(FSM)
	$(MAKE) -C docs html

build:
	docker build				    \
		$(DOCKER_BUILD_OPTS)		\
		-t $(CONTAINER_NAME)	    \
		.

interactive-test:
	docker run --name $(CONTAINER_NAME) --rm -ti $(CONTAINER_NAME)


build-test:
	docker build				    \
		$(DOCKER_BUILD_OPTS)		\
		--target build-test         \
		-t $(TEST_CONTAINER_NAME)	\
		.

run-test: build-test
	docker run					                \
		--name $(TEST_CONTAINER_NAME)		    \
		$(DOCKER_RUN_OPTS)			            \
		$(TEST_CONTAINER_NAME)			        \
		make -C /usr/src/pg_auto_failover test	\
		TEST='${TEST}'

man:
	$(MAKE) -C docs man

pdf: $(PDF)

$(PDF):
	$(MAKE) -s -C docs/tikz pdf
	perl -pi -e 's/(^.. figure:: .*)\.svg/\1.pdf/' docs/*.rst
	$(MAKE) -s -C docs latexpdf
	perl -pi -e 's/(^.. figure:: .*)\.pdf/\1.svg/' docs/*.rst
	ls -l $@

$(FSM): bin
	$(PG_AUTOCTL) do fsm gv | dot -Tpng > $@

$(TMUX_SCRIPT): bin
	mkdir -p $(TMUX_TOP_DIR)
	$(PG_AUTOCTL) do tmux script          \
         --root $(TMUX_TOP_DIR)           \
         --first-pgport $(FIRST_PGPORT)   \
         --nodes $(NODES)                 \
         --async-nodes $(NODES_ASYNC)     \
         --sync-standbys $(NODES_SYNC_SB) \
         --layout $(TMUX_LAYOUT) > $@

tmux-script: $(TMUX_SCRIPT) ;

tmux-clean:
	pkill pg_autoctl || true
	rm -rf $(TMUX_TOP_DIR)

cluster: install
	$(PG_AUTOCTL) do tmux session         \
         --root $(TMUX_TOP_DIR)           \
         --first-pgport $(FIRST_PGPORT)   \
         --nodes $(NODES)                 \
         --async-nodes $(NODES_ASYNC)     \
         --sync-standbys $(NODES_SYNC_SB) \
         --layout $(TMUX_LAYOUT)


azcluster: all
	$(PG_AUTOCTL) do azure create         \
         --prefix $(AZURE_PREFIX)         \
         --region $(AZURE_REGION)         \
         --location $(AZURE_LOCATION)     \
         --nodes $(NODES)

# make azcluster has been done before, just re-attach
az: all
	$(PG_AUTOCTL) do azure tmux session

azdrop: all
	$(PG_AUTOCTL) do azure drop

.PHONY: all clean check install docs
.PHONY: monitor clean-monitor check-monitor install-monitor
.PHONY: bin clean-bin install-bin
.PHONY: build-test run-test
.PHONY: tmux-clean cluster
.PHONY: azcluster azdrop az
