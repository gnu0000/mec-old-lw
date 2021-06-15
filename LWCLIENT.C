/* lwclient.c
   list window module
   This file contains the code to paint the client and label windows
     and contains the client and label window procs
 */


#define INCL_WIN
#include <os2.h>
// #include <stdio.h>
// #include <string.h>
#include <stdlib.h>
#include "listwin.h"
#include "lwframe.h"
#include "lwclient.h"
#include "lwutil.h"
#include "lwsplit.h"
#include "lwselect.h"
   


char pszDefStr[] = "Element X,Y";

int GetTextElement  (HWND   hChildWnd,
                     USHORT uRow,
                     USHORT uCol,
                     PSZ    *ppszText)
   {
   *ppszText = pszDefStr;
   (*ppszText)[ 8] = (unsigned char) 'A' + (unsigned char) uRow;
   (*ppszText)[10] = (unsigned char) 'A' + (unsigned char) uCol;
   return OK;
   }
              
              

               


int AdjustTextCol   (HWND   hChildWnd,  RECTL  *prclPos,
                     POINTS ptsScrPos,  RECTL  *rclUpdate)
   {
   SHORT iXInc;

   iXInc = LWQuery (hChildWnd, LWM_XScrollInc, TRUE);

   if (rclUpdate->xRight < (prclPos->xLeft  += -ptsScrPos.x * iXInc))
      return NOT_OK;
   if (rclUpdate->xLeft  > (prclPos->xRight += -ptsScrPos.x * iXInc))
      return NOT_OK;
   return OK;
   }







int AddBaseYOffset (HWND   hChildWnd,
                    USHORT uYScrPos,
                    USHORT uFirstRow,
                    RECTL  *prclPos)
   {
   SHORT iAdjust;

   iAdjust = LWQuery (hChildWnd, LWM_YWindowSize,  TRUE) +
             LWQuery (hChildWnd, LWM_YRowSize, TRUE) *
             (uYScrPos - uFirstRow);
   WinOffsetRect (NULL, prclPos, 0, iAdjust);
   return OK;
   }





int GetUpdateRange (HWND hwnd, PRECTL prclUpdate, USHORT uYScrPos,
                USHORT *puFirst, USHORT *puLast,  USHORT *puInc)
   {
   USHORT      uWinTop, uMax;

   *puInc  = LWQuery (hwnd, LWM_YRowSize, TRUE);
   uWinTop = LWQuery (hwnd, LWM_YWindowSize, TRUE);
   *puFirst= uYScrPos + (uWinTop - (USHORT) prclUpdate->yTop) / (*puInc);
   *puLast = *puFirst + (USHORT)(prclUpdate->yTop -
                                prclUpdate->yBottom) / (*puInc);
   uMax = LWQuery (hwnd, LWM_NumRows, TRUE) - 1;
   *puLast = min ((*puLast + 1), uMax);
   return OK;
   }





int PaintClient (HWND hwnd)
   {
   HPS    hps;
   RECTL  rclPos, rclUpdate;
   POINTS ptsScrPos;
   USHORT uNumCols, uCol, uRow, ufFlags, uActive;
   USHORT uFirst, uLast, uInc, ufOptions, uXRowSize;
   PSZ    pszText;
   LONG   lFColor, lBColor, lAColor;
   PEDT   pedt;
   PMET   pmet;
   DRAWINFO diInfo;

   pedt = LWGetPEDT (hwnd, TRUE, TRUE);
   pmet = LWGetPMET (hwnd, TRUE);
   ufOptions = LWQuery (hwnd, LWM_Options, TRUE);

   GetScrollPos (hwnd, &ptsScrPos);
   uNumCols = LWQuery (hwnd, LWM_NumCols, TRUE);
   lBColor = LWColor (hwnd, LWC_CLIENT, TRUE);
   lAColor = LWColor (hwnd, LWC_ACTIVE, TRUE);
   uActive = LWQuery (hwnd, LWM_Active, TRUE);

   hps = WinBeginPaint (hwnd, NULL, &rclUpdate);
   WinFillRect (hps, &rclUpdate, LWColor (hwnd, LWC_CLIENT, TRUE));

   if (pmet->InitClientDraw != NULL)
      (*(pmet->InitClientDraw))(hwnd, hps);


   GetUpdateRange (hwnd, &rclUpdate, ptsScrPos.y, &uFirst, &uLast, &uInc);

   for (uCol = 0; uCol < uNumCols; uCol++)
      {
      rclPos  = pedt[uCol].rclPos;
      if (AdjustTextCol (hwnd, &rclPos, ptsScrPos, &rclUpdate) == OK)
         {
         AddBaseYOffset (hwnd, ptsScrPos.y, uFirst, &rclPos);
         lFColor = pedt[uCol].lFColor;
         ufFlags = pedt[uCol].ufFlags;
         for (uRow = uFirst; uRow <= uLast; uRow++)
            {
            WinOffsetRect (NULL, &rclPos, 0, - (SHORT) uInc);

            if (pmet->DrawClientText == NULL)
               {
               (*(pmet->GetText))(hwnd, uRow, uCol, &pszText);
               WinDrawText (hps, -1, pszText, &rclPos,
                            lFColor, lBColor, ufFlags);
               }
            else /* (pmet->DrawClientText != NULL) */
               {
               diInfo.hps     = hps;
               diInfo.uRow    = uRow;
               diInfo.uCol    = uCol;
               diInfo.prclPos = &rclPos;
               diInfo.lFColor = lFColor;
               diInfo.lBColor = lBColor;
               diInfo.ufFlags = ufFlags;

               (*(pmet->DrawClientText))(hwnd, &diInfo);
               }
            }
         }
      }
   if (uActive >= uFirst && uActive <= uLast)
      {
      uXRowSize = LWQuery (hwnd, LWM_XRowSize, TRUE);
      AssignRcl (&rclPos, 0, 0, uXRowSize, uInc);
      AdjustTextCol (hwnd, &rclPos, ptsScrPos, &rclUpdate);
      AddBaseYOffset (hwnd, ptsScrPos.y, uActive+1, &rclPos);
      WinDrawBorder (hps, &rclPos, 2, 2, lAColor, lAColor, DB_STANDARD);
      }

   WinEndPaint (hps);
   
   return OK;
   }



int PaintLabel (HWND hwnd)
   {
   HPS    hps;
   RECTL  rclPos, rclUpdate;
   POINTS ptsScrPos;
   USHORT uNumCols, uCol, ufFlags;
   LONG   lFColor, lBColor;
   PEDT   pedt;
   PSZ    *ppszTxt, pszText;

   pedt     = LWGetPEDT (hwnd, TRUE, FALSE);
   ppszTxt  = LWGetPPSZ (hwnd);
   GetScrollPos (hwnd, &ptsScrPos);
   uNumCols = LWQuery (hwnd, LWM_NumLabelCols, TRUE);
   lBColor  = LWColor (hwnd, LWC_LABEL, TRUE);

   hps = WinBeginPaint (hwnd, NULL, &rclUpdate);
   WinFillRect (hps, &rclUpdate, LWColor (hwnd, LWC_LABEL, TRUE));

   for (uCol = 0; uCol < uNumCols; uCol++)
      {
      rclPos  = pedt[uCol].rclPos;
      if (AdjustTextCol (hwnd, &rclPos, ptsScrPos, &rclUpdate) == OK)
         {
         pszText = ppszTxt[uCol];
         lFColor = pedt[uCol].lFColor;
         ufFlags = pedt[uCol].ufFlags;
         WinDrawText (hps, -1, pszText, &rclPos, lFColor, lBColor, ufFlags);
         }
      }
   WinEndPaint (hps);

   return OK;
   }




WINPROC LWClientProc (HWND hwnd, USHORT umsg, MPARAM mp1, MPARAM mp2)
   {
   switch (umsg)
      {
      case WM_PAINT:
         PaintClient (hwnd);
         return 0;
         break;

//      case WM_ERASEBACKGROUND:
//         return (MRESULT)TRUE;
//         break;

      case WM_BUTTON1DOWN:
         {
         USHORT   uNewRow, fOptions, uDistance;
//         HWND     hFrameWnd;

         if ((uNewRow = RowFromMouse (hwnd, mp1, &uDistance)) > LWMP_ERROR)
            return (MRESULT)TRUE;

         fOptions = LWQuery (hwnd, LWM_Options, TRUE);
         if (fOptions & LWS_MULTISELECT)
            DoSingleSelect (hwnd, mp1);
         else if (fOptions & LWS_SELECT)
            {
            DoSingleSelect (hwnd, mp1);
//            hFrameWnd = WinQueryWindow (hwnd, QW_PARENT, FALSE);
//            InvalidateRow (hFrameWnd, LWQuery (hwnd, LWM_Active, TRUE));
//            (LWGetPMET (hFrameWnd, FALSE))->uActive = uNewRow;
//            InvalidateRow (hFrameWnd, uNewRow);
            }
         return (MRESULT) TRUE;
         break;
         }

      default:
         return WinDefWindowProc (hwnd, umsg, mp1, mp2);
      }
   return 0L;
   }



WINPROC LWLabelProc  (HWND hwnd, USHORT umsg, MPARAM mp1, MPARAM mp2)
   {
   switch (umsg)
      {
      case WM_PAINT:
         return (MRESULT) PaintLabel (hwnd);
      case WM_ERASEBACKGROUND:
         return (MRESULT) TRUE;
      }
   return WinDefWindowProc (hwnd, umsg, mp1, mp2);
   }
