#include "ui.h"

struct editData dCmdLine;

// OPEN AN EMPTY COMMAND LINE
void uiStartEdit(BINT fullscreen)
{
    if(dCmdLine.State) {
        // WE ALREADY HAVE A COMMAND LINE, REOPEN IT
        if(fullscreen) halSetCmdLineHeight(SCREEN_HEIGHT-halScreen.Menu1-halScreen.Menu2, halScreen.CmdLineFont->BitmapHeight*dCmdLine.DispLines);

        halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight*dCmdLine.DispLines);
    }
    else {
        // START A FRESH COMMAND LINE WITH EMPTY TEXT

        DecompStringEnd


    }
}
