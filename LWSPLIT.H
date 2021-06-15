/* lwsplit.h
   list window header module
 */




/* used by UpdateScrollBars */
#define LWSB_SETTHUMBPOS         1
#define LWSB_UPDATETHUMBSIZES    2
#define LWSB_UPDATESCROLLRANGES  3

 /* extra size of split bar intersection */
#define LW_EXTRACENTER           3

/* minimum sizes of window */
#define LW_XMINWINDOW            20
#define LW_YMINWINDOW            20

#define QW_POS                   0
#define QW_HOMELOCK              2

/* pointer resource ID's */
#define PTR_HORZ     1
#define PTR_VERT     2
#define PTR_HV       3

/* pointer resource handles */
extern HPOINTER hHptr;
extern HPOINTER hVptr;
extern HPOINTER hHVptr;

/* this proc uses a handle to a scroll bar and returns its pos, and max pos */
int GetScrollInfo (HWND hScrollWnd, USHORT *uScrollPos, USHORT *uScrollMax);
int GetScrollPos (HWND hChildWnd, POINTS *pptsScrollPos);
int UpdateScrollBars (HWND hFrameWnd, USHORT ucmd, USHORT uChildID, USHORT u1);

WINPROC LWSplitProc  (HWND hwnd, USHORT umsg, MPARAM mp1, MPARAM mp2);

int  SetSplitLocation (HWND hChildWnd, SHORT iPos, SHORT bLock);

int  GetSplitLocation (HWND hChildWnd, SHORT *iPos, SHORT *bLock);

int InitMousePointers (void);
int DeInitMousePointers (void);

