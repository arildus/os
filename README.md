# os
Me and two friends implemented some simple operating system tasks in C and unix commands.
We made one alarm clock scheduler (handling processes and killing them), one webserver and one shell.

## How to run
Must be runned using Linux or Unix based OS.

For the alarm clock scheduler:
```
cd AlarmClock
gcc main.c
./a.out
```

For the web server:
```
cd webserver
gcc mtwwwd.c
mtwwwd <path> <port> <#threads> <#bufferslots>
```

For the shell:
```
cd shell
gcc flush.c
flush
```
