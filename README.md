# DB48X project

The DB48X intends to rebuild and improve the user experience of the HP48, HP49
and HP50 families of calculators, and to offer this experience on various
existing and future calculator hardware. It is really intended to be a platform
to build calculators on, each calculator being highly customizable in terms of
user experience.

## Why name the project DB48X?

DB stands for "Dave and Bill".
Dave and Bill are more commonly known as Hewlett and Packard.
One of their great legacy is a legendary series of calculators.
The HP48SX remains one of my favorites, and this project aims at recreating it.


## How to build this project

This project can build either simulated calculators to run on a PC, Linux or
Mac, or firmware for real calculators.

### Building a simulator

1. Install the required dependencies, notably compiler toolchain and Qt6. The
   correct way to do this depends on your operating system.

2. Run `make` to build all the possible simulators, or `make <target>` to only
   build the simulator for that target. Available targets can obtained by using
   `make help`.

3. If you specified a given target, the `bd48x-<target>` executable is placed in
   the top-level directory, where `<target>` is the name of the target you
   selected. If you did not specify a target, all possible simulators are placed
   in the top-level directory.

### Building a firmware image

1. Install the required dependencies, notably cross-compiler toolchains. The
   correct way to do this depends on your operating system.

2. Run `make prime-fw` for HP Prime firmware, `make dm42-fw` for DM42 firmware,
   `make hp50g-fw` for HP50G firmware, and so on. The list of supported targets is
   available by typing `make help`.

3. The firmware files are stored in a file that that looks like
   `bd48x-<target>-fw.<extension>`, where `<target>` is the name you
   used after `make` and `<extension>` is the suitable extension for that
   target.


### Building the command-line newRPL compiler

An additional target is `make compiler`, which builds a newRPL compiler intended
to run on a computer, and places that compiler under the directory `tools-bin`.
The compiler is automatically built if you build a simulator.


## Credits

This project heavily borrows from a number of existing projects, including:

- [newRPL](https://sourceforge.net/projects/newrpl/) by Claudio Lapilli
- [WP43S](https://gitlab.com/Over_score/wp43s) and [WP43C](https://gitlab.com/Jaymos/wp43c) for [SwissMicros DM42](https://www.swissmicros.com/product/dm42) support
- [Emu48](http://emu48mac.sourceforge.net) for Saturn emulation
- [HPDS 3.0](https://github.com/c3d/HPDS) for HP48 source code, including [PacMan](https://github.com/c3d/HPDS/blob/master/HP48%20Sources/Pac%20Man/PacLib.48S) and [Lemmings](https://github.com/c3d/HPDS/blob/master/HP48%20Sources/Lemmings/Lemmings%202.48S)
