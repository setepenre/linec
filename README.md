# linec

Toy compiler made using flex, bison and llvm.

## dependencies
- flex 2.6.4
- bison (GNU Bison) 3.3.2
- LLVM version 8.0.0
- g++ (Ubuntu 8.3.0-6ubuntu1) 8.3.0

## setup

```bash
    $ make
```

## usage

```bash
    $ bin/linec [input] [output]
```

### example/

```bash
    $ cat example/example.lc
    + 1 2
    $ bin/linec example/example.lc bin/example
    $ bin/example
    3.000000
```
