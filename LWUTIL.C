/* lwutil.c
   list window module
   This file contains utility procedures for the other modules
 */



#define INCL_WIN
#include <os2.h>
#include "listwin.h"
#include "lwframe.h"
#include "lwutil.h"
#include "lwsplit.h"

USHORT Init (HWND hFrameWnd)
   {
   return (LWGetPMET (hFrameWnd, FALSE))->Init;
   }


/* valid uCmd values to calculate 
   LWM_YRowSize
   LWM_XRowSize
   LWM_YWindowSize
   LWM_XWindowSize
   LWM_YWindowPos
   LWM_XWindowPos
   LWM_YScrollInc
   LWM_XScrollInc
   LWM_NumRows
   LWM_NumCols
   LWM_NumLabelCols
   LWM_YLabelSize
   LWM_Options
*/

USHORT LWQuery (HWND hwnd, USHORT uCmd, USHORT bChild)
   {
   PMET  pmet;
   SWP swp;

   pmet = LWGetPMET (hwnd, bChild);

   switch (uCmd)
      {
      case LWM_YRowSize:
         return (USHORT)(pmet->rclElementPos.yTop - pmet->rclElementPos.yBottom);
      case LWM_XRowSize:
         return (USHORT)(pmet->rclElementPos.xRight - pmet->rclElementPos.xLeft);
      case LWM_YWindowSize:
         WinQueryWindowPos (hwnd, &swp);
         return swp.cy;
      case LWM_XWindowSize:
         WinQueryWindowPos (hwnd, &swp);
         return swp.cx;
      case LWM_YWindowPos:
         WinQueryWindowPos (hwnd, &swp);
         return swp.y;
      case LWM_XWindowPos:
         WinQueryWindowPos (hwnd, &swp);
         return swp.x;
      case LWM_YScrollInc:
         return (USHORT)(pmet->rclElementPos.yTop - pmet->rclElementPos.yBottom);
      case LWM_XScrollInc:
         return pmet->uXScrollInc;
      case LWM_NumRows:
         return pmet->uClientRows;
      case LWM_NumCols:
         return pmet->uClientCols;
      case LWM_NumLabelCols:
         return pmet->uLabelCols;
      case LWM_YLabelSize:
         return pmet->uYLabel;
      case LWM_Options:
         return pmet->ufOptions;
      case LWM_Active:
         return pmet->uActive;
      default:
         return NOT_OK;
      }
   }





PRECTL ElementRect (HWND hChildWnd)
   {
   return &((LWGetPMET (hChildWnd, TRUE))->rclElementPos);
   }



PEDT LWGetPEDT (HWND hwnd, USHORT bChild, USHORT bClient)
   {
   if (bClient)
      return (LWGetPMET (hwnd, bChild))->pedtClient;
   else
      return (LWGetPMET (hwnd, bChild))->pedtLabel;
   }


PSZ *LWGetPPSZ (HWND hChildWnd)
   {
   return (LWGetPMET (hChildWnd, TRUE))->ppszTxt;
   }


PMET LWGetPMET (HWND hwnd, USHORT bChild)
   {
   if (bChild)
      return (PMET) WinQueryWindowPtr (WinQueryWindow
                                (hwnd, QW_PARENT, FALSE), QWP_MET);
   else
      return (PMET) WinQueryWindowPtr (hwnd, QWP_MET);
   }

                           



LONG LWColor (HWND hwnd, USHORT uColorType, USHORT bChild)
   {
   PMET     pmet;

   pmet = LWGetPMET (hwnd, bChild);

   switch (uColorType)
      {
      case LWC_CLIENT:
         return pmet->lClientColor;
      case LWC_LABEL: 
         return pmet->lLabelColor;
      case LWC_SELECT:
         return pmet->lSelectColor;
      case LWC_ACTIVE:
         return pmet->lActiveColor;
      case LWC_SPLITBORDER:
         return pmet->lSplitBColor;
      case LWC_SPLITMIN:
         return pmet->lSplitMin;
      case LWC_SPLITMAX:
         return pmet->lSplitMax;
      default:
         return CLR_DARKGRAY;
      }
   return CLR_DARKGRAY;
   }




int AssignPts (POINTS *ppts1, POINTS *ppts2,
            short xp1, short yp1,
            short xp2, short yp2)
   {
   ppts1->x = xp1 , ppts1->y = yp1;
   ppts2->x = xp2 , ppts2->y = yp2;
   return OK;
   }


int AssignRcl (RECTL *rcl, LONG xLeft, LONG yBottom,
                           LONG xRight, LONG yTop)
   {
   rcl->xLeft   = xLeft;
   rcl->yBottom = yBottom;
   rcl->xRight  = xRight;
   rcl->yTop    = yTop;
   return OK;
   }




/* This procedure calculates the size and position of all
   the windows in the frame window. currently, it centers the
   splitter windows. this procedure is passed the size of the
   frame window rather than the procedure getting the information
   itself because this proc could conceviably be called before the
   frame window has a chance to actually resize itself.

   fMethod = LWCALC_SCRATCH --- calc all windows from frame only
             LWCALC_SPLIT   --- calc windows from frame and split bars

   this proc uses:  
            SHORT xVSB;
            SHORT yHSB;
            SHORT yHSPLIT;
            SHORT xVSPLIT;
*/





USHORT CalcClientArea (HWND hFrameWnd, PRECTL prclClient)
   {
   SHORT fOptions;

   WinQueryWindowRect (hFrameWnd, prclClient);
   fOptions = LWQuery (hFrameWnd, LWM_Options, FALSE);

   prclClient->xRight  -= (fOptions & LWS_VSCROLL ? xVSB : 0);
   prclClient->yBottom += (fOptions & LWS_HSCROLL ? yHSB : 0);
   prclClient->yTop    -= LWQuery (hFrameWnd, LWM_YLabelSize, FALSE);
   return OK;
   }






/* this routine determines if the split bar is at an edge */
/* iPos = x or y position of the split bar*/
/* this proc assumes y has been switched around so y=0 is a top */
/* -1 = far left or top
    0 = in use
    1 = far right or bottom
*/

USHORT SplitBarAtEdge (HWND hFrameWnd, USHORT uChildID, SHORT iPos)
   {
   RECTL rclClient;

   CalcClientArea (hFrameWnd, &rclClient);
   if (uChildID == LWID_VSPLIT)
      {
      if (iPos - LW_XMINWINDOW < (SHORT) rclClient.xLeft)
         return -1;
      if (iPos + LW_XMINWINDOW > (SHORT) rclClient.xRight)
         return  1;
      return 0;
      }
   else /* uChildID == LWID_HSPLIT */
      /* this proc assumes iPos is relative yTop !!!!!!!!! */
      {
      if (iPos - LW_YMINWINDOW < 0)
         return -1;
      if (iPos + LW_YMINWINDOW > (SHORT) (rclClient.yTop- rclClient.yBottom))
         return  1;
      return 0;
      }
   }

/* tells how split bar should be positioned
   -1 = far left or top
    0 = in use
    1 = far right or bottom
*/

SHORT SplitBarUse (HWND hFrameWnd, USHORT uChildID, int *iNewPos)
   {
   SHORT iWinPos, fOptions, bLock, iEdge, iLockVal;
   RECTL  rclClient;


   fOptions = LWQuery (hFrameWnd, LWM_Options, FALSE);
   iLockVal = (fOptions & LWS_SPLITATTOP ? -1 : 1);
   CalcClientArea (hFrameWnd, &rclClient);

   if (uChildID == LWID_VSPLIT)
      {
      if (!(fOptions & LWS_VSPLIT))
         {
         *iNewPos = (SHORT)rclClient.xRight;
         return 0;
         }
      GetSplitLocation (WinWindowFromID (hFrameWnd, uChildID), &iWinPos, &bLock);
      iEdge = (bLock? iLockVal: SplitBarAtEdge (hFrameWnd, uChildID, iWinPos));

      switch (iEdge)
         {
         case -1 : *iNewPos = (SHORT) rclClient.xLeft;             break;
         case  1 : *iNewPos = (SHORT) rclClient.xRight - xVSPLIT;  break;
         case  0 : *iNewPos = iWinPos;                             break;
         }
      return iEdge;
      }

   else /* LWID_HSPLIT */
      {
      if (!(fOptions & LWS_HSPLIT))
         {
         *iNewPos = (SHORT) rclClient.yBottom - yHSPLIT; 
         return 0; /* try returning 1 */
         }

      GetSplitLocation (WinWindowFromID (hFrameWnd, uChildID), &iWinPos, &bLock);
      iEdge = (bLock ? iLockVal : SplitBarAtEdge (hFrameWnd, uChildID, iWinPos));
      
      switch (iEdge)
         {
         case -1 : *iNewPos = (SHORT) rclClient.yTop - yHSPLIT;   break;
         case  1 : *iNewPos = (SHORT) rclClient.yBottom;          break;
         case  0 : *iNewPos = (SHORT) rclClient.yTop - iWinPos;   break;
         }
      return iEdge;
      }

   }




int CalcSplit ( HWND hFrameWnd,
                POINTS ptsFSize,
                USHORT uChildID,
                POINTS *pptsPos,
                POINTS *pptsSiz)
   {
   int   iNewPos, fOptions;

   fOptions = LWQuery (hFrameWnd, LWM_Options, FALSE);

   if (uChildID == LWID_VSPLIT)
      {
      pptsPos->y = 0;
      pptsSiz->y = ptsFSize.y;
      pptsSiz->x = xVSPLIT;

      if (SplitBarUse (hFrameWnd, uChildID, &iNewPos))
         pptsSiz->y = (fOptions & LWS_HSCROLL ? yHSB : 0);

      pptsPos->x = iNewPos;
      return OK;
      }

   else /* LWID_HSPLIT */
      {
      pptsPos->x = 0;
      pptsSiz->x = ptsFSize.x;
      pptsSiz->y = yHSPLIT;

      if (SplitBarUse (hFrameWnd, uChildID, &iNewPos))
         {
         pptsSiz->x = (fOptions & LWS_VSCROLL ? xVSB : 0);
         pptsPos->x = ptsFSize.x - pptsSiz->x;
         }
      pptsPos->y = iNewPos;
      return OK;
      }
   return NOT_OK;
   }




int CalcOther (HWND hFrameWnd,
               POINTS ptsFSize,
               USHORT uChildID,
               POINTS *pptsPos,
               POINTS *pptsSiz)

   {
   SHORT X0, X1, X2, X3, X1B, X2B;
   SHORT Y0, Y1, Y2, Y3, Y1B, Y2B;
   SHORT iHSplit, iVSplit, iHScrPos, iVScrPos;
   SHORT yLabel, i, j, fOptions, iyHSB, ixVSB;
   RECTL rclClient;

   CalcClientArea (hFrameWnd, &rclClient);

   iHSplit = SplitBarUse (hFrameWnd, LWID_HSPLIT, &iHScrPos);
   iVSplit = SplitBarUse (hFrameWnd, LWID_VSPLIT, &iVScrPos);
   yLabel  = LWQuery (hFrameWnd, LWM_YLabelSize, FALSE);
   fOptions = LWQuery (hFrameWnd, LWM_Options, FALSE);
   iyHSB = (fOptions & LWS_HSCROLL ? yHSB : 0);
   ixVSB = (fOptions & LWS_VSCROLL ? xVSB : 0);

   X0 = (SHORT) rclClient.xLeft;
   X1 = (X1B = iVScrPos) + (iVSplit==1 ? xVSPLIT : 0);
   X2 = (X2B = iVScrPos + xVSPLIT) - (iVSplit==-1 ? xVSPLIT : 0);
   X3 = (SHORT) rclClient.xRight;

   Y0 = (SHORT) rclClient.yBottom;
   Y1 = (Y1B = iHScrPos) + (iHSplit==-1 ? yHSPLIT : 0);
   Y2 = (Y2B = iHScrPos + yHSPLIT) - (iHSplit==1 ? yHSPLIT : 0);
   Y3 = (SHORT) rclClient.yTop;

   for (i = 0; i < 2; i++)
      {
      for (j = 0; j < 2; j++)
         if (uChildID == LWID_CLIENT[j][i])
            return AssignPts (pptsPos, pptsSiz,
                                 (i?X2:X0),       (j?Y2:Y0),
                                 (i?X3-X2:X1-X0), (j?Y3-Y2:Y1-Y0));

      if (uChildID == LWID_LABEL[i])
         return AssignPts (pptsPos, pptsSiz,
                              (i?X2:X0),       Y3,
                              (i?X3-X2:X1-X0), yLabel);

      if (uChildID == LWID_HSB[i])
         return  AssignPts (pptsPos, pptsSiz,
                              (i?X2B:X0),        0,
                              (i?X3-X2B:X1B-X0), iyHSB);

      if (uChildID == LWID_VSB[i])
         return AssignPts (pptsPos, pptsSiz,
                              X3,    (i?Y2B:Y0),
                              ixVSB, (i?Y3-Y2B:Y1B-Y0));
      }
   if (uChildID == LWID_QBUTTON)
         return AssignPts (pptsPos, pptsSiz, X3, 0, ixVSB, iyHSB);
   return NOT_OK;
   }





int CalcWinPosSize ( HWND hFrameWnd,
                     POINTS ptsFSize,
                     USHORT uChildID,
                     POINTS *pptsPos,
                     POINTS *pptsSize)
   {

   if (uChildID == LW_CLIENTAREA)
      {
      RECTL rclClient;

      CalcClientArea (hFrameWnd, &rclClient);
      return AssignPts (pptsPos, pptsSize,
                        (SHORT)rclClient.xLeft, (SHORT)rclClient.yBottom,
                        (SHORT)rclClient.xRight-(SHORT)rclClient.xLeft,
                        (SHORT)rclClient.yTop-(SHORT)rclClient.yBottom);
      }
   if (uChildID == LWID_VSPLIT || uChildID == LWID_HSPLIT)
      return CalcSplit (hFrameWnd, ptsFSize, uChildID, pptsPos, pptsSize);

   return CalcOther (hFrameWnd, ptsFSize, uChildID, pptsPos, pptsSize);

   return NOT_OK;
   }
   




