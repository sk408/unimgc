// custom_printf.c
#include "custom_printf.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

extern HWND hEditOutput;  // Declare external reference to the edit control for GUI output

static char lastBuffer[1024] = {0};  // Buffer to store the last printed message

// Helper function to replace '\n' with '\r\n' in a string (but not '\r')
void replace_newlines(char *str, size_t size) {
    char temp[1024];
    int j = 0;
    for (int i = 0; str[i] != '\0' && j < size - 1; i++) {
        if (str[i] == '\n' && (i == 0 || str[i - 1] != '\r')) {  // Only replace '\n' if it's not part of '\r\n'
            if (j < size - 2) {
                temp[j++] = '\r';  // Add carriage return before newline
            }
        }
        temp[j++] = str[i];
    }
    temp[j] = '\0';
    strncpy(str, temp, size - 1);
}

// Custom printf implementation
int custom_printf(const char *format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Replace '\n' with '\r\n' for proper line breaks in Windows EDIT control
    replace_newlines(buffer, sizeof(buffer));

    // If we are in GUI mode, output to the edit control
    if (hEditOutput != NULL) {
        if ((strchr(buffer, '\r') != NULL) && (strchr(buffer,'\r\n') == NULL)) {  // If the message contains a carriage return
            // Overwrite the last line by removing it first
            int textLength = GetWindowTextLength(hEditOutput);

            // Move caret to the end of the text and select from there back to where the last message started
            SendMessage(hEditOutput, EM_SETSEL, (WPARAM)(textLength - strlen(lastBuffer)), (LPARAM)textLength);
            
            // Replace selected text with the new message
            SendMessage(hEditOutput, EM_REPLACESEL, 0, (LPARAM)buffer);

            // Update lastBuffer with the new message for future replacements
            strncpy(lastBuffer, buffer, sizeof(lastBuffer) - 1);
        } else {
            // Normal case: append new text without overwriting
            int textLength = GetWindowTextLength(hEditOutput);

            // Move caret to the end of the text
            SendMessage(hEditOutput, EM_SETSEL, (WPARAM)textLength, (LPARAM)textLength);

            // Append new text
            SendMessage(hEditOutput, EM_REPLACESEL, 0, (LPARAM)buffer);

            // Clear lastBuffer because this is not an overwrite case
            memset(lastBuffer, 0, sizeof(lastBuffer));
        }
    } else {
        // Otherwise, fallback to standard console output
        printf("%s", buffer);
    }

    return 0;
}
