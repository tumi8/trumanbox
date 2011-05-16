#/usr/bin/env bash

if [ $# != 2 ]; then
	echo "`basename $0`: <db-user> <dbname>"
	exit 1;
fi

dbuser=$1
dbname=$2

echo "
CREATE TABLE trumanbox_settings (
	key	text,
	value	numeric
);

CREATE TABLE malwaresamples (
	id		numeric,
	beginlogging	date,
	endlogging	date,
	comments	text
);

CREATE TABLE UNKNOWN_UDP_LOGS (
	ClientIP	Inet,
	ClientPort	numeric(7),
	ServerIP	Inet,
	ServerPort	numeric(7),
	Orig_ServerIP	Inet,
	Orig_ServerPort	numeric(7),
	Date		timestamp,
	TrumanTimestamp	text,
	Sample_ID	numeric,
	Type		text,
	BinaryLocation	text
);

CREATE TABLE FTP_LOGS (
        ClientIP        Inet,
        ClientPort      numeric(7),
        ServerIP        Inet,
        ServerPort      numeric(7),
        Orig_ServerIP   Inet,
        Orig_ServerPort numeric(7),
        Date            timestamp,
        TrumanTimestamp text,
        Sample_ID       numeric,
	Type		text,
	Message		text
);

CREATE TABLE FTP_PASSIVE_LOGS (
        ClientIP        Inet,
        ClientPort      numeric(7),
        ServerIP        Inet,
        ServerPort      numeric(7),
        Orig_ServerIP   Inet,
        Orig_ServerPort numeric(7),
        Date            timestamp,
        TrumanTimestamp text,
        Sample_ID       numeric,
	BinaryLocation	Text,
	Type		Text,
	Filename	text
);

CREATE TABLE HTTP_LOGS (
	ClientIP	Inet,
	ClientPort	numeric(7),
	ServerIP	Inet,
	ServerPort	numeric(7),
	Orig_ServerIP	Inet,
	Orig_ServerPort	numeric(7),
	Date		timestamp,
	TrumanTimestamp	text,
	Sample_ID	numeric,
	RequestedHost	text,
	RequestedLocation text,
	UserAgent	text,
	Method		text,
	RequestHandler	text,
	RequestBodyBinaryLocation text,
	ResponseLastModified text,
	ResponseContentType text,
	ServerType	text,
	ResponseReturnedType	text,
	ResponseHeader	text,
	ResponseBodyBinaryLocation	text
);

CREATE TABLE IRC_CLIENT_LOGS (
	ClientIP	Inet,
	ClientPort	numeric(7),
	ServerIP	Inet,
	ServerPort	numeric(7),
	Orig_ServerIP	Inet,
	Orig_ServerPort	numeric(7),
	Date		timestamp,
	TrumanTimestamp	text,
	Sample_ID	numeric,
	Command		text,
	Arguments	text
);

CREATE TABLE IRC_SERVER_LOGS (
	ClientIP	Inet,
	ClientPort	numeric(7),
	ServerIP	Inet,
	ServerPort	numeric(7),
	Orig_ServerIP	Inet,
	Orig_ServerPort	numeric(7),
	Date		timestamp,
	TrumanTimestamp	text,
	Sample_ID	numeric,
	ServerName	text,
	NumericReply	text,
	RecipientNickName text,
	Message		text
);

CREATE TABLE IRC_LOGS (
	ClientIP	Inet,
	ClientPort	numeric(7),
	ServerIP	Inet,
	ServerPort	numeric(7),
	Orig_ServerIP	Inet,
	Orig_ServerPort	numeric(7),
	Date		timestamp,
	TrumanTimestamp	text,
	Sample_ID	numeric,
	LogFileLocation text
);

CREATE TABLE SMTP_LOGS (
	ClientIP	Inet,
	ClientPort	numeric(7),
	ServerIP	Inet,
	ServerPort	numeric(7),
	Orig_ServerIP	Inet,
	Orig_ServerPort	numeric(7),
	Date		timestamp,
	TrumanTimestamp	text,
	Sample_ID	numeric,
	Type		text,
	Message		text
);

CREATE TABLE SSL_LOGS (
	ClientIP	Inet,
	ClientPort	numeric(7),
	ServerIP	Inet,
	ServerPort	numeric(7),
	Orig_ServerIP	Inet,
	Orig_ServerPort	numeric(7),
	Date		timestamp,
	TrumanTimestamp	text,
	Sample_ID	numeric,
	Client_Hello_SSL_Version text,
	Server_Certificate_Location text,
	Http_Request_Location text
);

CREATE TABLE SSL_MITM_LOGS (
	ClientIP	Inet,
	ClientPort	numeric(7),
	ServerIP	Inet,
	ServerPort	numeric(7),
	Orig_ServerIP	Inet,
	Orig_ServerPort	numeric(7),
	Date		timestamp,
	TrumanTimestamp	text,
	Sample_ID	numeric,
	Type		text,
	BinaryLocation	text
);

CREATE TABLE UNKNOWN_LOGS (
	Type		text,
	BinaryLocation	text
);

CREATE TABLE AWAITED_PASV (
	ServerIP	inet,
	ServerPort	numeric(7),
	type		text,
	filename	text
);

\quit
" | 
psql -U $dbuser $dbname -W 
