#
# Copyright 2017 Giovanni Mels
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

.PHONY: all install uninstall installdirs test clean maintainer-clean distclean dist tar zip 

SHELL=/bin/sh
LEX=flex

prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin

CXXFLAGS=-O2 -Wall -march=native
LFLAGS=--warn

INSTALL=install
INSTALL_PROGRAM=$(INSTALL)
INSTALL_DATA=$(INSTALL) -m 644
LEXER_CC=N3Lexer.cc
SOURCES:=src/$(LEXER_CC) $(filter-out src/$(LEXER_CC), $(wildcard src/*.cc))
OBJECTS:=$(patsubst src/%.cc, obj/%.o, $(SOURCES))
INCLUDES:=$(wildcard src/*.hh)


all: carl


carl: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)


obj/%.o: src/%.cc $(INCLUDES)
	@mkdir -p $(@D)
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -std=c++11 -o $@ $<


src/$(LEXER_CC): src/N3.l
	$(LEX) $(LFLAGS) -o $@ $<


install: carl installdirs
	$(INSTALL_PROGRAM) carl $(DESTDIR)$(bindir)


uninstall:
	rm -f $(DESTDIR)$(bindir)/carl


installdirs:
	mkdir -p $(DESTDIR)$(bindir)


test: carl
	$(MAKE) -C test
	test/test-carl


clean:
	rm -f obj/*.o
	rm -f carl
	rm -f carl.tar.gz
	rm -f carl.zip
	$(MAKE) -C test clean


maintainer-clean: clean
	rm -f src/$(LEXER_CC)


distclean: clean


dist: tar


tar: carl.tar.gz


carl.tar.gz: $(SOURCES) src/N3.l $(INCLUDES) LICENSE README.md
	$(eval TMP := $(shell mktemp -d carl.XXXXXXXX -t))
	mkdir -p $(TMP)/carl/src
	cp $(MAKEFILE_LIST) LICENSE README.md $(TMP)/carl
	cp $(SOURCES) src/N3.l $(INCLUDES) $(TMP)/carl/src
	mkdir $(TMP)/carl/test
	cp test/*.cc test/*.hpp test/Makefile $(TMP)/carl/test
	tar -C $(TMP) -czf $@ carl
	rm -rf $(TMP)


zip: carl.zip


carl.zip: $(SOURCES) src/N3.l $(INCLUDES) LICENSE README.md
	$(eval TMP := $(shell mktemp -d carl.XXXXXXXX -t))
	mkdir -p $(TMP)/carl/src
	cp $(MAKEFILE_LIST) LICENSE README.md $(TMP)/carl
	cp $(SOURCES) src/N3.l $(INCLUDES) $(TMP)/carl/src
	mkdir $(TMP)/carl/test
	cp test/*.cc test/*.hpp test/Makefile $(TMP)/carl/test
	cd $(TMP) && zip -r $@ carl
	cp $(TMP)/$@ .
	rm -rf $(TMP)
