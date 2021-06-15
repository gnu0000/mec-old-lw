/* lwframe.h
   list window header module
 */


/*
   WM_SPLITMOVE
     this message is sent from the splitter bar proc to the
     frame proc when a splitter bar is moved.  
   WM_CHANGENUMELEMENTS
     this message is sent to the frame proc when the number of elements
     has been changed.  mp2 = new number of elements
   WM_UPDATEIFVISIBLE
     this message is sent from the frame window to the client or label
     window when an item must be updated from other than a paint condition.
     mp2 = element number to be updated.
   WM_SELECTELEMENT
     this message is sent from a client window to the frame window when
     a selection is changed
     mp1, mp2
*/

/* winproc user messages */
#define LWINTMSG_START            WM_USER+ 200
#define WM_SPLITMOVE             (LWINTMSG_START + 0)
#define WM_RESET                 (LWINTMSG_START + 1)
#define LWID_FRAME                0x0000

/* pointer index into reserved memory for frame window pmet */
#define QWP_MET                  0

/*
   This structure is internally kept by the frame window
   It is initially set up by the Init proc
*/


typedef struct { USHORT Init;
                 USHORT uClientRows;
                 USHORT uClientCols;
                 USHORT uLabelCols;
                 USHORT uActive;
                 USHORT uStartSel;
                 LONG   lClientColor;
                 LONG   lLabelColor;
                 LONG   lSelectColor;
                 LONG   lActiveColor;
                 LONG   lSplitBColor;
                 LONG   lSplitMin;
                 LONG   lSplitMax;
                 USHORT uXScrollInc;
                 USHORT ufOptions;
                 PEDT   pedtClient;
                 PEDT   pedtLabel;
                 PSZ    *ppszTxt;
                 /* these are calculated from above for speed */
                 USHORT uYLabel;
                 RECTL  rclElementPos;
                 int    (*GetText) (HWND hwnd, USHORT uRow, USHORT uCol, PSZ *ppszText);
                 int    (*InitClientDraw) (HWND hwnd, HPS hps);
                 int    (*DrawClientText) (HWND hwnd, PDRAWINFO pdiInfo);

               } MET;

typedef MET FAR *PMET;





extern USHORT LWID_LABEL[2];
extern USHORT LWID_HSB[2];
extern USHORT LWID_VSB[2];
extern USHORT LWID_CLIENT[2][2];
                          
extern USHORT LWID_VSPLIT;
extern USHORT LWID_HSPLIT;

extern USHORT LWID_QBUTTON;
#define LW_CLIENTAREA             0x0100

/* these globals will be in the global module */
extern SHORT xVSB;
extern SHORT yHSB;
extern SHORT yHSPLIT;
extern SHORT xVSPLIT;

extern char szLWLabelClass[];
extern char szLWClientClass[];
extern char szLWSplitClass[];


#define WINPROC MRESULT EXPENTRY

WINPROC LWFrameProc (HWND hwnd, USHORT umsg, MPARAM mp1, MPARAM mp2);

int DoVScroll (HWND hFrameWnd, MPARAM mp1, MPARAM mp2);
int DoHScroll (HWND hFrameWnd, MPARAM mp1, MPARAM mp2);

int SizeAllChildWindows (HWND hFrameWnd, MPARAM mp);

