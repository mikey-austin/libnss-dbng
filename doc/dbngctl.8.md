dbngctl(8) -- libnss_dbng database management
=============================================

## SYNOPSIS

`dbngctl` **-s** service [**-b** base] [**-d** key] [**-atly**]

## DESCRIPTION

`dbngctl` allows an administrator to safely manipulate the Berkeley DB databases and indexes for a particular *name service*. Using `dbngctl`, a particular service database may:

* be truncated
* have records added/updated
* delete individual records by primary key
* and dump all records

The options are as follows:

* **-s** *service*:
The service to be operated on. May currently be **passwd**, **shadow** or **group**. This option is required.

* **-b** *base*:
The base filesystem location of the service databases (ie the Berkeley DB environment home directory).

* **-a**:
Parse entries from STDIN and add the corresponding records to the service database. If the record's key already exists, the record is updated. Entries are expected in the traditional database's format, take passwd for example:

    mail:x:8:12:mail:/var/spool/mail:/sbin/nologin

* **-d** *primary key*:
Delete an individual record identified by the supplied primary key.

* **-t**:
Truncate the service database. In the absence of the **-y** option, manual confirmation is required.

* **-y**:
Do not ask for confirmation when truncating the database with **-t**.

* **-l**:
Dump the records in the database in the service's traditional format. This is the default action if no other is specified.

## AUTHORS

`dbngctl` was written by Mikey Austin <mikey@jackiemclean.net>
