#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>

struct Editor {
    // Editor values
    int xCoor;
    int yCoor;
    char lastKeyPressed;

    // Insert mode tracking
    int insertMode;

    // Tracks if the app should exit
    int exitApp;

    // Current file that is being edited
    FILE *filePtr;
}

processKeyPress(int keyPressed, struct Editor *editor) {
    // Always check for the escape key being pressed because regardless of
    // anything else we want to exit insert mode and move into navigation
    // mode.
    if (keyPressed == 9) { // Key code for escape
        editor->insertMode = 0;
    } else if (!editor->insertMode) { // Key bindings for navigation
        if (keyPressed == 106) {
             editor->yCoor += 1;
        } else if (keyPressed == 'k') {
             editor->yCoor -= 1;
        } else if (keyPressed == 'h') {
             editor->xCoor -= 1;
        } else if (keyPressed == 'l') {
             editor->xCoor += 1;
        } else if (keyPressed == 'i') {
            editor->insertMode = 1;
        } else if (keyPressed == 'q') {
            editor->exitApp = 1;
        }
    } else if (editor->insertMode) { // Key bindings for typing stuff
        // TODO(map) Implement the insert mode
    } else {
        printf("There was a massive error somewhere");
        exit(1);
    }
}

void readFile(struct Editor *editor, char shortenedName[]) {
    // Read the contents of the file
    editor->filePtr = fopen(shortenedName, "r");
}

void drawFile(struct Editor *editor) {
    char fileContents[100];
    while (fgets(fileContents, 100, editor->filePtr)) {
#ifdef NO_LINK
        printf("%s", fileContents);
#elif LINK
        printw("%s", fileContents);
#endif
    }

}

int main(int argc, char *argv[]) {
    struct termios info;  // Holds attributes for the terminal

    // Holds the data for the editor itself
    struct Editor editor;
    // Init the editor struct with the appropriate values
    editor.xCoor = 0;
    editor.yCoor = 0;
    editor.insertMode = 0;
    editor.exitApp = 0;

    char key; // Holder for the key that is pressed

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
    // Set the file on the editor
    readFile(&editor, shortenedName);
    drawFile(&editor);

    // Get the terminal info
    tcgetattr(0, &info);
    info.c_lflag &= ~ICANON;
    info.c_cc[VMIN] = 1;
    info.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &info);

    // TODO(map) Put on a splash screen at some point

    // Set the cursor location. This is done here because if it isn't then
    // the printf calls up till now will happen at the designated location.
    printf("\33[%d;%dH", editor.yCoor, editor.xCoor);

    // Loop forever
    while (editor.exitApp == 0) {

        key = getchar();
                    
        processKeyPress(key, &editor);

        // Redraw the cursor at the new location
        /* system("clear"); */
        // TODO(map) It looks like the terminal itself will be a problem here.
        // We may need to redraw everything and track the changes that have
        // been made to the file.
        printf("\33[%d;%dH", editor.yCoor, editor.xCoor);

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
    readFile(&editor, shortenedName);
    drawFile(&editor);

    move(editor.yCoor, editor.xCoor);
    refresh();

    while (editor.exitApp == 0) {

        key = getch();
            
        processKeyPress(key, &editor);

        // TODO(map) Should the move be in the processKeyPress?
        move(editor.yCoor, editor.xCoor);
        refresh();
    }

    endwin();

// End all code
#endif

    // Clean up editor data including closing the pointer to the file.
    fclose(editor.filePtr);
    return 0;
}
