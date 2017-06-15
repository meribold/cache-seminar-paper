SHELL := /bin/bash
MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

CXX      := g++
CXXFLAGS := -std=c++14 -pthread -Wall -Wextra -march=native -O3

herb: herb.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -DTHREADS=8 $< -o $@

# vim: tw=90 ts=8 sts=-1 sw=3 noet
