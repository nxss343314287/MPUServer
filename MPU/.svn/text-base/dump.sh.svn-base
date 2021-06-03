#!/bin/sh
rm -rf ~/$3.sql*
echo '.dump '$2|sqlite3 $1  |grep INSERT > ~/$2.sql
