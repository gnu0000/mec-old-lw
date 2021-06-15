/* lwclient.c
   list window module
   This file contains the code to paint the frame window
     and contains the frame window and init procs
 */


#define  INCL_WIN
#include <os2.h>
#include "listwin.h"
#include "lwframe.h"
#include "lwutil.h"
#include "lwclient.h"
#include "lwsplit.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>



char szITI_LWClass[]  = "LWList";


USHORT LWID_VSPLIT       =  0x0001;
USHORT LWID_HSPLIT       =  0x0002;

USHORT LWID_LABEL[2]     =  {0x0003, 0x0004};
USHORT LWID_HSB[2]       =  {0x0005, 0x0006};
USHORT LWID_VSB[2]       =  {0x0007, 0x0008};
USHORT LWID_CLIENT[2][2] = {{0x0009, 0x000A},
                            {0x000B, 0x000C}};
USHORT LWID_QBUTTON      =  0x000D;


char szLWLabelClass[]  = "LWLabel";
char szLWClientClass[] = "LWClient";
char szLWSplitClass[]  = "LWSplit";

SHORT xVSB;
SHORT yHSB;
SHORT yHSPLIT;
SHORT xVSPLIT;


#define LWSB_CUSTOMSCROLL 101

int CreateKids (HWND hFrameWnd);
USHORT LWSetColor (HWND hwnd, USHORT uColorType, long lColor);


int GlobalsInit (void)
   {
   xVSB     = (USHORT) WinQuerySysValue (HWND_DESKTOP, SV_CXVSCROLL);
   yHSB     = (USHORT) WinQuerySysValue (HWND_DESKTOP, SV_CYHSCROLL);
   yHSPLIT  = (USHORT) WinQuerySysValue (HWND_DESKTOP, SV_CYSIZEBORDER);
   xVSPLIT  = (USHORT) WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER);
   return OK;
   }



int LWRegister (HAB hab)
   {
   GlobalsInit ();  /* this should be removed eventually */
   InitMousePointers();
   if (WinRegisterClass (hab, szITI_LWClass,  LWFrameProc,  0 /*CS_SIZEREDRAW*/,
      sizeof (PMET)) == FALSE)
      return NOT_OK;
   if (WinRegisterClass (hab, szLWLabelClass,  LWLabelProc,  0, 0) == FALSE)
      return NOT_OK;
   if (WinRegisterClass (hab, szLWClientClass, LWClientProc, 0, 0) == FALSE)
      return NOT_OK;
   if (WinRegisterClass (hab, szLWSplitClass,  LWSplitProc,
       CS_SIZEREDRAW| CS_MOVENOTIFY| CS_SYNCPAINT, 2*sizeof(USHORT))==FALSE)
      return NOT_OK;
   return OK;
   }


int LWTerminate (void)
   {
   DeInitMousePointers ();
   return OK;
   }


int FreeMemory (HWND hwnd)
   {
   USHORT   i;
   PMET     pmet;

   pmet = LWGetPMET (hwnd, FALSE);
   if (pmet->pedtLabel != pmet->pedtClient)
      free (pmet->pedtLabel);
   free (pmet->pedtClient);
   for (i = 0; i < pmet->uLabelCols; i++)
      free (pmet->ppszTxt[i]);
   free (pmet->ppszTxt);
   pmet->Init = FALSE;
   return OK;
   }



int KillKids (HWND hwnd)
   {
   HENUM henum;
   HWND  hChildWnd;

   henum = WinBeginEnumWindows (hwnd);
   while ((hChildWnd = WinGetNextWindow (henum)) != NULL)
      WinDestroyWindow (hChildWnd);
   return 0;
   }


int LWInit (HWND hwnd,
            PLWINIT plwInit,
            EDT aedtClient[],
            EDT aedtLabel[],
            PSZ pszLabelTxt[],
            PFNGETTEXT GetText)
   {
   PMET     pmet;
   USHORT   i, uSize;
   RECTL    rclUnionRect;
   RECTL    rclZero = { 0, 0, 1, 1};
   HAB      hab;

   pmet = LWGetPMET (hwnd, FALSE);

   if (pmet->Init)
      {
      KillKids (hwnd);
      FreeMemory (hwnd);
      }

   pmet->uClientRows = plwInit->uClientRows;
   pmet->uClientCols = plwInit->uClientCols;
   pmet->ufOptions   = plwInit->ufOptions;
   pmet->uActive     = 0xFFFF;
   pmet->uStartSel   = 0;
   pmet->GetText     = (GetText == NULL ? GetTextElement : GetText);

   if (!(plwInit->ufOptions & LWS_LABEL))
      pmet->uLabelCols  = 0;
   else if (plwInit->uLabelCols == 0)              /* default */
      pmet->uLabelCols  = plwInit->uClientCols;
   else
      pmet->uLabelCols  = plwInit->uLabelCols;

   if (plwInit->uXScrollInc == 0)                  /* default */
      pmet->uXScrollInc = 40;
   else
      pmet->uXScrollInc = plwInit->uXScrollInc;

   LWSetColor (hwnd, LWC_RESET, 0L);

   uSize = sizeof (EDT) * pmet->uClientCols;
   pmet->pedtClient = memcpy (malloc (uSize), aedtClient, uSize);


   if (aedtLabel == NULL || !(plwInit->ufOptions & LWS_LABEL)) /* default */
      pmet->pedtLabel = pmet->pedtClient;
   else
      {
      uSize = sizeof (EDT) * pmet->uLabelCols;
      pmet->pedtLabel = memcpy (malloc (uSize), aedtLabel, uSize);
      }

       /* calculate the rectangle bounding an entire entry */
   hab = WinQueryAnchorBlock (hwnd);
   rclUnionRect = pmet->pedtClient[0].rclPos;
   for (i = 1; i < pmet->uClientCols; i++)
      WinUnionRect (hab, &rclUnionRect,
                    &rclUnionRect, &(pmet->pedtClient[i].rclPos));
   WinUnionRect (hab, &rclUnionRect, &rclUnionRect, &rclZero);
//   rclUnionRect.yBottom = 0;


   if (plwInit->uYClientSize > 0)             /* un-default */
      rclUnionRect.yTop = plwInit->uYClientSize;
   pmet->rclElementPos = rclUnionRect;

       /* calculate the height of the label */
   if (!(plwInit->ufOptions & LWS_LABEL))
      pmet->uYLabel = 0;
   else if (plwInit->uYLabelSize > 0)
      pmet->uYLabel = plwInit->uYLabelSize;
   else                                       /* default */
      {
      rclUnionRect = pmet->pedtLabel[0].rclPos;
      for (i = 1; i < pmet->uLabelCols; i++)
         WinUnionRect (hab, &rclUnionRect,
                     &rclUnionRect, &(pmet->pedtLabel[i].rclPos));
      pmet->uYLabel = (USHORT)rclUnionRect.yTop;
      }

   if (plwInit->ufOptions & LWS_LABEL)
      {
      pmet->ppszTxt = (PSZ *) malloc (sizeof (PSZ) * pmet->uLabelCols);
      for (i = 0; i < pmet->uLabelCols; i++)
         pmet->ppszTxt[i] = strdup (pszLabelTxt[i]);
      }
   else
      pmet->ppszTxt = NULL;

   pmet->Init = TRUE;

   CreateKids (hwnd);
   WinSendMsg (hwnd, WM_RESET, 0L, 0L);

   return OK;
   }





//int LWInitClientDraw (HWND hwnd,
//                      FNINITDRAW InitClientDraw,
//                      FNDRAWTEXT DrawClientText)
//   {
//   PMET  pmet;
//
//   pmet = LWGetPMET (hwnd, FALSE);
//
//   pmet->InitClientDraw = InitClientDraw;
//   pmet->DrawClientText = DrawClientText;
//   return OK;
//   }
//


/* valid uColorTypes Are:
 *    LWC_RESET     resets all 4 colors to default values   
 *    LWC_CLIENT    sets client background color
 *    LWC_LABEL     sets label background color
 *    LWC_SELECT    selects selected element background color
 *    LWC_ACTIVE    selects active element outline color
 *
 * any valid color may be used for the color value including
 * the default values defined above
 */                      
                       



int CreateKids (HWND hFrameWnd)
   {
   int    x, y;
   POINTS ptsPos, ptsSize, ptsFSize;
   HWND   hwnd;
   HAB    hab;
   LONG   *pFlags = NULL;
   USHORT fOptions;


   hab = WinQueryAnchorBlock (hFrameWnd);
   ptsFSize.x = LWQuery (hFrameWnd, LWM_XWindowSize, FALSE);
   ptsFSize.y = LWQuery (hFrameWnd, LWM_YWindowSize, FALSE);
   fOptions =   LWQuery (hFrameWnd, LWM_Options, FALSE);

   /* make split windows */
   if (fOptions & LWS_VSPLIT)
      {
      CalcWinPosSize (hFrameWnd, ptsFSize, LWID_VSPLIT, &ptsPos, &ptsSize);
      hwnd = WinCreateWindow (hFrameWnd, szLWSplitClass, "",
             WS_VISIBLE, ptsPos.x, ptsPos.y, ptsSize.x, ptsSize.y,
             hFrameWnd, /*HWND_BOTTOM*/ HWND_TOP, LWID_VSPLIT, pFlags, NULL);
      if (hwnd == NULL)   
         ErrDisplayWindowError (hab, NULL);
      }

   if (fOptions & LWS_HSPLIT)
      {
      CalcWinPosSize (hFrameWnd, ptsFSize, LWID_HSPLIT, &ptsPos, &ptsSize);
      hwnd = WinCreateWindow (hFrameWnd, szLWSplitClass, "",
             WS_VISIBLE, ptsPos.x, ptsPos.y, ptsSize.x, ptsSize.y,
             hFrameWnd, /*HWND_BOTTOM*/ HWND_TOP, LWID_HSPLIT, pFlags, NULL);
      if (hwnd == NULL)   
         ErrDisplayWindowError (hab, NULL);
      }
   /* make VSB Windows */
   for (y = 0; y < (fOptions & LWS_HSPLIT ? 2 : 1); y++)
      {
      if (fOptions & LWS_VSCROLL)
         {
         CalcWinPosSize (hFrameWnd, ptsFSize, LWID_VSB[y], &ptsPos, &ptsSize);
         hwnd = WinCreateWindow (hFrameWnd, WC_SCROLLBAR, "",
                SBS_VERT | WS_VISIBLE, ptsPos.x, ptsPos.y,ptsSize.x, ptsSize.y,
                hFrameWnd, HWND_TOP, LWID_VSB[1-y], pFlags, NULL);
         if (hwnd == NULL)   
            ErrDisplayWindowError (hab, NULL);
         }
      }


   for (x = 0; x < (fOptions & LWS_VSPLIT ? 2 : 1); x++)
      {
      /* make HSB Windows */
      if (fOptions & LWS_HSCROLL)
         {
         CalcWinPosSize (hFrameWnd, ptsFSize, LWID_HSB[x], &ptsPos, &ptsSize);
         hwnd = WinCreateWindow (hFrameWnd, WC_SCROLLBAR, "",
                SBS_HORZ | WS_VISIBLE, ptsPos.x, ptsPos.y, ptsSize.x, ptsSize.y,
                hFrameWnd, HWND_TOP, LWID_HSB[x], pFlags, NULL);
         if (hwnd == NULL)   
            ErrDisplayWindowError (hab, NULL);
         }


      /* make Label Windows */
      if (fOptions & LWS_LABEL)
         {
         CalcWinPosSize (hFrameWnd, ptsFSize, LWID_LABEL[x], &ptsPos, &ptsSize);
         hwnd = WinCreateWindow (hFrameWnd, szLWLabelClass, "",
                WS_VISIBLE, ptsPos.x, ptsPos.y, ptsSize.x, ptsSize.y,
                hFrameWnd, HWND_TOP, LWID_LABEL[x], pFlags, NULL);
         if (hwnd == NULL)   
            ErrDisplayWindowError (hab, NULL);
         }

      for (y = 0; y < (fOptions & LWS_HSPLIT ? 2 : 1); y++)
         {
         /* make client windows */
         CalcWinPosSize (hFrameWnd, ptsFSize, LWID_CLIENT[y][x], &ptsPos, &ptsSize);
         hwnd = WinCreateWindow (hFrameWnd, szLWClientClass, "",
                WS_VISIBLE, ptsPos.x, ptsPos.y, ptsSize.x, ptsSize.y,
                hFrameWnd, HWND_TOP, LWID_CLIENT[1-y][x], pFlags, NULL);
         if (hwnd == NULL)   
            ErrDisplayWindowError (hab, NULL);
         }
      }
   /* new buttons */
   if (fOptions & LWS_QBUTTON) 
      {
      CalcWinPosSize (hFrameWnd, ptsFSize, LWID_QBUTTON, &ptsPos, &ptsSize);
      hwnd = WinCreateWindow (hFrameWnd, WC_BUTTON, "?",
                               WS_VISIBLE | BS_PUSHBUTTON | BS_NOPOINTERFOCUS,
                               ptsPos.x, ptsPos.y,
                               ptsSize.x, ptsSize.y,
                               hFrameWnd, HWND_BOTTOM,
                               LWID_QBUTTON, pFlags, NULL);
      if (hwnd == NULL)   
            ErrDisplayWindowError (hab, NULL);
       }
   return OK;
   }



int ScrollClientWindow (HWND hFrameWnd,
                        USHORT uScrollID,
                        SHORT iX, SHORT iY)
   {
   HWND hScrollWnd;

   hScrollWnd = WinWindowFromID (hFrameWnd, uScrollID);
   WinScrollWindow (hScrollWnd,
                    -iX * LWQuery (hScrollWnd, LWM_XScrollInc, TRUE), 
                     iY * LWQuery (hScrollWnd, LWM_YScrollInc, TRUE),
                     (PRECTL) NULL, (PRECTL) NULL, (HRGN) NULL,
                     (PRECTL) NULL, SW_INVALIDATERGN);
   return OK;
   }

/* mp1L = uScrollID
 * mp2l = SliderTrackPosition
 * mp2H = ScrollCmd
 */


int DoVScroll (HWND hFrameWnd, MPARAM mp1, MPARAM mp2)
   {
   USHORT   uScrollID, uMax, uPos, uRow;
   HWND     hScrollWnd;
   SHORT    i, x;
   USHORT   fOptions;

   uScrollID = SHORT1FROMMP (mp1);
   hScrollWnd = WinWindowFromID (hFrameWnd, SHORT1FROMMP (mp1));
   fOptions = LWQuery (hFrameWnd, LWM_Options, FALSE);
   GetScrollInfo (hScrollWnd, &uPos, &uMax);

   switch (SHORT2FROMMP (mp2))
      {
      case SB_LINEUP:
         i = -1;
         break;
      case SB_LINEDOWN:
         i =  1;
         break;
      case SB_PAGEUP:
         i = -LWQuery (hScrollWnd, LWM_YWindowSize, TRUE) /
              LWQuery (hScrollWnd, LWM_YScrollInc, TRUE);
         break;
      case SB_PAGEDOWN:
         i =  LWQuery (hScrollWnd, LWM_YWindowSize, TRUE) /
              LWQuery (hScrollWnd, LWM_YScrollInc, TRUE);
         break;
      case SB_SLIDERTRACK:
         i =  SHORT1FROMMP (mp2) - uPos;
         break;
      case LWSB_CUSTOMSCROLL:
         i =  SHORT1FROMMP (mp2);
         break;
      default:
         i =  0;
         break;
      }

   if (!(i = max (-((SHORT) uPos), min ((SHORT)uMax - (SHORT)uPos, i))))
      return OK;
   uRow = (uScrollID==LWID_VSB[0] ? 0 : 1);
   WinSendMsg (hScrollWnd, SBM_SETPOS, MPFROMSHORT (uPos + i), 0L);
   for (x = 0; x < (fOptions &LWS_VSPLIT ? 2 : 1); x++)
      ScrollClientWindow (hFrameWnd,  LWID_CLIENT[uRow][x], 0, i);
   }

   
int DoHScroll (HWND hFrameWnd, MPARAM mp1, MPARAM mp2)
   {
   USHORT   uScrollID, uMax, uPos, uCol;
   HWND     hScrollWnd;
   SHORT    i, x;
   USHORT   fOptions;

   uScrollID = SHORT1FROMMP (mp1);
   hScrollWnd = WinWindowFromID (hFrameWnd, uScrollID);
   fOptions = LWQuery (hFrameWnd, LWM_Options, FALSE);

   GetScrollInfo (hScrollWnd, &uPos, &uMax);

   switch (SHORT2FROMMP (mp2))
      {
      case SB_LINELEFT:
         i = -1;
         break;
      case SB_LINERIGHT:
         i =  1;
         break;
      case SB_PAGELEFT:
         i = -LWQuery (hScrollWnd, LWM_XWindowSize, TRUE) /
              LWQuery (hScrollWnd, LWM_XScrollInc, TRUE);
         break;
      case SB_PAGERIGHT:
         i =  LWQuery (hScrollWnd, LWM_XWindowSize, TRUE) /
              LWQuery (hScrollWnd, LWM_XScrollInc, TRUE);
         break;
      case SB_SLIDERTRACK:
         i =  SHORT1FROMMP (mp2) - uPos;
         break;
      case LWSB_CUSTOMSCROLL:
         i =  SHORT1FROMMP (mp2);
         break;
      default:
         return OK;
      }

   if (!(i = max (-(SHORT)uPos, min ((SHORT)uMax - (SHORT)uPos, i))))
      return OK;
   uCol = (uScrollID==LWID_HSB[0] ? 0 : 1);
   WinSendMsg (hScrollWnd, SBM_SETPOS, MPFROMSHORT (uPos + i), 0L);
   for (x = 0; x < (fOptions &LWS_HSPLIT ? 2 : 1); x++)
      ScrollClientWindow (hFrameWnd, LWID_CLIENT[1-x][uCol], i, 0);
   if (fOptions & LWS_LABEL)
      ScrollClientWindow (hFrameWnd, LWID_LABEL [uCol], i, 0);
   WinUpdateWindow (WinWindowFromID (hFrameWnd, LWID_LABEL[uCol]));
   }



int PaintFrameWin (HWND hwnd)
   {
   HPS   hps;
   RECTL rectl;

   hps = WinBeginPaint (hwnd, (HPS)NULL, &rectl);
   WinFillRect (hps, &rectl, LWColor (hwnd, LWC_LABEL, FALSE));
   WinEndPaint (hps);
   return OK;
   }



int SizeWin (HWND hFrameWnd, HWND hOrder, POINTS ptsFSize,
             USHORT uChildID, USHORT uWinFlags)
   {
   HWND     hChildWnd;
   POINTS   ptsPos, ptsSize;

   hChildWnd = WinWindowFromID (hFrameWnd, uChildID);
   CalcWinPosSize (hFrameWnd, ptsFSize, uChildID, &ptsPos, &ptsSize);
   WinSetWindowPos (hChildWnd, hOrder, ptsPos.x, ptsPos.y,
                     ptsSize.x, ptsSize.y, uWinFlags);
   return OK;
   }



/*
   this uses the current size of the frame window and the position of
   the splitter windows to resize all the other windows.
   mp contains: low:frame win width,
                hi :frame win height.
*/
int SizeAllChildWindows (HWND hFrameWnd, MPARAM mp)
   {
   int      x, y;
   USHORT   fOptions, fWinFlags;
   POINTS   ptsFSize;

   if (Init (hFrameWnd) == FALSE)
      return OK;

   fOptions =   LWQuery (hFrameWnd, LWM_Options, FALSE);
   fWinFlags =  SWP_MOVE | SWP_SIZE;
   ptsFSize.x = SHORT1FROMMP (mp);
   ptsFSize.y = SHORT2FROMMP (mp);

   for (x = 0; x < (fOptions & LWS_VSPLIT ? 2 : 1); x++)
      {
      for (y = 0; y < (fOptions & LWS_HSPLIT ? 2 : 1); y++)
         SizeWin (hFrameWnd, HWND_TOP, ptsFSize, LWID_CLIENT[1-y][x], fWinFlags);
      if (fOptions & LWS_LABEL)
         SizeWin (hFrameWnd, HWND_TOP, ptsFSize, LWID_LABEL[x], fWinFlags);
      if (fOptions & LWS_HSCROLL)
         SizeWin (hFrameWnd, HWND_TOP, ptsFSize, LWID_HSB[x], fWinFlags);
      }

   for (y = 0; y < (fOptions & LWS_HSPLIT ? 2 : 1); y++)
      {
      if (fOptions & LWS_VSCROLL)
         SizeWin (hFrameWnd, HWND_TOP, ptsFSize, LWID_VSB[1-y], fWinFlags);
      }

//   if (fOptions & LWS_HSPLIT)
//      SizeWin (hFrameWnd, HWND_TOP, ptsFSize, LWID_CLIENT[0][0], fWinFlags);

//   if ((fOptions & LWS_HSPLIT) && fOptions & LWS_VSPLIT)
//      SizeWin (hFrameWnd, HWND_TOP, ptsFSize, LWID_CLIENT[0][1], fWinFlags);

   if (fOptions & LWS_VSPLIT)
      SizeWin (hFrameWnd, HWND_BOTTOM, ptsFSize, LWID_VSPLIT, fWinFlags);

   if (fOptions & LWS_HSPLIT)
      SizeWin (hFrameWnd, HWND_BOTTOM, ptsFSize, LWID_HSPLIT, fWinFlags);

   if (fOptions & LWS_QBUTTON) 
      SizeWin (hFrameWnd, HWND_BOTTOM, ptsFSize, LWID_QBUTTON, fWinFlags);

   UpdateScrollBars (hFrameWnd, LWSB_UPDATETHUMBSIZES, 0, 0);
   UpdateScrollBars (hFrameWnd, LWSB_UPDATESCROLLRANGES, 0, 0);
   return OK;
   }


/************************************************************************/
/******************LWFrameProc support routines end**********************/
/************************************************************************/

/************************************************************************/
/*****************LWFrameProc message routines start*********************/
/************************************************************************/



USHORT LWSetColor (HWND hwnd, USHORT uColorType, long lColor)
   {
   PMET     pmet;

   pmet = LWGetPMET (hwnd, FALSE);
   switch (uColorType)
      {
      case LWC_RESET:
         pmet->lClientColor = LWCLR_DEFCLIENT;
         pmet->lLabelColor  = LWCLR_DEFLABEL ;
         pmet->lSelectColor = LWCLR_DEFSELECT;
         pmet->lActiveColor = LWCLR_DEFACTIVE;
         pmet->lSplitBColor = LWCLR_DEFSPLITBORDER;
         pmet->lSplitMin    = LWCLR_DEFSPLITMIN;
         pmet->lSplitMax    = LWCLR_DEFSPLITMAX;
         break;
      case LWC_CLIENT:
         pmet->lClientColor = lColor;
         break;
      case LWC_LABEL: 
         pmet->lLabelColor  = lColor;
         break;
      case LWC_SELECT:
         pmet->lSelectColor = lColor;
         break;
      case LWC_ACTIVE:
         pmet->lActiveColor = lColor;
         break;
      case LWC_SPLITBORDER:
         pmet->lSplitBColor = lColor;
         break;
      case LWC_SPLITMIN:
         pmet->lSplitMin    = lColor;
         break;
      case LWC_SPLITMAX:
         pmet->lSplitMax    = lColor;
         break;
      default:
         return NOT_OK;
      }
   return OK;
   }



/************************************************************************/
/******************LWFrameProc message routines end**********************/
/************************************************************************/


/* this is the window procedure for the LW frame */

WINPROC LWFrameProc (HWND hwnd, USHORT umsg, MPARAM mp1, MPARAM mp2)
   {
   PMET  pmet;

   switch (umsg)
      {
      case WM_CREATE:
         /* init met structure */
         pmet = (PMET) malloc (sizeof (MET));
         WinSetWindowPtr (hwnd, QWP_MET, pmet);
         pmet->InitClientDraw = pmet->DrawClientText = NULL;
         pmet->Init = FALSE;
         return FALSE;
         break;

      case WM_SIZE:
         SizeAllChildWindows (hwnd, mp2); 
         break;

      case WM_RESET:
         {
         RECTL rcl;

         WinQueryWindowRect (hwnd, &rcl);
         mp2 = MPFROM2SHORT (rcl.xRight, rcl.yTop);
         SizeAllChildWindows (hwnd, mp2);
         WinInvalidateRegion (hwnd, NULL, TRUE);
         break;
         }

      case WM_SPLITMOVE:
         {
         SWP swp;

         WinQueryWindowPos (hwnd, &swp);
         SizeAllChildWindows (hwnd, MPFROM2SHORT (swp.cx,swp.cy));
         break;
         }

      case WM_VSCROLL:
         DoVScroll (hwnd, mp1, mp2);
         break;

      case WM_HSCROLL:
         DoHScroll (hwnd, mp1, mp2);
         break;

      case WM_PAINT:
         PaintFrameWin (hwnd);
         break;

      case WM_DESTROY:
         {
         if ((LWGetPMET (hwnd, FALSE))->Init)
            FreeMemory (hwnd);
         free (LWGetPMET (hwnd, FALSE));
         break;
         }

      case WM_LWSETEDT:             /* (lbl=f|clnt=t,c)   (PEDT)  */
         {
         USHORT bClient, uCol;

         pmet = LWGetPMET   (hwnd, FALSE);
         bClient = SHORT1FROMMP (mp1);
         uCol = SHORT2FROMMP (mp1);  
         if (uCol >= (bClient ? pmet->uClientCols : pmet->uLabelCols))
            return 0;
         (LWGetPEDT (hwnd, FALSE, bClient))[uCol] = *((PEDT)mp2);
         break;
         }

      case WM_LWSETLABEL:           /* (c,-)              (PSZ)   */
         {
         USHORT uCol;

         pmet = LWGetPMET (hwnd, FALSE);
         uCol = SHORT1FROMMP (mp1);

         if (uCol >= pmet->uLabelCols)
            return 0;
         free ((pmet->ppszTxt)[uCol]);
         (pmet->ppszTxt)[uCol] = strdup ((PSZ) mp2);
         break;
         }

      case WM_LWSETDRAWPROC:        /* (pINIT)            (pDRAW) */
         {
         pmet = LWGetPMET (hwnd, FALSE);
         pmet->InitClientDraw = (PFNINITDRAW) mp1;
         pmet->DrawClientText = (PFNDRAWTEXT) mp2;
         break;
         }

      case WM_LWSETCOLOR:           /* (type,-)           (lColor)*/
         LWSetColor (hwnd, SHORT1FROMMP (mp1), (long) mp2);
         break;

      case WM_LWSETTEXTPROC:        /* (pTEXT)            (-)     */
         (LWGetPMET (hwnd, FALSE))->GetText = (PFNGETTEXT) mp1;
         break;

      case WM_LWROWCHANGED:         /* (first,last)       (-)     */
         break;

      case WM_LWROWDELETED:         /* (first,last)       (-)     */
         break;

      case WM_LWROWINSERTED:        /* (first,last)       (-)     */
         break;

      case WM_LWQUERYNUMSELECTIONS: /* (-)                (-)     */
         break;

      case WM_LWQUERYSELECTION:     /* (s#=f|?s=t,indx|r) (-)     */
         break;

      case WM_LWREDRAW:             /* (all=f|client=t)   (-)     */
         {
         USHORT fOptions, y, x;

         if (!SHORT1FROMMP (mp1))
            WinInvalidateRect (hwnd, NULL, TRUE);
         else
            {
            fOptions = LWQuery (hwnd, LWM_Options, FALSE);
            for (y = 0; x < (fOptions & LWS_HSPLIT ? 2U : 1U); x++)
               for (x = 0; x < (fOptions & LWS_VSPLIT ? 2U : 1U); x++)
                  WinInvalidateRect(WinWindowFromID
                     (hwnd, LWID_CLIENT[1-y][x]), NULL, TRUE);
            }
         break;
         }

      case WM_LWMAXSIZE:            /* (pRECT|NULL)       (-)     */
         {
         RECTL rcl, rclAdj = { 0,0,0,0 };

         rclAdj = (mp1 == NULL ? rclAdj : *((PRECTL) mp1));
         WinQueryWindowRect (WinQueryWindow (hwnd, QW_PARENT, FALSE), &rcl);
         WinSetWindowPos (hwnd, HWND_TOP,
                          (SHORT) rclAdj.xLeft,
                          (SHORT) rclAdj.yBottom,
                          (SHORT) (rcl.xRight - rclAdj.xRight),
                          (SHORT) (rcl.yTop - rclAdj.yTop),
                          SWP_MOVE | SWP_SIZE);
         break;
         }

      case WM_LWVIEWROW:            /* (r,c)              (-)     */
         break;

      case WM_LWSCROLLCLIENT:       /* (r,c)           (rel=f|abs=t,-) */
         break;

      case WM_LWMAKEACTIVE:         /* (r)                (-)     */
         break;

      case WM_COMMAND:
         {
         switch (SHORT1FROMMP(mp1))
            {
            case 0x000D:
               WinSendMsg (WinQueryWindow (hwnd, QW_PARENT, FALSE),
                           WM_LWNQBUTTON, (MPARAM) hwnd, 0L);
//               WinMessageBox (HWND_DESKTOP, hwnd,
//                  "Project : XXXXXXXX \nJob     : XXXXXXXX \nItem    : XXXXXXXX \n This is descriptive information \n about this window",
//                              "Window Type", 1,
//                               MB_OK | MB_ICONASTERISK | MB_HELP | MB_MOVEABLE | MB_APPLMODAL);
               return 0;
            default:
               return WinDefWindowProc (hwnd, umsg, mp1, mp2);
            }
         }
      default:
         return WinDefWindowProc (hwnd, umsg, mp1, mp2);
      }
   return 0;
   }


