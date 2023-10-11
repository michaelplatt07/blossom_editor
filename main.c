#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#ifdef LINK
#include <ncurses.h>
#endif

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

    // File to hold temporary changes
    FILE *tmpFilePtr;
}

processKeyPress(int keyPressed, struct Editor *editor) {
    // Always check for the escape key being pressed because regardless of
    // anything else we want to exit insert mode and move into navigation
    // mode.
    if (keyPressed == 27) { // Key code for escape
        editor->insertMode = 0;
    } else if (editor->insertMode == 0) { // Key bindings for navigation
        if (keyPressed == 'j') {
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
    } else if (editor->insertMode == 1) { // Key bindings for typing stuff
        // TODO(map) Implement the insert mode
        int success = fseek(editor->filePtr, editor->xCoor, SEEK_SET);
        if (success == 0) {

            // Copy the contents from the current cursor position onward to a
            // temporary file so they can be added back later.
            char fileBuffer[1];
            while (feof(editor->filePtr) == 0) {
                fread(fileBuffer, sizeof(fileBuffer), 1, editor->filePtr);
                fputs(fileBuffer, editor->tmpFilePtr);
            }
            // Re-seek to the correct spot after moving through the file.
            fseek(editor->filePtr, editor->xCoor, SEEK_SET);

            // Insert the keys to the file if they are pressed
            if (keyPressed == 'j') {
                 fputs("j", editor->filePtr);
            } else if (keyPressed == 'k') {
                 fputs("k", editor->filePtr);
            } else if (keyPressed == 'h') {
                 fputs("h", editor->filePtr);
            } else if (keyPressed == 'l') {
                 fputs("l", editor->filePtr);
            } else {
                printf("Do not know how to insert character");
                exit(1);
            }
            editor->xCoor++;
            printf("\33[%d;%dH", editor->yCoor, editor->xCoor);

            // Place the contents of the temporary file into the original file
            fseek(editor->tmpFilePtr, 0, SEEK_SET);
            while (feof(editor->tmpFilePtr) == 0) {
                fread(fileBuffer, sizeof(fileBuffer), 1, editor->tmpFilePtr);
                fputs(fileBuffer, editor->filePtr);
            }
            // Re-seek to the correct spot after moving through the file.
            fseek(editor->filePtr, editor->xCoor, SEEK_SET);

        } else {
            printf("Error seeking to file with code %d\n", success);
            exit(1);
        }
        // TODO(map) Write to file.
    } else {
        printf("There was a massive error somewhere");
        exit(1);
    }
}

void readFile(struct Editor *editor, const char shortenedName[]) {
    // Read the contents of the file
    editor->filePtr = fopen(shortenedName, "r+");
    editor->tmpFilePtr = fopen("tmp.txt", "ab+");
}

void drawFile(struct Editor *editor) {
    char fileContents[100];
    while (fgets(fileContents, sizeof(fileContents), editor->filePtr)) {
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

    // Get the file name itself using strrchr to find the last instance of a 
    // slash in the fully qualified file path. Then increment the pointer
    // returned by one to move past the forward slash.
    char *shortenedName;
    shortenedName = strrchr(fileName, '/');
    shortenedName++;

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

    // Start at 0,0 in the terminal so the file is printed from the beginning
    printf("\33[0;0H");

    // Loop forever
    while (editor.exitApp == 0) {
                    
        key = getchar();
        processKeyPress(key, &editor);

        // TODO(map) It looks like the terminal itself will be a problem here.
        // We may need to redraw everything and track the changes that have
        // been made to the file.

        // TODO(map) Figure out why initial key press doesn't do anything.
        system("clear"); // Clear the screen
        printf("\33[0;0H"); // Put cursor in upper left corner
        /* rewind(editor.filePtr); // Rewind the pointer so we can reread the file */

        // TODO(map) WIP
        fclose(editor.filePtr);
        readFile(&editor, shortenedName);

        drawFile(&editor); // Print the file contents
        printf("\33[%d;%dH", editor.yCoor, editor.xCoor); // Move cursor to correct new location.
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
    fclose(editor.tmpFilePtr);
    return 0;
}
