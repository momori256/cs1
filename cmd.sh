#!/bin/sh

cont="cs1-mysql" # container name.
mount="/src" # bind mount.
pass="root" # MySQL password.
db="db1" # MySQL database name.

case "$1" in
  "init")
    if docker ps -a | grep "$cont" > /dev/null; then
      docker stop "$cont"
      docker rm "$cont"
    fi

    docker run -d --name "$cont" -e MYSQL_ROOT_PASSWORD="$pass" -v "`pwd`/src":"$mount" -p 54321:54321 mysql

    docker exec -it "$cont" apt-get -y update
    docker exec -it "$cont" apt-get -y upgrade
    docker exec -it "$cont" apt-get -y install libmysqlclient-dev mysql-server gcc make libssl-dev

    until docker logs "$cont" 2>&1 | grep 'mysqld: ready for connections.*port: 3306' > /dev/null; do
      sleep 1
    done

    docker exec -it "$cont" mysql -p"$pass" -uroot -e "create database $db"
    docker exec -it "$cont" mysql -p"$pass" -uroot -e "create table $db.tb1(id integer, name varchar(16))"
    docker exec -it "$cont" mysql -p"$pass" -uroot -e "insert into $db.tb1 values (1, 'Alice'), (2, 'Bob'), (3, 'Charlie')"
    ;;
  "bash")
    docker exec -it "$cont" /bin/bash
    ;;
  "mysql")
    docker exec -it "$cont" mysql -p"$pass" -uroot -D"$db"
    ;;
  "make")
    docker exec -it "$cont" "$@" --directory "$mount"
    ;;
  "run")
    docker exec -it "$cont" /src/a.out
    ;;
  *)
    echo "./cmd.sh <init | bash | mysql | make | run>"
    ;;
esac
