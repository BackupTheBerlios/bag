dnl Autoconf Macros for Postgres
dnl by Konrad Rosenbaum

# Postgres Autoconf Macros
# version 0.1, oct2002

AC_DEFUN(BAG_POSTGRES,
[
dnl Help text
AC_ARG_WITH(postgres,
	[ --with-postgres=PATH Prefix for the Postgres Library/Headers],
	postgres="$withval", postgres="")


dnl Test case
AC_MSG_CHECKING(PostgreSQL)


if test "$postgres" = "" ; then
	cflags_save="$CFLAGS"
	ldflags_save="$LDFLAGS"
	#search lib
	for pgpath in /usr /usr/local /usr/local/postgres /usr/local/postgresql /usr/local/pgsql ; do
		LDFLAGS="$ldflags_save -L$pgpath/lib"
		AC_CHECK_LIB(pq, PQfinish,[postgres=$pgpath;break], )
	done
	CFLAGS=$cflags_save
	LDFLAGS=$ldflags_save
fi

if test "$postgres" != "" ; then
	pginclude=""
	for ipath in . postgres postgresql pgsql ; do 
		AC_CHECK_HEADERS($postgres/include/$ipath/libpq-fe.h,[pginclude="$postgres/include/$ipath";break],)
	done
	if test "$pginclude" = "" ; then
		postgres=""
	fi
fi

if test "$postgres" = "" ; then
	AC_MSG_RESULT(no)
	HAVE_POSTGRES=0
	POSTGRES_DIR=""
	POSTGRES_CFLAGS=""
	POSTGRES_LDFLAGS=""
else
	AC_MSG_RESULT([  yes, library: $postgres/lib header: $pginclude])
	HAVE_POSTGRES=1
	POSTGRES_DIR="$postgres"
	POSTGRES_CFLAGS="-I$pginclude"
	POSTGRES_LDFLAGS="-L$postgres/lib -lpq"
	AC_DEFINE(HAVE_POSTGRES)
fi

AC_SUBST(HAVE_POSTGRES)
AC_SUBST(POSTGRES_DIR)
AC_SUBST(POSTGRES_CFLAGS)
AC_SUBST(POSTGRES_LDFLAGS)

dnl End of BAG_POSTGRES:
] )
