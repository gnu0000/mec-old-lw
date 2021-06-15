/* lwutil.h
   list window header module
 */

#define LWM_YRowSize             0
#define LWM_XRowSize             1
#define LWM_YWindowSize          2
#define LWM_XWindowSize          3
#define LWM_YWindowPos           4
#define LWM_XWindowPos           5
#define LWM_YScrollInc           6
#define LWM_XScrollInc           7
#define LWM_NumRows              8
#define LWM_NumCols              9
#define LWM_NumLabelCols         10
#define LWM_YLabelSize           11
#define LWM_Options              12
#define LWM_Active               13


USHORT LWQuery (HWND hwnd, USHORT uCmd, USHORT bChild);


int CalcWinPosSize ( HWND hFrameWnd,
                     POINTS ptsFSize,
                     USHORT uChildID,
                     POINTS *pptsPos,
                     POINTS *pptsSize);

USHORT CalcClientArea (HWND hFrameWnd, PRECTL prclClient);
USHORT SplitBarAtEdge (HWND hFrameWnd, USHORT uChildID, SHORT iPos);
int AssignPts (POINTS *ppts1, POINTS *ppts2,
               short xp1, short yp1,
               short xp2, short yp2);

SHORT  SplitBarUse (HWND hFrameWnd, USHORT uChildID, int *iNewPos);
USHORT Init (HWND hFramewnd);
PRECTL ElementRect (HWND hChildWnd);

PEDT LWGetPEDT (HWND hwnd, USHORT bChild, USHORT bClient);
PMET LWGetPMET (HWND hwnd, USHORT bChild);
PSZ *LWGetPPSZ (HWND hChildWnd);
LONG LWColor (HWND hChildWnd, USHORT uColorType, USHORT bChild);

