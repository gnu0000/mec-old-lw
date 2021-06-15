/*
 * Error.c
 * Mark Hampton
 * Copyright (C) 1990 Info Tech, Inc.
 *
 */

#define INCL_WIN
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "error.h"


void ErrDisplayWindowError (HAB hab, HWND hwndActivate)
   {
   PERRINFO perrinfo;
   SEL      selError;
   PUSHORT  pusOffsets;
   ERRORID   usError;

   perrinfo = WinGetErrorInfo (hab);
   usError  = WinGetLastError (hab);

   if ((perrinfo == NULL) && usError == 0)
      {
      WinMessageBox (HWND_DESKTOP, hwndActivate,
         "ErrDisplayWindowError was called, but no error was found.",
         "ErrDisplayWindowError", 0,
         MB_OK | MB_ICONEXCLAMATION | MB_HELP | MB_MOVEABLE | MB_SYSTEMMODAL);
      return;
      }

   if (perrinfo != NULL)
      {
      /* get the selector to the error message segment */
      selError = SELECTOROF (perrinfo);

      /* get a pointer to the array of offsets to messages */
      pusOffsets = MAKEP (selError, perrinfo->offaoffszMsg);

      WinMessageBox (HWND_DESKTOP, hwndActivate,
         MAKEP (selError, pusOffsets [0]), "Win Subsystem Error",
         0, MB_OK | MB_ICONEXCLAMATION | MB_HELP | MB_MOVEABLE |
         MB_APPLMODAL);

      WinFreeErrorInfo (perrinfo);
      }
   else
      {
      char szBuffer [100];

      sprintf (szBuffer, "Unknown error %d", usError);
      WinMessageBox (HWND_DESKTOP, hwndActivate,
         szBuffer, "Win Subsystem Error",
         0, MB_OK | MB_ICONEXCLAMATION | MB_HELP | MB_MOVEABLE |
         MB_APPLMODAL);


      }
   }


