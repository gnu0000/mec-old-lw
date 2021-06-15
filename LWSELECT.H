/*
  
  
 */
#define LWMP_BLANK  0xFFFF
#define LWMP_BELOW  0xFFFE
#define LWMP_ABOVE  0xFFFD
#define LWMP_ERROR  0xFFF0

USHORT RowFromMouse (HWND hClientWnd, MPARAM mp1, USHORT *uDistance);
USHORT InvalidateRow (HWND hFrameWnd, USHORT uRow);
USHORT DoSingleSelect (HWND hClientWnd, MPARAM mp1);
