# Crun
A tool to quickly run c projects through the command line, without explicitly compiling.

```c
#include <stdio.h>

int main(int argc, char** argv) {
    if (argc < 2) return 1;
    printf("Hello %s!", argv[1]);
}
```

```c
crun main.c -- Ege
```