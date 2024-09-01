# Crun

A tool to quickly run c projects through the command line, without explicitly compiling.

```c
// hello.c
#include <stdio.h>

int main(int argc, char** argv) {
    if (argc < 2) return 1;
    printf("Hello %s!\n", argv[1]);
}
```

```shell
$ crun hello.c Ege
Hello Ege!
```

## Building
You can bootstrap `crun` with an older version of `crun`. The snippet above for example can be executed through the `crun` binary like this:

```shell
$ crun crun.c hello.c Ege
Hello Ege!
```
Here old `crun` invokes the compiler with `crun.c` and calls the compiled binary with args `hello.c Ege`. The new `crun` then invokes the compiler with `hello.c` and calls the resulting executable with the arg `Ege`.

Otherwise the single source file can just be compiled regularly. (GCC example)

```shell
$ gcc crun.c -o crun
```
