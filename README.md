# Description

MySQL server written with C language.

# How to setup

```sh
cd cs1
./cmd.sh init # Initialize Docker container.
./cmd.sh make # Build source codes in the container.
./cmd.sh run # Run the program.
```

# TODO

- benchmark
  - `ab -c 8 -n 10000 localhost:54321/2`
  - request per second
- thread pool
