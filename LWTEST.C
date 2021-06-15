/*
 * lwtest.c
 * This module creates a list window for test purposes
 */



#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listwin.h"
#include "Error.h"

#define LWID_FRAME 0
char pszStr[] = "NewProc X,Y";
USHORT  bMode = 0;

MRESULT EXPENTRY ClientWndProc (HWND, USHORT, MPARAM, MPARAM);
int InitForGr (HWND hListWnd);
int InitForGr2 (HWND hListWnd);
int InitForGr3 (HWND hListWnd);
int InitForText (HWND hListWnd);
int InitForText2 (HWND hListWnd);
int InitForText3 (HWND hListWnd);


int CreateTestWindow (HAB hab, HWND *hMainWnd, HWND *hListWnd, PSZ pszTxt)
   {
   HWND  hClientWnd;
   CHAR  szClientClass [] = "Welcome";
   RECTL rcl;
   ULONG flFrameFlags = FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER |
                               FCF_MINMAX | FCF_SHELLPOSITION | FCF_TASKLIST;

   WinRegisterClass (hab, szClientClass, ClientWndProc, /*CS_SIZEREDRAW*/ 0, 0);
   if (LWRegister (hab) == NOT_OK)
      ErrDisplayWindowError (hab, NULL);


   *hMainWnd = WinCreateStdWindow (HWND_DESKTOP, WS_VISIBLE, &flFrameFlags,
                     szClientClass, NULL, WS_VISIBLE, (HMODULE) NULL, 0,
                     &hClientWnd);
   if (*hMainWnd == NULL)
      {
      ErrDisplayWindowError (hab, NULL);
      return NOT_OK;
      }

   WinQueryWindowRect (hClientWnd, &rcl);

   *hListWnd = WinCreateWindow (hClientWnd,
                    szITI_LWClass,
                    "",
                    WS_VISIBLE | WS_CLIPCHILDREN,
                    (USHORT) rcl.xLeft,
                    (USHORT) rcl.yBottom,
                    (USHORT) rcl.xRight,
                    (USHORT) rcl.yTop,
                    hClientWnd,
                    HWND_TOP,
                    LWID_FRAME,
                    NULL,
                    NULL);
   if (*hListWnd == NULL)
      {
      ErrDisplayWindowError (hab, NULL);
      return NOT_OK;
      }
   WinSetWindowText (*hMainWnd, pszTxt);
   return 0;
   }



int GetTextStr  (HWND   hChildWnd, USHORT uRow, USHORT uCol, PSZ *ppszText)
   {
   *ppszText = pszStr;
   (*ppszText)[ 8] = (unsigned char) 'A' + (unsigned char) uRow;
   (*ppszText)[10] = (unsigned char) 'A' + (unsigned char) uCol;
   return OK;
   }



int InitDraw (HWND hwnd, HPS hps)
   {
   GpiCreateLogColorTable(hps, 0L, LCOLF_RGB, 0L, 0L, NULL);
   return 0;
   }

              
// { hps,uRow,uCol,prclPos,lFColor,lBColor,ufFlags} DRAWINFO;
int DrawRect (HWND hwnd, PDRAWINFO pdi)
   {
   POINTL     ptl;
   AREABUNDLE abnd;
   
   abnd.lColor = (ULONG)pdi->uRow << 13 | (ULONG)pdi->uCol << 5;
   GpiSetAttrs(pdi->hps, PRIM_AREA, ABB_COLOR, 0L, (PBUNDLE)&abnd);
   ptl.x = pdi->prclPos->xLeft;
   ptl.y = pdi->prclPos->yBottom;
   GpiMove (pdi->hps, &ptl);
   ptl.x = pdi->prclPos->xRight-1;
   ptl.y = pdi->prclPos->yTop-1;
   GpiBox (pdi ->hps, DRO_FILL, &ptl, 0L, 0L);
   return 0;
   }


int DrawRect2 (HWND hwnd, PDRAWINFO pdi)
   {
   POINTL     ptl;
   AREABUNDLE abnd;
   
   abnd.lColor = (ULONG)pdi->uRow << 19 | (ULONG)pdi->uCol << 3;
   GpiSetAttrs(pdi->hps, PRIM_AREA, ABB_COLOR, 0L, (PBUNDLE)&abnd);
   ptl.x = pdi->prclPos->xLeft;
   ptl.y = pdi->prclPos->yBottom;
   GpiMove (pdi->hps, &ptl);
   ptl.x = pdi->prclPos->xRight-1;
   ptl.y = pdi->prclPos->yTop-1;
   GpiBox (pdi ->hps, DRO_OUTLINEFILL, &ptl, 60L, 60L);
   return 0;
   }


int DrawRect3 (HWND hwnd, PDRAWINFO pdi)
   {
   POINTL     ptl;
   AREABUNDLE abnd;
   
//   abnd.lColor = (ULONG)pdi->uRow << 12 | (ULONG)pdi->uCol << 4;
   abnd.lColor = (ULONG)rand() |
                 (((ULONG)rand() & 0x00ff) << 16);
   GpiSetAttrs(pdi->hps, PRIM_AREA, ABB_COLOR, 0L, (PBUNDLE)&abnd);
   ptl.x = pdi->prclPos->xLeft;
   ptl.y = pdi->prclPos->yBottom;
   GpiMove (pdi->hps, &ptl);
   ptl.x = pdi->prclPos->xRight-1;
   ptl.y = pdi->prclPos->yTop-1;
   GpiBox (pdi ->hps, DRO_OUTLINEFILL, &ptl, 20L, 20L);
   return 0;
   }






MRESULT EXPENTRY ClientWndProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
   {
   switch (msg)
      {
      case WM_SIZE:
         {
         HWND     hListWnd;
         USHORT   cx, cy;

         hListWnd = WinWindowFromID (hwnd, LWID_FRAME);
         cx = SHORT1FROMMP (mp2);
         cy = SHORT2FROMMP (mp2);
         WinSetWindowPos (hListWnd, HWND_TOP, 0, 0, cx, cy,
                          SWP_SIZE | SWP_MOVE);
         return 0;
         }
      case WM_LWNQBUTTON:
         {
         HWND     hListWnd;

         hListWnd = WinWindowFromID (hwnd, LWID_FRAME);
         bMode++;
         bMode %= 6;
         switch (bMode)
            {
            case 0: InitForText (hListWnd);  break;
            case 1: InitForGr (hListWnd);    break;
            case 2: InitForText2 (hListWnd); break;
            case 3: InitForGr2 (hListWnd);   break;
            case 4: InitForText3 (hListWnd); break;
            case 5: InitForGr3 (hListWnd);   break;
            }
         WinSendMsg (hListWnd, WM_LWREDRAW, 0L, 0L);
         return 0;
         }
      }
   return WinDefWindowProc (hwnd, msg, mp1, mp2);
   }




// --- LWINIT STRUCTURE ---
//   USHORT uClientRows;
//   USHORT uClientCols;
//   USHORT uLabelCols;    /* may be zero */
//   USHORT uYClientSize;  /* may be zero */
//   USHORT uYLabelSize;   /* may be zero */
//   USHORT uXScrollInc;   /* may be zero */
//   USHORT ufOptions;
// 
// 
int InitForGr (HWND hListWnd)
   {
   USHORT   i, urow = 8, ucol = 8;
   USHORT   uFlags = LWS_VSPLIT | LWS_SELECT | LWS_QBUTTON |
                     LWS_HSPLIT | LWS_SPLITATTOP;
   LWINIT   lwinit = {urow, ucol, 0, 0, 0, 0, uFlags};
   EDT      edtC[8];

   for (i = 0; i < ucol; i++)
      {
      AssignRcl (&(edtC[i].rclPos), i*80, 0, (i+1)*80, 80);
      edtC[i].lFColor = CLR_DARKBLUE;
      edtC[i].ufFlags = DT_VCENTER | DT_LEFT;
      }
   WinSendMsg (hListWnd,WM_LWSETDRAWPROC, (MPARAM)InitDraw, (MPARAM)DrawRect);
   LWInit (hListWnd, &lwinit, edtC, NULL, NULL, NULL);
   WinSendMsg (hListWnd, WM_LWSETCOLOR, MPFROMSHORT(LWC_SPLITMIN), (MPARAM) CLR_BLACK); 
   WinSendMsg (hListWnd, WM_LWSETCOLOR, MPFROMSHORT(LWC_SPLITMAX), (MPARAM) CLR_WHITE); 
   WinSendMsg (hListWnd, WM_LWSETCOLOR, MPFROMSHORT(LWC_CLIENT),   (MPARAM) CLR_RED); 
   WinSetWindowText (WinQueryWindow (hListWnd, QW_PARENT, FALSE),
                     "Graphics Window #1");
   WinSendMsg (hListWnd, WM_LWREDRAW, 0L, 0L);
   return 0;
   }

int InitForGr2 (HWND hListWnd)
   {
   USHORT   i, urow = 32, ucol = 32;
   USHORT   uFlags = LWS_HSCROLL | LWS_VSPLIT | LWS_QBUTTON |
                     LWS_VSCROLL | LWS_HSPLIT;
   LWINIT   lwinit = {urow, ucol, 0, 0, 0, 0, uFlags};
   EDT      edtC[64];

   for (i = 0; i < ucol; i++)
      {
      AssignRcl (&(edtC[i].rclPos), i*60, 0, (i+1)*60, 60);
      edtC[i].lFColor = CLR_DARKBLUE;
      edtC[i].ufFlags = DT_VCENTER | DT_LEFT;
      }
   WinSendMsg (hListWnd,WM_LWSETDRAWPROC, (MPARAM)InitDraw, (MPARAM)DrawRect2);
   LWInit (hListWnd, &lwinit, edtC, NULL, NULL, NULL);
   WinSendMsg (hListWnd, WM_LWSETCOLOR, MPFROMSHORT(LWC_SPLITMIN), (MPARAM) CLR_BLACK); 
   WinSendMsg (hListWnd, WM_LWSETCOLOR, MPFROMSHORT(LWC_SPLITMAX), (MPARAM) CLR_WHITE);
   WinSendMsg (hListWnd, WM_LWSETCOLOR, MPFROMSHORT(LWC_CLIENT), (MPARAM) CLR_GREEN);
   WinSetWindowText (WinQueryWindow (hListWnd, QW_PARENT, FALSE),
                     "Graphics Window #2");
   WinSendMsg (hListWnd, WM_LWREDRAW, 0L, 0L);
   return 0;
   }

int InitForGr3 (HWND hListWnd)
   {
   USHORT   i, urow = 32, ucol = 32;
   USHORT   uFlags = LWS_HSCROLL | LWS_VSPLIT | LWS_QBUTTON |
                     LWS_VSCROLL | LWS_HSPLIT | LWS_SELECT;
   LWINIT   lwinit = {urow, ucol, 0, 40, 0, 0, uFlags};
   EDT      edtC[32];

   for (i = 0; i < ucol; i++)
      {
      AssignRcl (&(edtC[i].rclPos), i*40, 10, i*40+20, 30);
      edtC[i].lFColor = CLR_DARKBLUE;
      edtC[i].ufFlags = DT_VCENTER | DT_LEFT;
      }
   WinSendMsg (hListWnd,WM_LWSETDRAWPROC, (MPARAM)InitDraw, (MPARAM)DrawRect3);
   LWInit (hListWnd, &lwinit, edtC, NULL, NULL, NULL);
   WinSendMsg (hListWnd, WM_LWSETCOLOR, MPFROMSHORT(LWC_SPLITMIN), (MPARAM) CLR_BLACK); 
   WinSendMsg (hListWnd, WM_LWSETCOLOR, MPFROMSHORT(LWC_SPLITMAX), (MPARAM) CLR_WHITE);
   WinSendMsg (hListWnd, WM_LWSETCOLOR, MPFROMSHORT(LWC_CLIENT), (MPARAM) CLR_GREEN);
   WinSetWindowText (WinQueryWindow (hListWnd, QW_PARENT, FALSE),
                     "Graphics Window #3");
   WinSendMsg (hListWnd, WM_LWREDRAW, 0L, 0L);
   return 0;
   }



//    {uClientRows uClientCols @uLabelCols @uYClientSize
//     @uYLabelSize @uXScrollInc ufOptions} LWINIT;

int InitForText (HWND hListWnd)
   {
   USHORT   urow = 40, ucol = 8;
   USHORT   uFlags = LWS_VSPLIT | LWS_LABEL | LWS_QBUTTON |
                     LWS_HSPLIT | LWS_SPLITATTOP | LWS_SELECT;
   LWINIT   lwinit = {urow, ucol, 0, 36, 0, 0, uFlags};
   EDT      edtC[8], edtL[8];
   PSZ      aszLabels[8];
   char     szTmp[100];
   USHORT i;

   for (i = 0; i < ucol; i++)
      {
      AssignRcl (&(edtC[i].rclPos), i*100+10, 19, (i+1)*100, 34);
      AssignRcl (&(edtL[i].rclPos), i*100+10, 19, (i+1)*100, 34);
      edtC[i].lFColor = CLR_DARKBLUE;
      edtC[i].ufFlags = DT_VCENTER | DT_LEFT;
      edtL[i].lFColor = CLR_WHITE;
      edtL[i].ufFlags = DT_VCENTER | DT_CENTER;
      sprintf (szTmp, "Column %d", i);
      aszLabels[i] = strdup (szTmp);
      }
   AssignRcl (&(edtC[i-1].rclPos), 30, 2, 200, 18);
   AssignRcl (&(edtL[i-1].rclPos), 30, 2, 160, 18);

   WinSendMsg (hListWnd,WM_LWSETDRAWPROC, (MPARAM)NULL, NULL);
   LWInit (hListWnd, &lwinit, edtC, edtL, aszLabels, GetTextStr);

   for (i = 0; i < ucol; i++)
      free (aszLabels[i]);

   WinSetWindowText (WinQueryWindow (hListWnd, QW_PARENT, FALSE),
                     "Text Window #1");
   WinSendMsg (hListWnd, WM_LWREDRAW, 0L, 0L);
   return 0;
   }


int InitForText2 (HWND hListWnd)
   {
   USHORT   urow = 80, ucol = 4;
   USHORT   uFlags = LWS_LABEL | LWS_QBUTTON | LWS_SELECT;
   LWINIT   lwinit = {urow, ucol, 0, 0, 0, 0, uFlags};
   EDT      edtC[4], edtL[4];
   PSZ      aszLabels[4];
   char     szTmp[100];
   USHORT i;

   for (i = 0; i < ucol; i++)
      {
      AssignRcl (&(edtC[i].rclPos), i*130, 0, (i+1)*130, 16);
      AssignRcl (&(edtL[i].rclPos), i*130, 0, (i+1)*130, 16);
      edtC[i].lFColor = CLR_YELLOW;
      edtC[i].ufFlags = DT_VCENTER | DT_LEFT;
      edtL[i].lFColor = CLR_BLACK;
      edtL[i].ufFlags = DT_VCENTER | DT_LEFT;
      sprintf (szTmp, "Column %d", i);
      aszLabels[i] = strdup (szTmp);
      }

   WinSendMsg (hListWnd,WM_LWSETDRAWPROC, (MPARAM)NULL, NULL);
   LWInit (hListWnd, &lwinit, edtC, edtL, aszLabels, NULL);

   edtC[2].lFColor = CLR_WHITE;
   WinSendMsg (hListWnd, WM_LWSETEDT, MPFROM2SHORT(TRUE, 2), (MPARAM) (edtC + 2)); 

   WinSendMsg (hListWnd, WM_LWSETCOLOR, MPFROMSHORT(LWC_CLIENT), (MPARAM) CLR_BLUE); 
   WinSendMsg (hListWnd, WM_LWSETCOLOR, MPFROMSHORT(LWC_LABEL), (MPARAM) CLR_WHITE); 

   for (i = 0; i < ucol; i++)
      free (aszLabels[i]);

   WinSetWindowText (WinQueryWindow (hListWnd, QW_PARENT, FALSE),
                     "Text Window #2");
   WinSendMsg (hListWnd, WM_LWREDRAW, 0L, 0L);
   return 0;
   }



int InitForText3 (HWND hListWnd)
   {
   USHORT   urow = 6, ucol = 8;
   USHORT   uFlags = LWS_VSPLIT | LWS_LABEL | LWS_QBUTTON |
                     LWS_HSPLIT | LWS_SPLITATTOP | LWS_SELECT;
   LWINIT   lwinit = {urow, ucol, 0, 36, 0, 0, uFlags};
   EDT      edtC[8], edtL[8];
   PSZ      aszLabels[8];
   char     szTmp[100];
   USHORT i;

   for (i = 0; i < ucol; i++)
      {
      AssignRcl (&(edtC[i].rclPos), i*100+10, 19, (i+1)*100, 34);
      AssignRcl (&(edtL[i].rclPos), i*100+10, 19, (i+1)*100, 34);
      edtC[i].lFColor = CLR_WHITE;
      edtC[i].ufFlags = DT_VCENTER | DT_LEFT;
      edtL[i].lFColor = CLR_WHITE;
      edtL[i].ufFlags = DT_VCENTER | DT_CENTER;
      sprintf (szTmp, "Column %d", i);
      aszLabels[i] = strdup (szTmp);
      }
   AssignRcl (&(edtC[i-1].rclPos), 30, 2, 200, 18);
   AssignRcl (&(edtL[i-1].rclPos), 30, 2, 160, 18);

   WinSendMsg (hListWnd,WM_LWSETDRAWPROC, (MPARAM)NULL, NULL);
   LWInit (hListWnd, &lwinit, edtC, edtL, aszLabels, GetTextStr);

   for (i = 0; i < ucol; i++)
      free (aszLabels[i]);

   WinSendMsg (hListWnd, WM_LWSETCOLOR, MPFROMSHORT(LWC_CLIENT), (MPARAM) CLR_BLUE); 
   WinSetWindowText (WinQueryWindow (hListWnd, QW_PARENT, FALSE),
                     "Text Window #3");
   WinSendMsg (hListWnd, WM_LWREDRAW, 0L, 0L);
   return 0;
   }





main (int argc, char *argv[])
   {
   HAB      hab; 
   HMQ      hmq;
   QMSG     qmsg;
   USHORT   uNum = 10, i=0, n=0, error;
   HWND     hListWnd[100], hMainWnd[100];
   char     pszTxt[100];

   hab = WinInitialize (0);
   hmq = WinCreateMsgQueue (hab ,0);

   do
      {
      sprintf (pszTxt, "win %d", n++);
      error = CreateTestWindow (hab, &hMainWnd[i], &hListWnd[i], pszTxt);
      if (error == OK)
         InitForText (hListWnd[i]);
      i++;
      }
   while (error == OK && i < 1);

   while (WinGetMsg (hab, &qmsg, NULL, 0, 0))
      WinDispatchMsg (hab, &qmsg);

   for (i=0; i<uNum; i++)
      WinDestroyWindow (hListWnd[i]);
   WinDestroyMsgQueue (hmq);
   WinTerminate (hab);
   return OK;
   }
