# Project of COMP4621

## Compile

```
gcc server_skeleton.c -o server -pthread
gcc client_skeleton.c -o client -pthread
```

## Run

- Firstly run the server by typing `./server`
- Then you could go with the interaction between server in other terminal windows

## Tests

|                Task                | Status |                  Issues(if any)                  |
| :---------------------------------: | :----: | :----------------------------------------------: |
|            Registration            |  DONE  |                       NULL                       |
|                Login                |  DONE  |                       NULL                       |
| Welcome for new user and login back |  DONE  |                       NULL                       |
|        Online direct message        |  DONE  |                       NULL                       |
|       Offline direct message       |  DONE  |                       NULL                       |
|                EXIT                | DONE |                         NULL                         |
|                 WHO                 |  DONE  |                       NULL                       |
|      OFFLINE message retrieval      |  DONE  |                       NULL                       |
|        Broadcasting message        |  DONE  |                       NULL                       |
|        Password for register        |  DONE  |                       NULL                       |
|         Password for log in         | DONE |      NULL                                          |

## TODO list
- [ ] GUI implementation with QT framework
- [ ] Exception handling for server interruption and restart
- [ ] Use the data structure of hash table to maintain the user list
- [ ] Add the password to restrict access of the offline message box (`txt file`)
- [ ] Encrypt the password for the user during communication 
