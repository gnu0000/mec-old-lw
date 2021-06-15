/* lwsplit.c
   list window module
   This file contains the code to move split windows
     and contains the split window proc
 */



#define INCL_WIN
#include <os2.h>
#include "listwin.h"
#include "lwframe.h"
#include "lwsplit.h"
#include "lwutil.h"
#include <stdlib.h>

#define DEFAULTPOS  0


HPOINTER hHptr;
HPOINTER hVptr;
HPOINTER hHVptr;


/***********************************************scrollbars*************/
/* valid uses:
   hwnd, LWSB_SETTHUMBPOS,       childID, uNewPos
   hwnd, LWSB_UPDATETHUMBSIZES,   0, 0
   hwnd, LWSB_UPDATESCROLLRANGES, 0, 0

   the childID is the the actual scroll bar to change
*/
int UpdateScrollBars   (HWND hFrameWnd,
                        USHORT ucmd,
                        USHORT uChildID,
                        USHORT u1)
   {
   HWND   hChildWnd;

   switch (ucmd)
      {
      case LWSB_SETTHUMBPOS:
         hChildWnd = WinWindowFromID (hFrameWnd, uChildID);
         WinSendMsg (hChildWnd, SBM_SETPOS, MPFROMSHORT (u1), 0L);
         return OK;

      case LWSB_UPDATETHUMBSIZES:
         {
         SHORT   x, uVisible, uMax;
         USHORT  fOptions;

         fOptions = LWQuery (hFrameWnd, LWM_Options, FALSE);

         if (fOptions & LWS_HSCROLL)
            for (x = 0; x < (fOptions & LWS_VSPLIT ? 2 : 1); x++)
               {
               hChildWnd = WinWindowFromID (hFrameWnd, LWID_HSB[x]);
               uMax     = LWQuery (hChildWnd, LWM_XRowSize, TRUE) /
                          LWQuery (hChildWnd, LWM_XScrollInc, TRUE);
               uVisible = LWQuery (hChildWnd, LWM_XWindowSize, TRUE) /
                          LWQuery (hChildWnd, LWM_XScrollInc, TRUE);
               WinSendMsg (hChildWnd, SBM_SETTHUMBSIZE, MPFROM2SHORT (uVisible, uMax), 0L);
               }

         if (fOptions & LWS_VSCROLL)
            for (x = 0; x < (fOptions & LWS_HSPLIT ? 2 : 1); x++)
               {
               hChildWnd = WinWindowFromID (hFrameWnd, LWID_VSB[1-x]);
               uMax    = (LWQuery (hChildWnd, LWM_NumRows, TRUE)     *
                          LWQuery (hChildWnd, LWM_YRowSize, TRUE))/
                          LWQuery (hChildWnd, LWM_YScrollInc, TRUE);
               uVisible = LWQuery (hChildWnd, LWM_YWindowSize, TRUE) /
                          LWQuery (hChildWnd, LWM_YScrollInc, TRUE);
               WinSendMsg (hChildWnd, SBM_SETTHUMBSIZE, MPFROM2SHORT (uVisible, uMax), 0L);
               }

         return OK;
         }

      case LWSB_UPDATESCROLLRANGES:
         {
         USHORT   uPos, uMax, fOptions, uVisible;
         int      x;

         fOptions = LWQuery (hFrameWnd, LWM_Options, FALSE);

         if (fOptions & LWS_HSCROLL)
            for (x = 0; x < (fOptions & LWS_VSPLIT ? 2 : 1); x++)
               {
               hChildWnd = WinWindowFromID (hFrameWnd, LWID_HSB[x]);
               uPos = SHORT1FROMMP (WinSendMsg (hChildWnd, SBM_QUERYPOS, 0L, 0L));
               uMax =     LWQuery (hChildWnd, LWM_XRowSize, TRUE) /
                          LWQuery (hChildWnd, LWM_XScrollInc, TRUE);
               uVisible = LWQuery (hChildWnd, LWM_XWindowSize, TRUE) /
                          LWQuery (hChildWnd, LWM_XScrollInc, TRUE);
               uMax = max (0, ((int)uMax - (int)uVisible));

               if (uPos > uMax)
                  {
                  MPARAM  mp2;
                  mp2 = MPFROM2SHORT (uMax-uPos, 101 /*LWSB_CUSTOMSCROLL*/);
                  DoHScroll (hFrameWnd, MPFROM2SHORT (LWID_HSB[x], 0), mp2);
                  }

               uPos = min (uPos, uMax);
               WinSendMsg (hChildWnd, SBM_SETSCROLLBAR, MPFROMSHORT (uPos), MPFROM2SHORT (0, uMax));


               }

         if (fOptions & LWS_VSCROLL)
            for (x = 0; x < (fOptions & LWS_HSPLIT ? 2 : 1); x++)
               {
               hChildWnd = WinWindowFromID (hFrameWnd, LWID_VSB[1-x]);
               uPos = SHORT1FROMMP (WinSendMsg (hChildWnd, SBM_QUERYPOS, 0L, 0L));
               uMax = LWQuery (hChildWnd, LWM_YRowSize, TRUE) *
                      LWQuery (hChildWnd, LWM_NumRows, TRUE) /
                      LWQuery (hChildWnd, LWM_YScrollInc, TRUE);
               uVisible = LWQuery (hChildWnd, LWM_YWindowSize, TRUE) /
                          LWQuery (hChildWnd, LWM_YScrollInc, TRUE);
               uMax = max (0, ((int) uMax - (int) uVisible));

               if (uPos > uMax)
                  {
                  MPARAM  mp2;
                  mp2 = MPFROM2SHORT (uMax-uPos, 101 /*LWSB_CUSTOMSCROLL*/);
                  DoVScroll (hFrameWnd, MPFROM2SHORT (LWID_VSB[1-x], 0), mp2);
                  }

               uPos = min (uPos, uMax);
               WinSendMsg (hChildWnd, SBM_SETSCROLLBAR, MPFROMSHORT (uPos), MPFROM2SHORT (0, uMax));
               }
                     
         return OK;
         }

      default:
         return NOT_OK;
      }
   }






/* this proc uses a handle to a scroll bar and returns its
   pos, and max pos
*/

int GetScrollInfo (HWND hScrollWnd,
                   USHORT *uScrollPos,
                   USHORT *uScrollMax)
   {
   *uScrollPos = SHORT1FROMMP (WinSendMsg
                                (hScrollWnd,   SBM_QUERYPOS, 0L, 0L));
   *uScrollMax = SHORT2FROMMP (WinSendMsg
                                (hScrollWnd, SBM_QUERYRANGE, 0L, 0L));
   return OK;
   }



/* this procedure gets a child win of a client window or a label window
   anb returns the scroll bar positions.
*/
int GetScrollPos (HWND hChildWnd, POINTS *pptsScrollPos)
   {
   USHORT uHpos, uVpos, uHMax, uVMax, x, y, uChildID;
   HWND   hFrameWnd;
   POINTS   ptsDummy;

   hFrameWnd = WinQueryWindow (hChildWnd, QW_PARENT, FALSE);
   uChildID  = WinQueryWindowUShort (hChildWnd, QWS_ID);
   
   for (x = 0; x < 2; x++)
      {
      if (uChildID == LWID_LABEL[x])
         {
         GetScrollInfo (WinWindowFromID (hFrameWnd, LWID_HSB[x]), &uHpos, &uHMax);
         return AssignPts (pptsScrollPos, &ptsDummy, uHpos, 0, uHMax, 0);
         }
   
      for (y = 0; y < 2; y++)
         if (uChildID == LWID_CLIENT[y][x])
            {
            GetScrollInfo (WinWindowFromID (hFrameWnd, LWID_HSB[x]), &uHpos, &uHMax);
            GetScrollInfo (WinWindowFromID (hFrameWnd, LWID_VSB[y]), &uVpos, &uVMax);
            return AssignPts (pptsScrollPos, &ptsDummy, uHpos, uVpos, uHMax, uVMax);
            }
      }
   return NOT_OK;
   }


/**************************************************************************/



int InitMousePointers (void)
   {
   hHptr  = WinLoadPointer (HWND_DESKTOP, 0, PTR_HORZ);
   hVptr  = WinLoadPointer (HWND_DESKTOP, 0, PTR_VERT);
   hHVptr = WinLoadPointer (HWND_DESKTOP, 0, PTR_HV);
   return OK;
   }


int DeInitMousePointers (void)
   {
   WinDestroyPointer (hHptr);
   WinDestroyPointer (hVptr);
   WinDestroyPointer (hHVptr);
   return OK;
   }



int InvertRect (HPS hps, PRECTL prclV, PRECTL prclH,
                         BOOL bVSplit, BOOL bHSplit)
   {
   if (bVSplit)
      WinInvertRect (hps, prclV);
   if (bHSplit)
      WinInvertRect (hps, prclH);
   return OK;
   }



int  SetSplitLocation (HWND hChildWnd, SHORT iPos, SHORT bLock)
   {
   WinSetWindowUShort (hChildWnd, QW_POS,      iPos);
   WinSetWindowUShort (hChildWnd, QW_HOMELOCK, bLock);
   return OK;
   }

int  GetSplitLocation (HWND hChildWnd, SHORT *iPos, SHORT *bLock)
   {
   *iPos  = WinQueryWindowUShort (hChildWnd, QW_POS);
   *bLock = WinQueryWindowUShort (hChildWnd, QW_HOMELOCK);
   return *iPos;
   }



int  SetSplitLocation2 (HWND hChildWnd, SHORT iPos)
   {
   /* if it goes from hedge->hedge, lock, dont change preferred location */
   /* if it goes from ~hedge->hedge then lock and viseversa a update loc*/
   USHORT uChildID, bOldLock, fOptions;
   SHORT  iNewEdge, iOldPos, iLockVal;
   HWND   hFrameWnd;


   hFrameWnd = WinQueryWindow (hChildWnd, QW_PARENT, FALSE);
   fOptions = LWQuery (hFrameWnd, LWM_Options, FALSE);
   iLockVal = (fOptions & LWS_SPLITATTOP ? -1 : 1);
   uChildID  = WinQueryWindowUShort (hChildWnd, QWS_ID);

   GetSplitLocation (hChildWnd, &iOldPos, &bOldLock);
   iNewEdge = SplitBarAtEdge (hFrameWnd, uChildID, iPos);

   WinSetWindowUShort (hChildWnd, QW_HOMELOCK, iNewEdge == iLockVal);

   if (!(bOldLock && iNewEdge == iLockVal))
         WinSetWindowUShort (hChildWnd, QW_POS, iPos);
   return OK;
   }





BOOL OtherSplitClose(HWND hSplitWnd, USHORT uChildID, MPARAM mp1)
   {
   USHORT   fOptions;
   SHORT    XPos, YPos;
   SWP      swpH, swpV;
   HWND     hFrameWnd;

   hFrameWnd = WinQueryWindow (hSplitWnd, QW_PARENT, FALSE);
   fOptions = LWQuery (hFrameWnd, LWM_Options, FALSE);

   if (!(fOptions & LWS_HSPLIT && fOptions & LWS_VSPLIT))
      return FALSE;

   WinQueryWindowPos (WinWindowFromID (hFrameWnd, LWID_HSPLIT), &swpH);
   WinQueryWindowPos (WinWindowFromID (hFrameWnd, LWID_VSPLIT), &swpV);
   YPos = (SHORT)SHORT2FROMMP (mp1) + swpV.y;
   XPos = (SHORT)SHORT1FROMMP (mp1) + swpH.x;

   if (uChildID == LWID_VSPLIT)
      return (YPos + LW_EXTRACENTER >= swpH.y &&
              YPos <= swpH.y + swpH.cy + LW_EXTRACENTER);
   else /* (uChildID == LWID_HSPLIT) */
      return (XPos + LW_EXTRACENTER >= swpV.x &&
              XPos <= swpV.x + swpV.cx + LW_EXTRACENTER);
   }
   







int DoSplitMove (HWND hSplitWnd, MPARAM mp1)
   {
   USHORT   uSplitID, uYLabel, fOptions;
   HWND     hFrameWnd, hVSplitWnd, hHSplitWnd;
   SWP      swpH, swpV;
   QMSG     qmsg;
   HPS      hps;
   RECTL    rclV, rclH, rclClient;
   LONG     lxMin, lxMax, lxMouse;
   LONG     lyMin, lyMax, lyMouse;
   BOOL     bHSplit, bVSplit, bVMain;
   SHORT    xExtraOffset, yExtraOffset;

   hFrameWnd = WinQueryWindow (hSplitWnd, QW_PARENT, FALSE);
   uSplitID  = WinQueryWindowUShort (hSplitWnd, QWS_ID);
   uYLabel   = LWQuery (hSplitWnd, LWM_YLabelSize, TRUE);
   fOptions  = LWQuery (hFrameWnd, LWM_Options, FALSE);

   CalcClientArea (hFrameWnd, &rclClient);

   /* lock clients */
   WinLockWindowUpdate (HWND_DESKTOP,
                        WinWindowFromID (hFrameWnd, LWID_CLIENT[1][0]));
   if (fOptions & LWS_VSPLIT)
      WinLockWindowUpdate (HWND_DESKTOP,
                        WinWindowFromID (hFrameWnd, LWID_CLIENT[1][1]));

   if (fOptions & LWS_HSPLIT)
      WinLockWindowUpdate (HWND_DESKTOP,
                        WinWindowFromID (hFrameWnd, LWID_CLIENT[0][0]));

   if ((fOptions & LWS_HSPLIT) && (fOptions & LWS_VSPLIT))
      WinLockWindowUpdate (HWND_DESKTOP,
                        WinWindowFromID (hFrameWnd, LWID_CLIENT[0][1]));

   WinSetCapture (HWND_DESKTOP, hSplitWnd);


   bHSplit = !(bVSplit = (bVMain = (uSplitID == LWID_VSPLIT)));
   if (bVMain)
      bHSplit = OtherSplitClose (hSplitWnd, uSplitID, mp1);
   else
      bVSplit = OtherSplitClose (hSplitWnd, uSplitID, mp1);

   if (bVSplit)
      {
      hVSplitWnd = WinWindowFromID (hFrameWnd, LWID_VSPLIT);
      WinQueryWindowPos (hVSplitWnd, &swpV);
      xExtraOffset = (SHORT)SHORT1FROMMP (mp1);
      AssignRcl (&rclV, swpV.x, rclClient.yBottom,
                 swpV.x + swpV.cx, rclClient.yTop + uYLabel);
      }
   if (bHSplit)
      {
      hHSplitWnd = WinWindowFromID (hFrameWnd, LWID_HSPLIT);
      WinQueryWindowPos (hHSplitWnd, &swpH);
      yExtraOffset = (SHORT)SHORT2FROMMP (mp1);
      AssignRcl (&rclH, rclClient.xLeft, swpH.y,
                 rclClient.xRight, swpH.y + swpH.cy);
      }

   if (bHSplit && bVSplit && bVMain)
      yExtraOffset += swpV.y - swpH.y;
   if (bHSplit && bVSplit && !bVMain)
      xExtraOffset += swpH.x - swpV.x;

   hps = WinGetClipPS (hFrameWnd, NULL,
                       PSF_LOCKWINDOWUPDATE | PSF_PARENTCLIP);

   InvertRect (hps, &rclV, &rclH, bVSplit, bHSplit);

   while (WinGetMsg (NULL, (PQMSG)&qmsg, NULL, 0, 0))
      switch (qmsg.msg)
      {
      case WM_BUTTON1UP:
         InvertRect (hps, &rclV, &rclH, bVSplit, bHSplit);
         WinReleasePS (hps);
         WinLockWindowUpdate (HWND_DESKTOP, NULL); /* unlock windows */
         WinSetCapture (HWND_DESKTOP, NULL);
         if (bVSplit)
            SetSplitLocation2 (hVSplitWnd, (SHORT)rclV.xLeft);
         if (bHSplit)
            SetSplitLocation2 (hHSplitWnd, (SHORT)(rclClient.yTop - rclH.yBottom));
         return OK;

      case WM_MOUSEMOVE:
         InvertRect (hps, &rclV, &rclH, bVSplit, bHSplit);
         if (bVSplit)
            {
            lxMin   = rclClient.xLeft;
            lxMax   = rclClient.xRight - swpV.cx;
            lxMouse = (LONG)((SHORT)SHORT1FROMMP (qmsg.mp1) +
                             (bVMain?swpV.x:swpH.x)         -
                              xExtraOffset);
            rclV.xLeft  = max (lxMin,  min (lxMax, lxMouse));
            rclV.xRight = rclV.xLeft + (LONG)swpV.cx;
            }
         if (bHSplit)
            {
            lyMin    = rclClient.yBottom;
            lyMax    = rclClient.yTop - swpH.cy;
            lyMouse  = (LONG)((SHORT)SHORT2FROMMP (qmsg.mp1) +
                              (bVMain?swpV.y:swpH.y)         -
                              yExtraOffset);
            rclH.yBottom  = max (lyMin,  min (lyMax, lyMouse));
            rclH.yTop     = rclH.yBottom + (LONG)swpH.cy;
            }
         InvertRect (hps, &rclV, &rclH, bVSplit, bHSplit);
         break;

      default:
         WinDispatchMsg (NULL, (PQMSG)&qmsg);
         break;
      }
   WinReleasePS (hps);
   if (bVSplit)
      SetSplitLocation2 (hVSplitWnd, (SHORT)rclV.xLeft);
   if (bHSplit)
      SetSplitLocation2 (hHSplitWnd, (SHORT)(rclClient.yTop - rclH.yBottom));
   WinLockWindowUpdate (HWND_DESKTOP, NULL); /* unlock windows */
   WinSetCapture (HWND_DESKTOP, NULL);
   return OK;
   }





int PaintSplitBar (HWND hwnd)
   {
   HPS    hps;
   RECTL  rclUpdate, rclWin;
   HWND   hOtherWnd, hFrameWnd;
   USHORT uChildID, uOtherID, fOptions;
   SHORT  iDummy;
   SWP    swp0, swp1;
   long   lBColor, lFColor;
   SHORT  iState;

   hFrameWnd = WinQueryWindow (hwnd, QW_PARENT, FALSE);
   uChildID = WinQueryWindowUShort (hwnd, QWS_ID);
   uOtherID = (uChildID == LWID_HSPLIT ? LWID_VSPLIT : LWID_HSPLIT);

   iState = SplitBarUse (hFrameWnd, uChildID, &iDummy);
   lBColor = LWColor (hwnd, LWC_SPLITBORDER, TRUE);
   lFColor = LWColor (hwnd, (iState? LWC_SPLITMIN: LWC_SPLITMAX), TRUE);

   hps = WinBeginPaint (hwnd, NULL, &rclUpdate);

   WinQueryWindowRect (hwnd, &rclWin);
   WinFillRect (hps, &rclUpdate, lFColor);
   WinDrawBorder (hps, &rclWin, 1, 1, lBColor, lBColor, DB_STANDARD);

   fOptions = LWQuery (hFrameWnd, LWM_Options, FALSE);
   if (!(fOptions & LWS_HSPLIT && fOptions & LWS_VSPLIT))
      return FALSE;

   if (!iState && !SplitBarUse (hFrameWnd, uOtherID, &iDummy))
      {
      hOtherWnd = WinWindowFromID (hFrameWnd, uOtherID);

      WinQueryWindowPos (hwnd, &swp0);
      WinQueryWindowPos (hOtherWnd, &swp1);
      AssignRcl (&rclWin, swp1.x  - swp0.x + 1,
                          swp1.y  - swp0.y + 1,
                          swp1.x  - swp0.x - 1 + swp1.cx,
                          swp1.y  - swp0.y - 1 + swp1.cy);
      WinFillRect (hps, &rclWin, lFColor);
      }

   WinEndPaint (hps);
   return 0;
   }



WINPROC LWSplitProc  (HWND hwnd, USHORT umsg, MPARAM mp1, MPARAM mp2)
   {
   switch (umsg)
      {
      case WM_CREATE:
         {
         WinSetWindowUShort (hwnd, QW_POS,      DEFAULTPOS);
         WinSetWindowUShort (hwnd, QW_HOMELOCK, TRUE);
         return 0;
         }

      case WM_PAINT:
         PaintSplitBar (hwnd);
         return 0;

      case WM_BUTTON1DBLCLK:
         {
         USHORT bLock;
         USHORT   uChildID;

         /* possibly move both split bars */
         uChildID = WinQueryWindowUShort (hwnd, QWS_ID);
         if (OtherSplitClose(hwnd, uChildID, mp1) &&
             mp1 != (MPARAM) 0xFFFFFFFFL)
            {
            HWND hFrameWnd, hChildWnd;

            hFrameWnd = WinQueryWindow (hwnd, QW_PARENT, FALSE);
            hChildWnd = WinWindowFromID (hFrameWnd,
                (uChildID == LWID_HSPLIT ? LWID_VSPLIT : LWID_HSPLIT));
            WinSendMsg (hChildWnd, umsg, (MPARAM)0xFFFFFFFFL, 0L); 
            }

         WinUpdateWindow (hwnd);
         bLock = WinQueryWindowUShort (hwnd, QW_HOMELOCK);
         WinSetWindowUShort (hwnd, QW_HOMELOCK, !bLock);

         WinSendMsg (WinQueryWindow (hwnd, QW_PARENT, FALSE),
                                     WM_SPLITMOVE, 0L, 0L);
         return 0;
         }
      case WM_BUTTON1DOWN:
         {
         WinUpdateWindow (hwnd);
         DoSplitMove (hwnd, mp1);
         WinSendMsg (WinQueryWindow (hwnd, QW_PARENT, FALSE),
                                     WM_SPLITMOVE, 0L, 0L);
         return 0;
         }
      case WM_MOUSEMOVE:
         {
         HPOINTER hptr;
         USHORT   uChildID;

         uChildID = WinQueryWindowUShort (hwnd, QWS_ID);
         hptr = ( uChildID == LWID_VSPLIT ? hVptr : hHptr);
         if (OtherSplitClose(hwnd, uChildID, mp1))
            hptr = hHVptr;
         WinSetPointer (HWND_DESKTOP, hptr);
         return 0;
         }
      case WM_MOVE:
         {
         WinInvalidateRect (hwnd, NULL, FALSE);
         return 0;
         }
      }
   return WinDefWindowProc (hwnd, umsg, mp1, mp2);
   }





