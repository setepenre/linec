# linec

Toy compiler made using flex, bison and llvm.

## dependencies
- flex 2.6.4
- bison (GNU Bison) 3.7.6
- LLVM version 11.1.0
- g++ 10.2.0

## setup

```bash
    $ make
```

## usage

```bash
    $ bin/linec -h
    usage: linec [options] source executable
      linec compiles source file to executable file

    options:
      -h, --help  display this message
      --llvm      emits llvm IR to standard output
```

### example/

```bash
    $ cat example/example.lc
    a = + 0 1
    b = + a 1
    c = + a b
    + c 2
    $ bin/linec example/example.lc bin/example
    $ bin/example
    5.000000
```
