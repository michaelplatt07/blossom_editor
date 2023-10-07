#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>

int main(int argc, char *argv[]) {
    struct termios info;  // Holds attributes for the terminal

    // Editor values
    int xCoor = 0;
    int yCoor = 0;
    char key;

    // Insert mode tracking
    int insertMode = 0;

    // Tracks if the app should exit
    int exitApp = 0;

    // Clear the screen and set the cursor location
    system("clear");

    // TODO(map) Remove me stuff once I can enter a file name correctly through
    // the editor
    if (argc != 2) {
        printf("Improper use of the file.");
        return 0;
    } 
    char *fileName = argv[1];
    printf("Loading file %s\n", fileName);
    sleep(2); // TODO(map) Sleep for two seconds so I can see the transition
    system("clear"); // TODO(map) Remove this when the sleep goes away

    // Substring the path to the file to be able to get just the file name to
    // be passed to loading the contents
    char *x;
    int index;
    x = strrchr(fileName, '/');
    index = (int)(x - fileName) + 1;
    /* printf("Index: %d\n", index); */
    char shortenedName[50];
    strncpy(shortenedName, fileName + index, strlen(fileName) - index);
    /* printf("File name: %s\n", shortenedName); */

// Code that doesn't use ncurses at all
#ifdef NO_LINK
    // Read the contents of the file
    FILE *filePtr;
    filePtr = fopen(shortenedName, "r");
    char fileContents[100];
    while (fgets(fileContents, 100, filePtr)) {
        printf("%s", fileContents);
    }
    fclose(filePtr);

    // Get the terminal info
    tcgetattr(0, &info);
    info.c_lflag &= ~ICANON;
    info.c_cc[VMIN] = 1;
    info.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &info);

    // TODO(map) Put on a splash screen at some point

    // Set the cursor location. This is done here because if it isn't then
    // the printf calls up till now will happen at the designated location.
    printf("\33[%d;%dH", yCoor, xCoor); 

    // Loop forever
    while (exitApp == 0) {

        key = getchar();
        // NOTE(map) This can be removed once I've captured all the keys
        printf("Key pressed = %d", key);
            
        // When certain keys are pressed do an action
        if (key == 9) { // Key code for escape
            insertMode = 0;
        } else if (key == 'j' && !insertMode) {
             yCoor += 1;
        } else if (key == 'k' && !insertMode) {
             yCoor -= 1;
        } else if (key == 'h' && !insertMode) {
             xCoor -= 1;
        } else if (key == 'l' && !insertMode) {
             xCoor += 1;
        } else if (key == 'i' && !insertMode) {
            insertMode = 1;
        } else if (key == 'q') {
            exitApp = 1;
        }

        // Redraw the cursor at the new location
        system("clear");
        printf("\33[%d;%dH", yCoor, xCoor); 

    }

    // Before program exit, set the terminal info back to normal
    tcgetattr(0, &info);
    info.c_lflag |= ICANON;
    tcsetattr(0, TCSANOW, &info);

// Start of code linkining using ncurses
#elif LINK

    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    // Load the file contents and print them on the screen
    FILE *filePtr;
    filePtr = fopen(shortenedName, "r");
    char fileContents[100];
    while (fgets(fileContents, 100, filePtr)) {
        printw("%s", fileContents);
    }
    fclose(filePtr);
    move(yCoor, xCoor);
    refresh();

    while (exitApp == 0) {

        key = getch();
            
        // When certain keys are pressed do an action
        if (key == 9) { // Key code for escape
            insertMode = 0;
        } else if (key == 'j' && !insertMode) {
             yCoor += 1;
        } else if (key == 'k' && !insertMode) {
             yCoor -= 1;
        } else if (key == 'h' && !insertMode) {
             xCoor -= 1;
        } else if (key == 'l' && !insertMode) {
             xCoor += 1;
        } else if (key == 'i' && !insertMode) {
            insertMode = 1;
        } else if (key == 'q') {
            exitApp = 1;
        }

        move(yCoor, xCoor);
        refresh();
    }

    endwin();

// End all code
#endif

    return 0;
}
