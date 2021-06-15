/* lwclient.h
   list window header module
 */


WINPROC LWLabelProc  (HWND hwnd, USHORT umsg, MPARAM mp1, MPARAM mp2);
WINPROC LWClientProc (HWND hwnd, USHORT umsg, MPARAM mp1, MPARAM mp2);

int GetTextElement  (HWND   hChildWnd,
                     USHORT uRow,
                     USHORT uCol,
                     PSZ    *ppszText);
int GetUpdateRange (HWND hwnd, PRECTL prclUpdate, USHORT uYScrPos,
                USHORT *puFirst, USHORT *puLast,  USHORT *puInc);

int AddBaseYOffset (HWND   hChildWnd,
                    USHORT uYScrPos,
                    USHORT uFirstRow,
                    RECTL  *prclPos);
