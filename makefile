# I don't really want to write portable shell scripts.  Just use Bash.
SHELL := /bin/bash

# Disable all built-in rules.  See <http://stackoverflow.com/q/4122831> and
# <http://gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html>.
MAKEFLAGS += --no-builtin-rules

# Clear the suffix list; no suffix rules in this makefile.  See section 7.2.1 of the GNU
# Coding Standards [1].  This might be redundant.
.SUFFIXES:
# [1]: https://www.gnu.org/prep/standards/standards.html#Makefile-Basics

.PHONY: all clean

# Set the default goal.
all: paper.pdf

paper.pdf: paper.tex $(wildcard tex/*.tex) paper.bib array-sum/size-time.csv \
   line-size/line-size.csv access-times/access-times.csv
	latexmk -quiet -pdf -shell-escape '$<'

# 1 KiB to 128 MiB.
working-set-sizes := $(shell for ((i=7; i<=24; i=i+1)); do printf "$$((8*2**i)) "; done)

access-time-results := $(addprefix access-times/, $(working-set-sizes))
access-time-results := $(addsuffix .out,$(access-time-results))

$(access-time-results): access-times/access-times.c
	size=$(patsubst access-times/%.out,%,$@); \
	gcc -O2 -DSIZE=$$((size/8)) '$<' && \
	sudo chrt -f 99 ocount -e CPU_CLK_UNHALTED ./a.out | tee '$@' && \
	gcc -O2 -DSIZE=$$((size/8)) -DBASELINE '$<' && \
	sudo chrt -f 99 ocount -e CPU_CLK_UNHALTED ./a.out | tee -a '$@'

# Combine all the .out files from runs with different values of `SIZE`.  See
# <http://wiki.bash-hackers.org/syntax/pe#substring_removal>.
access-times/access-times.out: $(access-time-results)
	@for f in $^; do \
	   size="$${f##*/}"; size="$${size%.out}"; \
	   printf "Bytes: $$size\n"; \
	   cat "$$f"; \
	done > '$@'

# access-times/access-times.out: access-times/access-times.c
# 	@# Working sets from 256 bytes to 32 MiB.
# 	for ((i=5; i<=23; i=i+1)); do \
# 	   size=$$((2**i)); printf "Bytes: $$((8*size))\n"; \
# 	   gcc -O2 -DSIZE=$$size -DBASELINE '$<' && \
# 	   sudo chrt -f 99 ocount -e CPU_CLK_UNHALTED ./a.out; \
# 	done | tee '$@'

# The `sed` command joins every 3 lines (https://stackoverflow.com/a/16906481).  The
# second `awk` command subtracts the third from the second column.
access-times/access-times.csv: access-times/access-times.out
	echo 'x y total baseline' > '$@'
	@# awk '/Bytes:/ { printf $$2" " }
	awk '/Bytes:/ { print $$2 } \
	     /CPU_CLK_UNHALTED/ { gsub(/,/,"",$$2); print $$2/100000000 }' '$<' | \
	sed 'N;N;s/\n/ /g' | \
	awk '{ print $$1" "$$2 - $$3" "$$2" "$$3 }' >> '$@'

line-size/line-size.csv: line-size/line-size.c
	echo 'x y' > '$@'
	for ((i=0; i<=10; i=i+1)); do \
	   gcc -O2 -DSTEP=$$((2**i)) '$<' && ./a.out >> '$@'; \
	done

array-sum/size-time.csv: array-sum/array-sum.c
	echo 'x y' > '$@'
	for ((i=0; i<=96; i+=1)); do \
	   gcc -O2 -DSIZE=$$((i*1024)) '$<' && { ./a.out | head -1 >> '$@';}; \
	done

clean:
	latexmk -C
	rm paper.{bbl,run.xml}

# vim: tw=90 ts=8 sts=-1 sw=3 noet
