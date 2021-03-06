# Database Initialization Script
#
# written by Konrad Rosenbaum
# Used by bagdinitdb.c
#

#Server configuration:
CREATE TABLE options(
        okey varchar(64) primary key,
        oval varchar(64)
);

CREATE TABLE servercert(
        srvname varchar(32) primary key,
        srvcert oid not null
);

CREATE RULE servercert_d AS ON DELETE
TO servercert WHERE old.srvcert <> NULL DO
SELECT lo_unlink(old.srvcert);

CREATE RULE servercert_u AS ON UPDATE
TO servercert WHERE old.srvcert != new.srvcert DO
SELECT lo_unlink(old.srvcert);


#Users:
CREATE TABLE vcsuser(
        usrname varchar(16) primary key,
        usrauthmethod smallint default 1,
        usracl varchar(8),
        usrpasswd char(32),
        usrcert oid
);
#usrauthmethod:
# 0x0001 - internal, check md5-password stored in usrpasswd
# 0x0002 - PAM, check against Unix system (PAM-service bag)
# 0x0004 - SSL certificate
# 0x4000 - anonymous (no password needed)
# 0x8000 - locked, user cannot login
#usracl:
# A - administrate (overrides all others)
# r - read any project/version/object/etc.
# w - write access on any project/version/object/etc.
# c - configure the server (read/write access on table options)
# p - administrate projects/branches
# u - administrate users

CREATE RULE vcsuser_cert_d AS ON DELETE
TO vcsuser WHERE old.usrcert <> NULL DO
SELECT lo_unlink(old.usrcert);

CREATE RULE vcsuser_cert_u AS ON UPDATE
TO vcsuser WHERE old.usrcert != new.usrcert AND old.usrcert != NULL DO
SELECT lo_unlink(old.usrcert);

#VCS-objects:
CREATE TABLE vobject(
        obid bigserial primary key,
        obtype varchar(32) not null
);
#valid object-types:
# text - normal text (eg. source code) [text-diff is used]
# binary - binary file [binary diff is used]
# device - special device [any new version contains devtype, major, minor as readable text]
# symlink - symbolic link [contains link content (as from readlink)]
# pipe - FIFO [no versions]
# socket - AF_UNIX sockets [no versions]
# directory - directory [modified text diff is used, linewise format: "objectid filename"]

#VCS-projects:
CREATE TABLE project(
        prid serial primary key,
        prname varchar(32) not null unique,
        prbaseobject bigint references vobject(obid) not null
);

#VCS-branches:
CREATE TABLE branch(
        brid serial primary key,
        brname varchar(64) not null unique
);

#link between projects and branches (n:m):
CREATE TABLE projectbranch(
        pbprid int references project(prid) not null,
        pbbrid int references branch(brid) not null,

        primary key (pbbrid,pbprid)
);

#Blessinglevel is the amount of trust the developer gives a version:
CREATE TABLE blessinglevel(
        blid serial primary key,
        blname varchar(64),
        blcomment text
);

INSERT INTO blessinglevel (blname,blcomment) VALUES('development','under development');
INSERT INTO blessinglevel (blname,blcomment) VALUES('alpha','alpha release: for developers only');
INSERT INTO blessinglevel (blname,blcomment) VALUES('beta','beta release: public test phasis');
INSERT INTO blessinglevel (blname,blcomment) VALUES('release','final release: stable enough to be used');
INSERT INTO blessinglevel (blname,blcomment) VALUES('bugfix','bugfix release');
INSERT INTO blessinglevel (blname,blcomment) VALUES('broken','broken version, don\'t use');

#versions of objects:
CREATE TABLE changeset(
        csid bigserial primary key,
        csprid int references project(prid) not null,
        csbrid int references branch(brid) not null,

        csblesslev int references blessinglevel(blid),

        ovcomment text,

        -- does not reference to avoid problems with deleted users
        csusr varchar(16) not null,
        cstime int
);

CREATE TABLE objectversion(
        ovid bigserial primary key,
        ovparent bigint references objectversion(ovid),
        ovcsid bigint references changeset(csid) not null,

        ovobid bigint references vobject(obid) not null,
        ovnum int not null,

        ovcontent OID,
        ovencoding varchar(32),
        ovrights varchar(16), --File rights (Unix: rwxrwxrwx...), this will eventually change to real ACL's

        unique (ovnum,ovobid,ovcsid)
);

#on insert: check wheter this combination of branch and project is allowed:
CREATE RULE changeset_rule_prid_brid_i AS ON INSERT
TO changeset WHERE NOT new.csbrid IN (SELECT pbbrid FROM projectbranch WHERE pbprid = new.csprid)
DO INSTEAD NOTHING;

#on update: don't let users change object, branch or project or an object version
CREATE RULE changeset_rule_prid_brid_u AS ON UPDATE
TO changeset WHERE NOT new.csbrid != old.csbrid OR new.csprid != old.csprid
DO INSTEAD NOTHING;

#on delete: automagically remove blob
CREATE RULE objectversion_rule_ovcontent_d AS ON DELETE
TO objectversion WHERE old.ovcontent <> NULL DO
SELECT lo_unlink(old.ovcontent);

#tag definition:
CREATE TABLE tag(
        tgid serial primary key,
        tgtag varchar(64) not null unique
);

#link between tags and object versions:
CREATE TABLE tagversion(
        tvtag int references tag(tgid) not null,
        tvversion bigint references objectversion(ovid),
        primary key (tvtag,tvversion)
);

CREATE TABLE mergepoint(
        mpid bigserial primary key,
        mpovid bigint references objectversion(ovid),
        mpmergedvrl varchar(255)
);

CREATE TABLE server(
        srvname varchar(32) primary key,
        srvvrl varchar(128)
);


CREATE TABLE projectacl(
        pausrname varchar(16) references vcsuser(usrname) not null,
        paprid int references project(prid) not null,
        parights varchar(8),

        primary key (pausrname,paprid)
);
#ACL:
# A - Admin
# r - read
# b - branch with username-prefix (username-branchname)
# B - branch without prefix (branchname=[A-Za-z0-9_]+)
# d - delete branches

CREATE TABLE branchacl(
        bausrname varchar(16) references vcsuser(usrname) not null,
        baprid int references project(prid) not null,
        babrid int references branch(brid) not null,
        barights varchar(16),

        primary key (bausrname,babrid,baprid)
);
#ACL:
# A - Admin (including removal of this branch)
# W - everything but A
# r - read
# w - write (add versions)
# d - delete versions
# a - add objects
# m - merge (set merge points)
# t - tag/untag

#on insert: check wheter this combination of branch and project is allowed:
CREATE RULE branchacl_rule_prid_brid_i AS ON INSERT
TO branchacl WHERE NOT new.babrid IN (SELECT pbbrid FROM projectbranch WHERE pbprid = new.baprid)
DO INSTEAD NOTHING;

#on update: don't let users change branch or project on acl's
CREATE RULE branchacl_rule_prid_brid_u AS ON UPDATE
TO branchacl WHERE NOT new.babrid != old.babrid OR new.baprid != old.baprid
DO INSTEAD NOTHING;

