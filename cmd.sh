#!/bin/sh

cont="cs1-mysql" # container name.
mount="/src" # bind mount.
pass="root" # MySQL password.
db="db1" # MySQL database name.
port="54321"

case "$1" in
  "init")
    if docker ps -a | grep "$cont" > /dev/null; then
      docker stop "$cont"
      docker rm "$cont"
    fi

    docker run -d --name "$cont" -e MYSQL_ROOT_PASSWORD="$pass" -v "`pwd`/src":"$mount" -p "$port":"$port" mysql

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
    if expr "$(expr "$2" : "multi")" = "$(expr length "multi")" > /dev/null; then
      docker exec -it "$cont" make MACRO=-DMULTI_THREAD --directory "$mount"
    else
      docker exec -it "$cont" "$@" --directory "$mount"
    fi
    ;;
  "run")
    docker exec -it "$cont" /src/a.out
    ;;
  "bench")
    ab -c "$2" -n "$3" localhost:"$port"/2
    ;;
  *)
    echo "./cmd.sh <init | bash | mysql | make | run | bench>"
    ;;
esac
