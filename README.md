# A Lisp interpreter in C

# Setup
- Linux: 
    - `sudo apt-get install build-essential`
    - `sudo apt-get install libreadline-dev`
    - `gcc -std=c99 -Wall lisp.c -lreadline -o lisp`. //Link readline library
    - `gcc -Wall -g lisp.c -lreadline -o lisp
    - `valgrind --leak-check=full ./lisp
- Mac: 
    - install command line tools 
    - `xcode-select --install`
    - brew install readline

# Reference
- https://en.cppreference.com/w/c.html
