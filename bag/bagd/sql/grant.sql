# Grant these to a DB-user
# Parameter: username

grant select,insert,update,delete
on blessinglevel,branch,branchacl,mergepoint,objectversion,options,
   project,projectacl,projectbranch,server,tag,tagversion,vcsuser,vobject
   to %s;