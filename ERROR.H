/*
 * Error.h
 * Mark Hampton
 * Copyright (C) 1990 Info Tech, Inc.
 *
 * This module displays various operating system error messages.
 */


/*
 * ErrDispalyWindowError displays the last Win subsystem error for
 * the PM thread identified by hab.  hwndActivate is the window that
 * is activated when this function returns.  If hwndActivate is not NULL,
 * this has the effect of creating an application modal dialog box.
 */

extern void ErrDisplayWindowError (HAB hab, HWND hwndActivate);
