bin_PROGRAMS = bagdinitdb bagd

bagdinitdb_SOURCES = bagdinitdb.c
bagd_SOURCES = bagd.c bagchild.c bagstream.c bagssl.c

bagdinitdb.c bagd.c: sql.h

SQL = $(shell ls sql/*.sql | sed 's/\.sql/.h/' |tr / _)

sql.h: $(SQL)
	echo '/*machine generated file, don'\''t edit!*/' > $@
	for i in $(SQL) ; do echo '#include "'$$i'"' >>$@ ; done

sql_%.h: sql/%.sql
	echo '#define' $$(echo SQL_$* | tr 'abcdefghijklmnopqrstuvwxyz.' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ_') '"\' >$@
	egrep -v '^[[:blank:]]*((--)|$$)' <$< | awk '{ gsub(/\"/, "\\\"") ; gsub(/$$/, "\\n\\") ; print }' >>$@
	echo '"' >>$@


