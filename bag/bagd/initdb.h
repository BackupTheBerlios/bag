#define SQL_INITDB "\
BEGIN TRANSACTION ;\n\
CREATE TABLE options(\n\
        okey varchar(64) primary key,\n\
        oval varchar(64)\n\
);\n\
CREATE TABLE vcsuser(\n\
        usrname varchar(16) primary key,\n\
        usrauthmethod tinyint default 0,\n\
        usracl varchar(8) default 'r',\n\
        usrpasswd char(32)\n\
);\n\
CREATE TABLE vobject(\n\
        obid bigsequence primary key,\n\
        obtype varchar(32) not null\n\
);\n\
CREATE TABLE project(\n\
        prid sequence primary key,\n\
        prname varchar(32) not null unique,\n\
        prbaseobject bigint references vobject(obid) not null\n\
);\n\
CREATE TABLE branch(\n\
        brid sequence primary key,\n\
        brname varchar(64) not null unique\n\
);\n\
CREATE TABLE projectbranch(\n\
        pbprid int references project(prid) not null,\n\
        pbbrid int references branch(brid) not null,\n\
        primary key (pbbrid,pbprid)\n\
);\n\
CREATE TABLE blessinglevel(\n\
        blid sequence primary key,\n\
        blname varchar(64),\n\
        blcomment text\n\
);\n\
CREATE TABLE objectversion(\n\
        ovid bigsequence primary key,\n\
        ovparent bigint references objectversion(ovid),\n\
        ovprid int references projectbranch(pbprid) not null,\n\
        ovbrid int references projectbranch(pbbrid) not null,\n\
        ovobid bigint references vobject(obid) not null,\n\
        ovnum int not null,\n\
        ovcontent OID,\n\
        ovcomment text,\n\
        ovencoding varchar(32),\n\
        ovblesslev int references blessinglevel(blid),\n\
        ovusr varchar(16) not null,\n\
        ovtime int,\n\
        unique (ovprid,ovbrid,ovobid,ovnum)\n\
);\n\
CREATE TABLE tag(\n\
        tgid sequence primary key,\n\
        tgtag varchar(64) not null\n\
);\n\
CREATE TABLE tagversion(\n\
        tvtag int references tag(tgid) not null,\n\
        tvversion bigint references objectversion(ovid),\n\
        primary key (tvtag,tvversion)\n\
);\n\
CREATE TABLE mergepoint(\n\
        mpid bigsequence primary key,\n\
        mpovid bigint references objectversion(ovid),\n\
        mpmergedvrl varchar(255)\n\
);\n\
CREATE TABLE server(\n\
        srvname varchar(32) primary key,\n\
        srvvrl varchar(128)\n\
);\n\
CREATE TABLE projectacl(\n\
        pausrname varchar(16) references vcsuser(usrname) not null,\n\
        paprid int references project(prid) not null,\n\
        parights varchar(8),\n\
        primary key (pausrname,paprid)\n\
);\n\
CREATE TABLE branchacl(\n\
        bausrname varchar(16) references vcsuser(usrname) not null,\n\
        baprid int references projectbranch(pbprid) not null,\n\
        babrid int references projectbranch(pbbrid) not null,\n\
        barights varchar(8),\n\
        primary key (bausrname,babrid,baprid)\n\
);\n\
COMMIT;\n\
"
