/*
 * $Log: xtty.cc,v $
 * Revision 1.2  2005/09/26 10:45:31  wahid
 * X11 application name changed
 *
 * Revision 1.1.1.1  2005/01/27 13:42:45  wahid
 * First project import
 * Wahid
 *
 * Revision 1.1  2002/03/19 15:03:22  boris
 * Vcitty model addition
 *
 * Revision 1.1.1.1  2002/02/28 12:58:33  disydent
 * Creation of Disydent CVS Tree
 *
 * Revision 1.1.1.1  2001/11/19 16:55:32  pwet
 * Changing the CVS tree structure of disydent
 *
 * Revision 1.1.1.1  2001/07/24 13:31:45  pwet
 * cvs tree of part of disydent
 *
 * Revision 1.1.1.1  2001/07/19 14:32:25  pwet
 * New cvs tree
 *
 * Revision 1.3  2000/11/14 14:04:02  pwet
 * Added an argument to main in tty.c and an argument in the call to xtty in
 * pitty.c in order for the tty name to reflect the name of the file used
 * to in the netlist.
 *
 * Revision 1.2  1998/09/16 16:11:32  pwet
 * passage a cvs
 *
 * Revision 1.1  1998/09/01 09:49:14  pwet
 * Initial revision
 *
 * Revision 1.1  1998/07/16 18:03:59  pwet
 * Initial revision
 *
 * Authors: Frédéric Pétrot and Denis Hommais 
 */

//#ident "$Id: xtty.cc,v 1.2 2005/09/26 10:45:31 wahid Exp $"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <signal.h>
#include "xtty.h"

#include <X11/Xutil.h>
#include <X11/Xos.h>


#ifdef __STDC__
static void ttySetGraphics(Colormap cmap, char *fname, xtty *tty)
#else
static void ttySetGraphics(cmap, fname, tty)
Colormap cmap;
char *fname;
xtty *tty;
#endif
{
XColor def, fit;
register int i;
static char *colorName[2] = {"Black", "Green"};
int  colors[2];
XGCValues vGC;
Font         font;
XFontStruct *qfst;


   XFreeColormap(tty->display, cmap);

   if ((qfst = XLoadQueryFont(tty->display, fname)) == NULL) {
         fprintf(stderr, "Font %s not available from X11 database.\n", fname);
         exit(-1);
   }
   tty->font_height  = qfst->ascent + qfst->descent;
   tty->font_width   = qfst->max_bounds.rbearing - qfst->min_bounds.lbearing;
   font = qfst->fid;
   /* XFreeFontInfo(&fname, qfst, 1); */

   for (i = 0; i < 2; i++) {
      if (!XAllocNamedColor(tty->display, cmap, 
               colorName[i], &def, &fit)) {
         colorName[i]=strdup("white");
         XAllocNamedColor(tty->display, cmap, colorName[i], &def, &fit);
      }
      colors[i] = def.pixel;
   }

   vGC.fill_style = FillSolid;
   vGC.foreground = colors[1];
   vGC.background = colors[0];
   vGC.plane_mask = AllPlanes;
   vGC.font = font;
   tty->gc = XCreateGC(tty->display,
                          RootWindow(tty->display,
                                     DefaultScreen(tty->display)),
                          (GCForeground | GCBackground | GCFillStyle | 
                             GCPlaneMask | GCFont), &vGC);

}

#define OFFSET 3

#ifdef __STDC__
void RefreshXtty(xtty *tty)
#else
void RefreshXtty(tty)
xtty *tty;
#endif
{
int y;

   for (y = 0; y < TTYHEIGHT; y++)
      XDrawImageString(tty->display,
                       tty->window,
                       tty->gc,
                       0,
                       y * tty->font_height - OFFSET,
                       &tty->video[y][0],
                       TTYWIDTH);

}

#ifdef __STDC__
static void UpdateXtty(xtty *tty, int refresh)
#else
static void UpdateXtty(tty, refresh)
xtty *tty;
int refresh;
#endif
{
int x, y;

   if (tty->xcurse == 0) {
      x = TTYWIDTH - 1;
      y = tty->ycurse - 1;
   } else {
      x = tty->xcurse - 1;
      y = tty->ycurse;
   }
   if (refresh)
      RefreshXtty(tty);
   else
      XDrawImageString(tty->display,
                    tty->window,
                    tty->gc,
                    x * tty->font_width,
                    y * tty->font_height - OFFSET,
                    &tty->video[y][x], 1);
}

#undef OFFSET

#ifdef __STDC__
static int ManageXtty(xtty *tty, char c)
#else
static int ManageXtty(tty, c)
xtty *tty;
char c;
#endif
{
int yy;
int i;

   switch (c) {
      case 0x08:
         if (tty->xcurse > 0) {
            tty->video[tty->ycurse][--tty->xcurse] = ' ';
            return 1;
         } else
            XBell(tty->display, 10);
         break;

      case 0x09:
         for (i=0; i<8; i++) {
            tty->video[tty->ycurse][tty->xcurse] = ' ';
            tty->xcurse++;
            if (tty->xcurse >= TTYWIDTH) {
               tty->xcurse = 0;
               tty->ycurse++;
            }
            if (tty->ycurse >= TTYHEIGHT) {
               for (yy = 1; yy < TTYHEIGHT; yy++)
                  memcpy(tty->video[yy - 1], tty->video[yy], TTYWIDTH);
               memset(tty->video[yy - 1], ' ', TTYWIDTH);
               tty->ycurse = yy - 1;
               return 1;
            }
         }
         break;

      case 0x0A:
      case 0x0D:
         tty->xcurse = 0;
         tty->ycurse++;
         break;

      default:
         tty->video[tty->ycurse][tty->xcurse] = c;
         tty->xcurse++;
   }
   if (tty->xcurse >= TTYWIDTH) {
      tty->xcurse = 0;
      tty->ycurse++;
   }
   if (tty->ycurse >= TTYHEIGHT) {
      for (yy = 1; yy < TTYHEIGHT; yy++)
         memcpy(tty->video[yy - 1], tty->video[yy], TTYWIDTH);
      memset(tty->video[yy - 1], ' ', TTYWIDTH);
      tty->ycurse = yy - 1;
      return 1;
   }
   return 0;
}

#ifdef __STDC__
static void EventXtty(xtty *tty, XEvent *event)
#else
static void EventXtty(tty, event)
xtty *tty;
XEvent *event;
#endif
{
KeySym key;
char c;

   switch (event->type) { 
      case Expose:
         RefreshXtty(tty);
         break;

      case KeyPress:
         XLookupString(&event->xkey, &c, 1, &key, NULL);

         if ((key & 0x100) != 0
             && key != XK_BackSpace
             && key != XK_BackSpace
             && key != XK_Tab
             && key != XK_Linefeed
             && key != XK_Return
             && key != XK_Escape
             && key != XK_Delete)
		 
		break ;
	 
	 if (tty->wptr == tty->rptr && (tty->keybuf[tty->wptr] & 0x100) == 0x100)
            XBell(tty->display, 10);

	 else {
	    write( 1, &c, 1) ; 
    	   // UpdateXtty(tty, ManageXtty(tty, c));
	    tty->keybuf[tty->wptr++] = 0 << 8 | c; //1 << 8 | c;
            tty->wptr %= 16;

         }
         break;

      default :
         return;
   }
}

#ifdef __STDC__
static Display *connect(Colormap *cmap)
#else
static Display *connect(cmap)
Colormap *cmap;
#endif
{
Display *display;

   if ((display = XOpenDisplay(NULL)) == NULL) {
      fputs("tty: cannot open X11 display\n", stderr);
      exit(1);
   }
   *cmap = DefaultColormap(display, DefaultScreen(display));
   return display;
}

#ifdef __STDC__
static void DestroyXtty(xtty *tty)
#else
static void DestroyXtty(tty)
xtty *tty;
#endif
{
   XDestroyWindow(tty->display, tty->window);
   XCloseDisplay(tty->display);
}

int main(int argc, char *argv[])
{
timeval timeout ;
xtty *tty;
char *title;
XSizeHints shint;
//XWMHints wmhint;
XEvent event;
Colormap Cmap;
char c;
int rs;

//fd_set rfds, wfds;
fd_set rfds;
int xfd;

   if (argc == 2)
      title = argv[1];
   else
      title = "SoCLib TTY";

   tty = (xtty *)malloc(sizeof(xtty));
   memset(tty->video, ' ', TTYHEIGHT * TTYWIDTH);
   
   tty->xcurse = 0;
   tty->ycurse = 1;
   memset(tty->keybuf, 0, MAXCHAR * sizeof(short));
   tty->rptr  = 0;
   tty->wptr  = 0;

   tty->display = connect(&Cmap);
   ttySetGraphics(Cmap, FONT, tty);

   shint.x = 100;
   shint.y = 0;
   shint.min_width = shint.max_width = shint.width = TTYWIDTH * tty->font_width;
   shint.min_height = shint.max_height = shint.height
                   = (TTYHEIGHT - 1) * tty->font_height;
   
   shint.flags = PSize | PMinSize | PMaxSize;

   tty->window = XCreateSimpleWindow(
                     tty->display,
                     DefaultRootWindow(tty->display), 
                     shint.x, shint.y, shint.width, shint.height, 5,
                     WhitePixelOfScreen(DefaultScreenOfDisplay(tty->display)),
                     BlackPixelOfScreen(DefaultScreenOfDisplay(tty->display)));

   XSetStandardProperties(tty->display, tty->window,
                          title, "Pouet",
                          None, NULL, 0, &shint);

   /*
	if (XpmCreatePixmapFromData(tty->display, tty->window, afraid_xpm,
					                &pixmap, &mask, NULL))
                               */
   XSelectInput(tty->display, tty->window,// ExposureMask);

                KeyPressMask    | ExposureMask);

   XMapRaised(tty->display, tty->window);

   /*
   for (i=0; i<15; i++)
      memcpy(tty->video[i+4], pouet[i], 65);
      */

   xfd=ConnectionNumber(tty->display);
   FD_ZERO(&rfds);
   FD_SET(xfd, &rfds);
   FD_SET(0, &rfds);

   signal(SIGINT, SIG_IGN);




   
   XFlush(tty->display);
   
   while (1) {

	   XFlush(tty->display);
	
	   FD_ZERO(&rfds);
	   FD_SET(xfd, &rfds);
   	   FD_SET(0, &rfds);

	   timeout.tv_sec = 2 ;
	   timeout.tv_usec = 0 ;
  	   rs = select (xfd+1, &rfds, NULL, NULL, &timeout);
	   
	   if (rs !=0){
		
  		 if ( FD_ISSET(0, &rfds) == 1) {
        		 read(0, &c, 1);
        		 UpdateXtty(tty, ManageXtty(tty, c));
		         RefreshXtty(tty);
			}


		 if (  FD_ISSET(xfd, &rfds) == 1)  {
  	  		 XNextEvent(tty->display, &event);
			 EventXtty (tty, &event) ;
		         RefreshXtty(tty);
			}
		 
	   	}

   }


//getchar();












/*


   while (1) {
   FD_SET(xfd, &rfds);
   FD_SET(0, &rfds);
      XFlush(tty->display);
      rs=select(xfd+1, &rfds, NULL, NULL, NULL);
      if (FD_ISSET(0, &rfds)) {
         memset(tty->video, ' ', TTYHEIGHT * TTYWIDTH);
         RefreshXtty(tty);
         read(0, &c, 1);
         UpdateXtty(tty, ManageXtty(tty, c));
         break;
      } else {
         while (XCheckWindowEvent(tty->display, tty->window,
               ExposureMask, &event))
            RefreshXtty(tty);
      }
   }


   
   
   while (1) {
   FD_SET(xfd, &rfds);
   FD_SET(0, &rfds);
      XFlush(tty->display);
      rs=select(xfd+1, &rfds, NULL, NULL, NULL);
      if (FD_ISSET(0, &rfds)) {
         read(0, &c, 1);
         UpdateXtty(tty, ManageXtty(tty, c));
      } else {
         while (XCheckWindowEvent(tty->display, tty->window,
               ExposureMask, &event))
            RefreshXtty(tty);
      }
   }


*/

   
   exit(0);
}

