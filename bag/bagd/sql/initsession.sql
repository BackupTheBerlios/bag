# Initialize a Session
# -> Sent before the first command in a newly opened session
#
# written by Konrad Rosenbaum
#
# Params: none
#
# dates are formatted big endian: 'YYYY-MM-DD hh:mm:ss'
SET DATESTYLE = 'ISO';
#
# we use GMT/UTC as basis for all time-values
SET TIMEZONE = 'GMT';
#
# highest isolation level for transactions, reason: paranoia
SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL SERIALIZE;