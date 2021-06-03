#!/bin/sh
rm /dev/shm/$2.sql
echo '.dump '$2|sqlite3 $1|sed -e 's/"//g'|grep INSERT > /dev/shm/$2.sql



