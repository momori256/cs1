# Description

C server using MySQL and HTTP.

# Setup

```sh
cd cs1
./cmd.sh init # Initialize Docker container.
./cmd.sh make # Build source codes in the container.
./cmd.sh run # Run the program.
```

# Benchmarking

`cmd.sh bench` benchmarks this server with `ab`, [Appache HTTP server benchmarking tool](https://httpd.apache.org/docs/2.4/programs/ab.html).

```sh
./cmd.sh make # Single-threading.
./cmd.sh run &
./cmd.sh bench 4 5000 # 4 clients, 5000 request.
```

```sh
./cmd.sh make multi # Multithreading.
./cmd.sh run &
./cmd.sh bench 8 10000 # 8 clients, 10000 request.
```

# Behavior

The main thread is waiting for requests on `epoll`. The main thread either `accept`s the incomming request if it is a `connect` request, or reads the data and passes it to a new worker thread.  
A new thread and a new MySQL connection are created for each request.
