# Create a new user account
#
# Parameters:
# 1 - username
# 2 - authentication method
# 3 - password (clear text)?
# 4 - initial ACL
#

INSERT INTO vcsuser (usrname,usrauthmethod,usrpasswd,usracl)
        values('%s','%s',md5('%s'),'%s');
