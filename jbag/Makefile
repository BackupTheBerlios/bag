##############################
## Makefile for JBag
##############################

###########
#Config

#Java Compiler, VM, Jar
JAVAC 	= javac
JAVA 	= java
JAR	= jar
JDOC	= javadoc


JFLAGS	= -deprecation
JDOCFLAGS= -private -version -author


#Directory/File with JDBC driver for PostgreSQL
PGJDBC	= /usr/lib/pgsql/share/jdbc/postgresql.jar

#Directory to install to
PREFIX	= /usr/local

#END OF Config
###########


LIBS = postgresql.jar

SOURCES = org/bag/Images.java $(shell find . -name '*.java' | grep -v org/bag/Images.java)
CLASSES = $(SOURCES:.java=.class)

IMGS = $(shell find . -name '*.gif')

all: bag.jar $(LIBS)


postgresql.jar:
	cp $(PGJDBC) $@

bag.manifest:
	echo Manifest-Version: 1.0 >$@
	echo Main-Class: org.bag.JBagApplication >>$@
	echo Class-Path: $(LIBS) >>$@


# $(CLASSES) is good enough for dependencies, but does not suffice for the jar:
# intenal classes are not covered
bag.jar: $(CLASSES) bag.manifest
	jar cmf bag.manifest $@ `find . -name '*.class'`

%.class: %.java
	javac $(JFLAGS) $<


org/bag/Images.java: $(IMGS)
	./createimage.sh $(IMGS) >$@


doc:
	mkdir -p doc/javadoc
	rm -rf doc/javadoc/*
	$(JDOC) $(JDOCFLAGS) -d doc/javadoc `find . -name '*.java'`

.SUFFIXES:
.PHONY: clean install doc

clean:
	rm -rf *.manifest `find . -name '*.class'` org/bag/Images.java

distclean: clean
	rm -rf *.jar

install: all
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/share/jbag
	echo '#!/bin/sh' >$(PREFIX)/bin/jbag
	echo exec $(JAVA) -jar $(PREFIX)/share/jbag/bag.jar >>$(PREFIX)/bin/jbag
	chmod +x $(PREFIX)/bin/jbag
	cp *.jar $(PREFIX)/share/jbag

