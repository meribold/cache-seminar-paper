# I don't really want to write portable shell scripts.  Just use Bash.
SHELL := /bin/bash

# Disable all built-in rules.  See <http://stackoverflow.com/q/4122831> and
# <http://gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html>.
MAKEFLAGS += --no-builtin-rules

# Clear the suffix list; no suffix rules in this makefile.  See section 7.2.1 of the GNU
# Coding Standards [1].  This might be redundant.
.SUFFIXES:
# [1]: https://www.gnu.org/prep/standards/standards.html#Makefile-Basics

CC       := gcc
CFLAGS   += -std=c11 -Wall -Wextra -march=native -O2
CXX      := g++
CXXFLAGS := -std=c++14 -Wall -Wextra -march=native -O3
OCOUNT   := sudo chrt -f 99 ocount -e CPU_CLK_UNHALTED

.PHONY: all clean graphics

# Set the default goal.
all: paper.pdf graphics

texfiles := $(shell find -name '*.tex')

paper.pdf: $(texfiles) $(wildcard tex/*.tex) paper.bib \
   line-size/line-size.csv access-times/access-times.csv \
   seq-access-times/access-times.csv seq-access-times/cpu-bound/access-times.csv \
   ithare/speedup.txt xpose/speedup.csv
   # xpose/xpose.csv xpose/xpose-simple.csv
   # seq-access-times/step8/access-times.csv array-sum/size-time.csv
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
	   $(CC) $(CFLAGS) -DSTEP=$$((2**i)) '$<' && \
	   sudo chrt -f 99 ./a.out >> '$@'; \
	done

xpose/xpose.csv: xpose/xpose.c
	echo 'x y' > '$@'
	for ((i=0; i<=32; i=i+1)); do \
	   size=$$(awk '{ printf "%.0f", 512*1.1^$$1 }' <<< $$i); \
	   $(CC) -std=c11 -Wall -Wextra -march=native -O3 -Dm=$$size -Dn=$$size '$<' && \
	   sudo chrt -f 99 ./a.out >> '$@'; \
	done

xpose/xpose-simple.csv: xpose/xpose.c
	echo 'x y' > '$@'
	for ((i=0; i<=32; i=i+1)); do \
	   size=$$(awk '{ printf "%.0f", 512*1.1^$$1 }' <<< $$i); \
	   $(CC) -std=c11 -Wall -Wextra -march=native -O3 -Dm=$$size -Dn=$$size -DSIMPLE '$<' && \
	   sudo chrt -f 99 ./a.out >> '$@'; \
	done

# `NR>1` skips the first line.  See <https://unix.stackexchange.com/q/198065>.
xpose/speedup.csv: xpose/xpose-simple.csv xpose/xpose.csv
	paste $^ | awk 'BEGIN { print "x y" } NR>1 { print $$1/1024" "$$2/$$4 }' > '$@'

ithare/list.out: ithare/list-vs-vector.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) '$<' -o ithare/list
	sudo chrt -f 99 ./ithare/list > '$@'

ithare/vector.out: ithare/list-vs-vector.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -DVECTOR '$<' -o ithare/vector
	sudo chrt -f 99 ./ithare/vector > '$@'

ithare/speedup.txt: ithare/list.out ithare/vector.out
	paste $^ | tail -1 | awk '{ print $$1/$$2 }' > '$@'

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

graphics: slides/access-time-plot.png slides/access-time-table.png \
   slides/line-size-plot.png slides/seq-access-time-plot.png \
   slides/cpu-bound-seq-access-time-plot.png slides/oo-picture.png

# `convert(1)` is part of ImageMagick.  This type of conversion requires Ghostscript [1].
#
# [1]: https://www.imagemagick.org/discourse-server/viewtopic.php?t=25823
# [2]: https://stackoverflow.com/q/6605006#comment26339769_6605085
# [3]: https://stackoverflow.com/q/2322750
slides/%.png: graphics/%.pdf
	convert -density 600 '$<' -background white -alpha remove '$@'

# This one should retain transparency.
# slides/oo-picture.png: graphics/oo-picture.pdf
# 	 convert -density 600 '$<' '$@'

graphics/%.pdf: tex/graphics/%.tex
	latexmk -quiet -pdf '$<' -outdir=graphics

# See <https://www.gnu.org/software/make/manual/html_node/Chained-Rules.html>.
.PRECIOUS: graphics/%.pdf

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
