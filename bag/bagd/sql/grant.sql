# Grant these to a DB-user
# Parameter: privileges,username

grant %s
on blessinglevel,branch,branchacl,mergepoint,objectversion,options,
   project,projectacl,projectbranch,server,tag,tagversion,vcsuser,vobject,
   servercert
   to %s;