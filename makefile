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

paper.pdf: array-sum/size-time.csv paper.tex paper.bib
	latexmk -pdf -shell-escape
	makeglossaries paper
	latexmk -pdf -shell-escape

array-sum/size-time.csv: array-sum/array-sum.c
	echo 'x y' > '$@'
	for ((i=0; i<=96; i+=1)); do \
	   gcc -DSIZE=$$((i*1024)) -O2 '$<' && { ./a.out | head -1 >> '$@';}; \
	done

clean:

# vim: tw=90 ts=8 sts=-1 sw=3 noet
