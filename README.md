# ls-clone-cloj

This is a clone of a common `ls` UNIX utility. It lists files in a directory passed as an argument.

You can supply `--tree / -t` option to this utility to list a recursive tree of a given directory with its subdirectories.

## Installation

Download from http://github.com/Killavus/ls-clone-cloj.

## Usage

    $ java -jar ls-clone-cloj-0.1.0-standalone.jar [args]

## Options

* `--tree / -t` - instead of returning a shallow list of files, return them in a tree-formatted format with subdirectories extended. Does not follow symlinks.

## To learn:

* Parsing command options
* Using `readdir` common POSIX API within Clojure
* Writing using typical POSIX tools (`write` / `read`)
* Judge for testing. Provide similar implementation in C and compare results.
