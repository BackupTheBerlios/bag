#Param: Username

SELECT usrauthmethod,usracl,usrpasswd,usrcert FROM vcsuser WHERE usrname=%s;
