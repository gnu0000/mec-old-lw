/*
  
  
 */


#define INCL_WIN
#include <os2.h>
#include <stdlib.h>
#include "listwin.h"
#include "lwframe.h"
#include "lwutil.h"
#include "lwsplit.h"
#include "lwselect.h"
#include "lwclient.h"


#define LW_TIMERID   10


/* returns either
 *
 * uRow       - if a row is selected
 * LWMP_BLANK - if mouse selects blank area
 * LWMP_BELOW - if mouse selects below window
 * LWMP_ABOVE - if mouse selects above window
 *
 */

USHORT RowFromMouse (HWND hClientWnd, MPARAM mp1, USHORT *uDistance)
   {
   SHORT    iYMouse;
   USHORT   uYPos, uRow;
   POINTS   ptsScrPos;
   RECTL    rclClient;

   WinQueryWindowRect (hClientWnd, &rclClient);
   iYMouse = (SHORT)(SHORT2FROMMP (mp1));

   if (iYMouse < 0)
      {
      *uDistance = (USHORT)(-iYMouse); 
      return LWMP_BELOW; /* mouse is below window */
      }
   else if (iYMouse > (SHORT) rclClient.yTop)
      {
      *uDistance = (USHORT)(iYMouse - (SHORT) rclClient.yTop);
      return LWMP_ABOVE; /* mouse is above window */
      }
   uYPos =  (USHORT)rclClient.yTop - (USHORT)iYMouse;
   GetScrollPos (hClientWnd, &ptsScrPos);
   uRow = ptsScrPos.y + uYPos / LWQuery (hClientWnd, LWM_YRowSize, TRUE);
   *uDistance = 0;
   return (uRow >= LWQuery (hClientWnd, LWM_NumRows, TRUE) ? LWMP_BLANK : uRow);
   }



USHORT ColFromMouse (HWND hClientWnd, MPARAM mp1)
   {
   return 0;
   }



USHORT InvalidateRow (HWND hFrameWnd, USHORT uRow)
   {
   SHORT    x, y;
   USHORT   uFirst, uLast, uYRowSize, fOptions;   
   HWND     hClientWnd;
   RECTL    rclClient, rclUpdate;
   POINTS   ptsScrPos;

   if (uRow >= LWQuery (hFrameWnd, LWM_NumRows, FALSE))
      return OK;

   fOptions =  LWQuery (hFrameWnd, LWM_Options, FALSE);
   for (y = 0; y < (fOptions & LWS_HSPLIT ? 2 : 1); y++)
      {
      for (x = 0; x < (fOptions & LWS_VSPLIT ? 2 : 1); x++)
         {
         hClientWnd = WinWindowFromID (hFrameWnd, LWID_CLIENT[1-y][x]);
         GetScrollPos (hClientWnd, &ptsScrPos);
         if ((USHORT)ptsScrPos.y > uRow)
            continue;
         WinQueryWindowRect (hClientWnd, &rclClient);
         if (rclClient.xRight == 0L | rclClient.yTop == 0L)
            continue;

         GetUpdateRange (hClientWnd, &rclClient, ptsScrPos.y,
                         &uFirst, &uLast, &uYRowSize);
         if (uRow >= uFirst && uRow <= uLast)
            {
            rclUpdate = rclClient;
            rclUpdate.yTop = uYRowSize;
            AddBaseYOffset (hClientWnd, ptsScrPos.y, uRow+1, &rclUpdate);
            WinInvalidateRect (hClientWnd, &rclUpdate, FALSE);
            }
         }
      }
   return OK;
   }





/* this routine returns 0xFFFF if the item is visible
 * or the abs scroll value that will just make it visible
 */
USHORT RowVisible (HWND hClientWnd, USHORT uRow)
   {
   POINTS   ptsScrPos;
   USHORT   uWinSize, uYRowSize;
   RECTL    rclClient;

   GetScrollPos (hClientWnd, &ptsScrPos);

   /* row above window */
   if ((USHORT)ptsScrPos.y > uRow)
      return uRow;

   WinQueryWindowRect (hClientWnd, &rclClient);
   uYRowSize = LWQuery (hClientWnd, LWM_YRowSize, TRUE);
   uWinSize = (USHORT)rclClient.yTop / uYRowSize;

   /* select bottom if more than half visible */
   if ((USHORT)rclClient.yTop % uYRowSize > uYRowSize /2)
      uWinSize += 1;

   /* row below window */
   if ((USHORT)ptsScrPos.y + uWinSize <= uRow)
      return uRow - uWinSize + 1;

   /* row visible */
   return 0xFFFF;
   }





USHORT SetNewActive (HWND hClientWnd, USHORT uVSBID, USHORT uOld, USHORT uNew)
   {
   USHORT uScroll;
   HWND   hFrameWnd;

   hFrameWnd = WinQueryWindow (hClientWnd, QW_PARENT, FALSE);

   (LWGetPMET (hFrameWnd, FALSE))->uActive = uNew;
   InvalidateRow (hFrameWnd, uOld);
   WinUpdateWindow (hClientWnd);

   if ((uScroll = RowVisible (hClientWnd, uNew)) != 0xFFFF)
      DoVScroll (hFrameWnd, MPFROMSHORT (uVSBID),
                  MPFROM2SHORT (uScroll, SB_SLIDERTRACK));

   InvalidateRow (hFrameWnd, uNew);
   return uNew;
   }



USHORT StopTimer (HWND hClientWnd, USHORT uTimerDir)
   {
   if (uTimerDir)
      WinStopTimer (WinQueryAnchorBlock (hClientWnd), hClientWnd, LW_TIMERID);
   return 0;
   }

USHORT StartTimer (HWND hClientWnd, USHORT uNew, USHORT uDistance)
   {
   USHORT uDelay;

//   uDelay = 300;
//   uDelay -= (uDistance > 5  ? 50:0);
//   uDelay -= (uDistance > 10 ? 45:0); 
//   uDelay -= (uDistance > 15 ? 40:0); 
//   uDelay -= (uDistance > 20 ? 35:0); 
//   uDelay -= (uDistance > 25 ? 30:0);
//   uDelay -= (uDistance > 30 ? 35:0); 
//   uDelay -= (uDistance > 35 ? 35:0); 
//   uDelay -= (uDistance > 40 ? 30:0); 
   uDelay = 3;

   WinStartTimer (WinQueryAnchorBlock (hClientWnd),
                  hClientWnd, LW_TIMERID, uDelay);
   return uNew;
   }





USHORT   DoSingleSelect (HWND hClientWnd, MPARAM mp1)
   {
   QMSG     qmsg;
   USHORT   uNew, uOld, uMax, uDistance;
   USHORT   uVSBID, uClientID;
   HWND     hFrameWnd;
   PMET     pmet;
   USHORT   uTimerDir = 0;
   HAB      hab;

   WinSetCapture (HWND_DESKTOP, hClientWnd);

   hFrameWnd= WinQueryWindow (hClientWnd, QW_PARENT, FALSE);
   hab      = WinQueryAnchorBlock (hClientWnd);
   pmet     = LWGetPMET (hClientWnd, TRUE);
   uOld     = LWQuery (hFrameWnd, LWM_Active, FALSE);
   uMax     = LWQuery (hFrameWnd, LWM_NumRows, FALSE) - 1;
   uClientID= WinQueryWindowUShort (hClientWnd, QWS_ID);
   uVSBID   = LWID_VSB[(uClientID == LWID_CLIENT[1][0] ||
                      uClientID == LWID_CLIENT[1][1] ? 1 : 0)];
   if ((uNew = RowFromMouse (hClientWnd, mp1, &uDistance)) != uOld)
      uOld = SetNewActive (hClientWnd, uVSBID, uOld, uNew);

   while (WinGetMsg (NULL, (PQMSG)&qmsg, NULL, 0, 0))
      switch (qmsg.msg)
      {
      case WM_BUTTON1UP:
         WinSetCapture (HWND_DESKTOP, NULL);
         StopTimer (hClientWnd, uTimerDir);
         return TRUE;

      case WM_MOUSEMOVE:
         if ((uNew = RowFromMouse (hClientWnd, qmsg.mp1, &uDistance)) == uOld)
            break;

         else if (uNew == LWMP_BLANK)
            uTimerDir = StopTimer (hClientWnd, uTimerDir);

         else if (uNew == LWMP_BELOW && uOld < uMax)   /* below window */
            uTimerDir = StartTimer (hClientWnd, uNew, uDistance);

         else if (uNew == LWMP_ABOVE && uOld > 0)      /* above window */
            uTimerDir = StartTimer (hClientWnd, uNew, uDistance);

         else if (uNew < LWMP_ERROR)                   /* on screen */
            {
            uTimerDir = StopTimer (hClientWnd, uTimerDir);
            uOld = SetNewActive (hClientWnd, uVSBID, uOld, uNew);
            }

         break;

      case WM_TIMER:
         if (LW_TIMERID != SHORT1FROMMP (qmsg.mp1))
            WinDispatchMsg (NULL, (PQMSG)&qmsg);

         if (uTimerDir == LWMP_BELOW && uOld == uMax ||
             uTimerDir == LWMP_ABOVE && uOld == 0)
            uTimerDir = StopTimer (hClientWnd, uTimerDir);

         else if (uTimerDir == LWMP_BELOW)
            uOld = SetNewActive (hClientWnd, uVSBID, uOld, uOld +1);

         else /*(uTimerDir == LWMP_ABOVE)*/
            uOld = SetNewActive (hClientWnd, uVSBID, uOld, uOld -1);

         break;

      default:
         WinDispatchMsg (NULL, (PQMSG)&qmsg);
         break;
      }
   WinSetCapture (HWND_DESKTOP, NULL);
   return TRUE;
   }




//LWDeselectAll ()
//LWSelectRange ()
//LWSelectRow   (T,F)
//BOOL LWRowSelected



//USHORT   DoMultiSelect (HWND hClientWnd, MPARAM mp1)
//   {
//   QMSG     qmsg;
//   USHORT   bShift, bCtrl, bEraseMode;
//   USHORT   uStart
//
//   WinSetCapture (HWND_DESKTOP, hClientWnd);
//
//   pmet   = LWGetPMET (hClientWnd, TRUE);
//   bShift = WinGetKeyState (HWND_DESKTOP, VK_SHIFT) & 0x8000
//   bCtrl  = WinGetKeyState (HWND_DESKTOP, VK_CTRL)  & 0x8000
//   uRow = RowFromMouse (hClientWnd, mp1);
//   bEraseMode = LWRowSelected (hClientWnd, uRow) && bCtrl;
//   uStart = (bShift ? pmet->uStartSel : uRow);
//
//
//   if (!bCtrl)
//      LWDeselectAll (hClientWnd);
//
//   if (!bShift)
//      pmet->uStartSel = uRow;
//   
//   if (bEraseMode)
//      LWSelectRow (hClientWnd, uRow, FALSE);
//   else
//      LWSelectRange (hClientWnd, uRow, uStart);
//
//
//
//   while (WinGetMsg (NULL, (PQMSG)&qmsg, NULL, 0, 0))
//      switch (qmsg.msg)
//      {
//      case WM_BUTTON1UP:
//         return OK;
//
//      case WM_MOUSEMOVE:
//         break;
//
//      default:
//         WinDispatchMsg (NULL, (PQMSG)&qmsg);
//         break;
//      }
//   WinSetCapture (HWND_DESKTOP, NULL);
//   return OK;
//   }
//
//
//
//
//
