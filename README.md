# TODO(map) Fill me out

# Building and running
There are two ways that I'm experimenting with this right now. The first is doing cursors and movement and display without linking any libraries. This involes running the program by simply doing:
```
gcc main.c -DNO_LINK
./a.out FILE_NAME
```
If using `ncurses` (which I'm not sure I want to permanently use), you can do
```
gcc main.c -DLINK -lncurses
./a.out FILE_NAME
```
