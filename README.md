SpeakUp
=========
**A simple point-to-point chat interface**

*Use Unix or Linux based systems only.*

Runs on port `4444`. You can change it from the source code.

1. Compile server using `gcc`.
2. Compile client using `gcc` on another machine (or other terminal window for localhost).
3. First run the server.
4. Then run the client.

Example :

**Server**
```
$ gcc server.c -o server
$ ./server
```

**Client**
```
$ gcc client.c -o client
$ ./client 127.0.0.1 #localhost
```

You should be able to see some colors as well!

[LICENSE] (https://github.com/j809/SpeakUp/blob/master/LICENSE)
