#include <windows.h>
#include <uxtheme.h>
#include <commdlg.h>
#include <CommCtrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <process.h>
#include "custom_printf.h"

#define IDC_OUTPUT 1001  // Identifier for the output text box
#define ID_FILE_OPEN 9001 // Menu command identifier
typedef struct {
    int argc;
    char **argv;
} MainArgs;

HWND hEditOutput;  // Handle to the edit control for output
HFONT hFont;       // Handle to the larger font

// Function to display an Open File Dialog using GetOpenFileName (for input file)
BOOL OpenFileDialog(HWND hwnd, char* filePath) {
    OPENFILENAME ofn;       // common dialog box structure
    char szFile[260];       // buffer for file name

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box
    if (GetOpenFileName(&ofn) == TRUE) {
        strcpy(filePath, ofn.lpstrFile);
        return TRUE;
    }
    return FALSE;
}

// Function to display a Save File Dialog using GetSaveFileName (for output file)
BOOL SaveFileDialog(HWND hwnd, char* filePath) {
    OPENFILENAME ofn;       // common dialog box structure
    char szFile[260];       // buffer for file name

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0'; 
    ofn.nMaxFile = sizeof(szFile);
ofn.lpstrFilter = "IMGC Files (*.img)\0*.img\0All Files (*.*)\0*.*\0";
ofn.nFilterIndex = 1; // Set the default filter to IMGC Files

    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    
    // Allow user to enter new filenames
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    // Display the Save dialog box
    if (GetSaveFileName(&ofn) == TRUE) {
        strcpy(filePath, ofn.lpstrFile);
        return TRUE;
    }
    
    return FALSE;
}

// Function to create a larger font (24pt)
HFONT CreateLargeFont() {
    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    
    lf.lfHeight = -24;  // Font height (negative value for point size)
    lf.lfWeight = FW_NORMAL;
    
   strcpy(lf.lfFaceName, "Consolas");  // Set font family
    
    return CreateFontIndirect(&lf);
}

int custom_main(int argc, char *argv[]);
// Function prototype for thread function
unsigned __stdcall RunCustomMain(void *args);

// Main Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
case WM_CREATE: {
    // Create a larger font (24pt) 
    hFont = CreateLargeFont();
    
    // Create edit control (unchanged)
    hEditOutput = CreateWindowEx(
        WS_EX_CLIENTEDGE, 
        "EDIT", 
        "", 
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        20, 20, 460, 300,
        hwnd, 
        (HMENU)IDC_OUTPUT,
        GetModuleHandle(NULL),
        NULL
    );
    
    // Set the larger font for the edit control
    SendMessage(hEditOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
/*
HWND hToolbar = CreateWindowEx(
    0,
    TOOLBARCLASSNAME,
    NULL,
    WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | CCS_TOP,
    0, 0, 0, 0,
    hwnd,
    NULL,
    GetModuleHandle(NULL),
    NULL
);

// Initialize common controls
SendMessage(hToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

// Set button size to be much larger
SendMessage(hToolbar, TB_SETBUTTONSIZE, 0, MAKELPARAM(100, 30));

// Add standard bitmaps
TBADDBITMAP tbab;
tbab.hInst = HINST_COMMCTRL;
tbab.nID = IDB_STD_SMALL_COLOR;
SendMessage(hToolbar, TB_ADDBITMAP, 0, (LPARAM)&tbab);

// Configure button
TBBUTTON tbb[1];
ZeroMemory(&tbb, sizeof(tbb));
tbb[0].iBitmap = STD_FILEOPEN;
tbb[0].idCommand = ID_FILE_OPEN;
tbb[0].fsState = TBSTATE_ENABLED;
// tbb[0].fsStyle = BTNS_AUTOSIZE | BTNS_SHOWTEXT;  // Changed this line
tbb[0].dwData = 0;
tbb[0].iString = (INT_PTR)L"Open File";

// Add buttons
SendMessage(hToolbar, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

// Auto-size the toolbar
SendMessage(hToolbar, TB_AUTOSIZE, 0, 0);
*/
    break;
}


        case WM_SIZE: {
            // Allow resizing of the edit control when the window is resized
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            MoveWindow(hEditOutput,
                       20, 20,
                       rcClient.right - 40,
                       rcClient.bottom - 40,
                       TRUE);
            
            break;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_FILE_OPEN) {  // Open selected from toolbar/menu
                char inputFile[MAX_PATH];
                char outputFile[MAX_PATH];

               if (OpenFileDialog(hwnd, inputFile)) {
                   custom_printf("Selected input file: %s\n", inputFile);

                   if (SaveFileDialog(hwnd, outputFile)) {  // Use Save File Dialog for output file
                       custom_printf("Selected output file: %s\n", outputFile);

                       // Prepare arguments for custom_main
                       char *argv[] = { "file_program", inputFile, outputFile };
                       int argc = sizeof(argv) / sizeof(argv[0]);

                       MainArgs *mainArgs = malloc(sizeof(MainArgs));
                       mainArgs->argc = argc;
                       mainArgs->argv = malloc(argc * sizeof(char *));

                       for (int i = 0; i < argc; i++) {
                           mainArgs->argv[i] = strdup(argv[i]);
                       }

                       HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, RunCustomMain, mainArgs, 0, NULL);

                       if (hThread == NULL) {
                           custom_printf("Failed to create thread for custom_main.\n");
                       } else {
                           CloseHandle(hThread);
                       }
                   }
               }
           }
           break;

        case WM_DESTROY:
           DeleteObject(hFont); // Clean up font object when destroying window
           PostQuitMessage(0);
           return 0;

        default:
           return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

// WinMain: Entry point for a Windows application
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "UnIMGC - IMGC Extractor";

    WNDCLASS wc = {0};
HWND consoleW = GetConsoleWindow();
ShowWindow(consoleW,SW_HIDE);
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    InitCommonControls();

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "UnIMGC - .IMGC Extractor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Enable DPI awareness for high-DPI displays
    SetProcessDPIAware();

    // Create a simple menu with an "Open" option and add it to the window
    HMENU hMenuBar = CreateMenu();
    
    HMENU hMenuFile = CreateMenu();
    
    AppendMenu(hMenuFile, MF_STRING, ID_FILE_OPEN, "&Open\tCtrl+O"); // Add keyboard shortcut Ctrl+O
    
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hMenuFile, "&File");
    
    SetMenu(hwnd, hMenuBar);

    MSG msg = {0};

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
