####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libbagd.a

INCLUDES = $(all_includes)

libbagd_a_SOURCES = bagdinitdb.c 

SUBDIRS = sql 

EXTRA_DIST = bagdinitdb.c initdb.sql sql_const.h 

####### kdevelop will overwrite this part!!! (end)############

bin_PROGRAMS = bagdinitdb

bagdinitdb_SOURCES = bagdinitdb.c

bagdinitdb.c bagd.c: sql.h

SQL = $(shell ls sql/*.sql | sed 's/\.sql/.h/' |tr / _)

sql.h: $(SQL)
	echo '/*machine generated file, don'\''t edit!*/' > $@
	for i in $(SQL) ; do echo '#include "'$$i'"' >>$@ ; done

sql_%.h: sql/%.sql
	echo '#define' $$(echo SQL_$* | tr 'abcdefghijklmnopqrstuvwxyz.' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ_') '"\' >$@
	egrep -v '^[[:blank:]]*((--)|$$)' <$< | awk '{ gsub(/\"/, "\\\"") ; gsub(/$$/, "\\n\\") ; print }' >>$@
	echo '"' >>$@


