if HAVE_POSTGRES
bin_PROGRAMS = bagdinitdb bagd
endif

bagdinitdb_SOURCES = bagdinitdb.c query.c
bagd_SOURCES = bagd.c bagchild.c bagstream.c bagssl.c bc_main.c bc_auth.c bc_user.c query.c md5.c misc.c
bagd_LDFLAGS = @LIBGNUTLS_LIBS@ @POSTGRES_LDFLAGS@ @PAM_LDFLAGS@
bagd_CFLAGS  = @LIBGNUTLS_CFLAGS@ @POSTGRES_CFLAGS@ -I../libbag/
bagd_LDADD   = ../libbag/libbag.la
bagdinitdb_LDFLAGS = @POSTGRES_LDFLAGS@
bagdinitdb_CFLAGS  = @POSTGRES_CFLAGS@ -I../libbag/
bagdinitdb_LDADD   = ../libbag/libbag.la

CLEANFILES = sql.h sql_*.h

bagdinitdb.c bagd.c: sql.h

SQL = $(shell ls sql/*.sql | sed 's/\.sql/.h/' |tr / _)

sql.h: $(SQL)
	echo '/*machine generated file, don'\''t edit!*/' > $@
	for i in $(SQL) ; do echo '#include "'$$i'"' >>$@ ; done

sql_%.h: sql/%.sql
	echo '#define' $$(echo SQL_$* | tr 'abcdefghijklmnopqrstuvwxyz.' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ_') '"\' >$@
	egrep -v '^[[:blank:]]*((--)|$$)' <$< | \
	egrep -v '^[[:blank:]]*((#)|$$)' | \
	awk '{ gsub(/\\/,"\\\\\\\\\\\\") ; gsub(/\"/, "\\\"") ; gsub(/$$/, "\\n\\") ; print }' >>$@
	echo '"' >>$@


