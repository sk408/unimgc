#include <windows.h>
#include <process.h>
#include "custom_printf.h"
typedef struct {
    int argc;
    char **argv;
} MainArgs;


// Thread function that calls custom_main with arguments passed from main.c.
unsigned __stdcall RunCustomMain(void *args) {
    
   MainArgs *mainArgs = (MainArgs *)args;

   custom_printf("Starting custom_main in background thread...\n");

   // Call custom_main with provided arguments.
   int result = custom_main(mainArgs->argc, mainArgs->argv);

   custom_printf("custom_main finished with result: %d\n", result);

   // Free allocated memory.
   for (int i = 0; i < mainArgs->argc; i++) {
       free(mainArgs->argv[i]);
   }
   free(mainArgs->argv);
   free(mainArgs);

   _endthreadex(0); 
   return result;
}