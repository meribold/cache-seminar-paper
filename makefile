# I don't really want to write portable shell scripts.  Just use Bash.
SHELL := /bin/bash

# Disable all built-in rules.  See <http://stackoverflow.com/q/4122831> and
# <http://gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html>.
MAKEFLAGS += --no-builtin-rules

# Clear the suffix list; no suffix rules in this makefile.  See section 7.2.1 of the GNU
# Coding Standards [1].  This might be redundant.
.SUFFIXES:
# [1]: https://www.gnu.org/prep/standards/standards.html#Makefile-Basics

CC     := gcc
CFLAGS += -Wall -march=native -O2
OCOUNT := sudo chrt -f 99 ocount -e CPU_CLK_UNHALTED

.PHONY: all clean

# Set the default goal.
all: paper.pdf

texfiles := $(shell find -name '*.tex')

paper.pdf: $(texfiles) $(wildcard tex/*.tex) paper.bib array-sum/size-time.csv \
   line-size/line-size.csv access-times/access-times.csv \
   seq-access-times/access-times.csv seq-access-times/cpu-bound/access-times.csv
   # seq-access-times/step8/access-times.csv
	latexmk -quiet -pdf -shell-escape '$(@:.pdf=.tex)'

# 1 KiB to 128 MiB.
working-set-sizes := $(shell for ((i=7; i<=24; i=i+1)); do printf "$$((8*2**i)) "; done)

access-time-results := $(addprefix access-times/, $(working-set-sizes))
access-time-results := $(addsuffix .out,$(access-time-results))

seq-access-time-results := $(addprefix seq-access-times/, $(working-set-sizes))
seq-access-time-results := $(addsuffix .out,$(seq-access-time-results))

cpu-bound-access-results := $(addprefix seq-access-times/cpu-bound/, $(working-set-sizes))
cpu-bound-access-results := $(addsuffix .out,$(cpu-bound-access-results))

seq8-access-time-results := $(addprefix seq-access-times/step8/, $(working-set-sizes))
seq8-access-time-results := $(addsuffix .out,$(seq8-access-time-results))

# See <https://www.gnu.org/software/make/manual/make.html#Canned-Recipes>.
define profile-access-times =
size=$(patsubst %.out,%,$(notdir $@)); \
$(CC) $(CFLAGS) -DSIZE=$$((size/8)) '$<' && \
$(OCOUNT) ./a.out | tee '$@' && \
$(CC) $(CFLAGS) -DSIZE=$$((size/8)) -DBASELINE '$<' && \
$(OCOUNT) ./a.out | tee -a '$@'
endef

# Counting fails sometimes, in which case the according .out files can be removed like so:
#    $ ag -l 'Event not counted' seq-access-times/ | xargs rm
# Then, re-invoking `make` will only regenerate those.  TODO: fix or at least automate
# this.

$(access-time-results): access-times/access-times.c
	$(profile-access-times)

# TODO: add `-fno-prefetch-loop-arrays`?
$(seq-access-time-results): seq-access-times/access-times.c
	$(profile-access-times)

# FIXME: DRY.
$(cpu-bound-access-results): seq-access-times/access-times.c
	size=$(patsubst %.out,%,$(notdir $@)); \
	$(CC) $(CFLAGS) -fno-prefetch-loop-arrays -DSIZE=$$((size/8)) -DWORKWORK '$<' && \
	$(OCOUNT) ./a.out | tee '$@' && \
	$(CC) $(CFLAGS) -fno-prefetch-loop-arrays -DSIZE=$$((size/8)) -DWORKWORK -DBASELINE '$<' && \
	$(OCOUNT) ./a.out | tee -a '$@'

# FIXME: DRY.
$(seq8-access-time-results): seq-access-times/access-times.c
	size=$(patsubst %.out,%,$(notdir $@)); \
	$(CC) $(CFLAGS) -fno-prefetch-loop-arrays -DSIZE=$$((size/8)) -DSTEP=8 '$<' && \
	$(OCOUNT) ./a.out | tee '$@' && \
	$(CC) $(CFLAGS) -fno-prefetch-loop-arrays -DSIZE=$$((size/8)) -DSTEP=8 -DBASELINE '$<' && \
	$(OCOUNT) ./a.out | tee -a '$@'

line-size/line-size.csv: line-size/line-size.c
	echo 'x y' > '$@'
	for ((i=0; i<=10; i=i+1)); do \
	   $(CC) $(CFLAGS) -DSTEP=$$((2**i)) '$<' && ./a.out >> '$@'; \
	done

array-sum/size-time.csv: array-sum/array-sum.c
	echo 'x y' > '$@'
	for ((i=0; i<=96; i+=1)); do \
	   $(CC) $(CFLAGS) -DSIZE=$$((i*1024)) '$<' && { ./a.out | head -1 >> '$@';}; \
	done

clean:
	latexmk -C
	rm paper.{bbl,run.xml}

access-times/access-times.csv : out-files := $(access-time-results)
seq-access-times/access-times.csv : out-files := $(seq-access-time-results)
seq-access-times/cpu-bound/access-times.csv : out-files := $(cpu-bound-access-results)
seq-access-times/step8/access-times.csv : out-files := $(seq8-access-time-results)

.SECONDEXPANSION:

# This is inefficient because too many processes are being spawned.  FIXME.  The `sed`
# command joins every 2 lines (https://stackoverflow.com/a/16906481).
access-times/access-times.csv seq-access-times/access-times.csv \
   seq-access-times/step8/access-times.csv seq-access-times/cpu-bound/access-times.csv: \
   $$(out-files)
	@echo "Merging profiling data into $@..."
	@echo 'x y total baseline' > '$@'
	@for f in $^; do \
	   size="$${f##*/}"; size="$${size%.out}"; \
	   awk '/CPU_CLK_UNHALTED/ { gsub(/,/,"",$$2); print $$2/100000000 }' "$$f" | \
	   sed 'N;s/\n/ /g' | \
	   awk '{ print '"$$size"'" "$$1 - $$2" "$$1" "$$2 }'; \
	done >> '$@'

# vim: tw=90 ts=8 sts=-1 sw=3 noet
