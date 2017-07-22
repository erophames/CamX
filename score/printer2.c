#include "include.h"
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

#include <exec/types.h>       /* Data types.             */
#include <exec/errors.h>      /* Exec error messages.    */
#include <devices/printer.h>  /* Printer Device.         */
#include <exec/io.h>          /* Standard request block. */

/* PrintRastPort() helps you with printing graphics. It takes a    */
/* pointer to a RastPort, and dumps that RastPort to the printer.  */
/* Note that some printers does not support graphics. If the user  */
/* has a printer that can not handle graphics, this function will  */
/* return immediately with the error number "PDERR_NOTGRAPHICS".   */
/*                                                                 */
/* Synopsis: error = PrintRastPort( io, rp, cm, modes, sx, sy,     */
/*                                  sw, sh, dw, dh, special );     */
/*                                                                 */
/* error:   (BYTE) PrintRastPort() returns 0 if everything was OK, */
/*          else an error number is returned.                      */
/*                                                                 */
/* io:      (union printerIO *) Pointer to a printer request       */
/*          block.                                                 */
/*                                                                 */
/* rp:      (struct RastPort *) Pointer to the RastPort which      */
/*          should be printed.                                     */
/*                                                                 */
/* cm:      (struct ColorMap *) Pointer to a ColorMap structure    */
/*          which contains the colour information.                 */
/*                                                                 */
/* modes:   (ULONG) The ViewPort's display modes.The information   */
/*          is used to convert the picture which will be printed   */
/*          to the correct aspects. (On a low resolution screen    */
/*          each pixels is equally wide as tall. However, on a     */
/*          high resolution screen, each pixel is only half as     */
/*          wide as it is tall. The same applies for interlaced    */
/*          and non interlaced screens.) The printer device must   */
/*          also know if you want to print a "normal" picture, or  */
/*          a picture with one of the special display modes like   */
/*          "HAM" or "Extrahalf Brite". The following flags may    */
/*          be used:                                               */
/*                                                                 */
/*            HIRES:  Set this flag if you want to print a high    */
/*                    resolution screen. If this flag is not set,  */
/*                    the printer device assumes that you are      */
/*                    using a low resolution screen.               */
/*                                                                 */
/*            LACE:   Set this flag if you want to print an inter- */
/*                    laced picture. If this flag is not set, the  */
/*                    printer device assumes that you are using a  */
/*                    noninterlaced picture.                       */
/*                                                                 */
/*            HAM:    Set this flag if you want to print a "HAM"   */
/*                    picture.                                     */
/*                                                                 */
/*            EXTRA_HALFBRITE: Set this flag if you want to print  */
/*                    an "extra halfbrite" picture.                */
/*                                                                 */
/*            PUALPF: Set this flag if you want to print a dual    */
/*                    playfields screen.                           */
/*                                                                 */
/*          Note that the simplest way is to copy the Viewport     */
/*          structure's "modes" field. You will then not risk to   */
/*          forget one or more display flags.                      */
/*                                                                 */
/* sx:      (UWORD) X offset of the source picture.                */
/*                                                                 */
/* sy:      (UWORD) Y offset of the source picture.                */
/*                                                                 */
/* sw:      (UWORD) Width of the source picture.                   */
/*                                                                 */
/* sh:      (UWORD) Height of the source picture.                  */
/*                                                                 */
/* dw:      (LONG) Width of the printed picture.                   */
/*                                                                 */
/* dh:      (LONG) Height of the printed picture.                  */
/*                                                                 */
/* special: (UWORD) Special graphical printing modes. Here is the  */
/*          complete list of flags that may be used:               */
/*                                                                 */
/*            SPECIAL_MILCOLS:    If this flag is set the "dw"     */
/*                                parameter is in 1/1000".         */
/*                                                                 */
/*            SPECIAL_MILROWS:    If this flag is set the "dh"     */
/*                                parameter is in 1/1000".         */
/*                                                                 */
/*            SPECIAL_FULLCOLS:   Set this flag if you want the    */
/*                                width of the printed picture to  */
/*                                be as wide as possible.          */
/*                                                                 */
/*            SPECIAL_FULLROWS:   Set this flag if you want the    */
/*                                height of the printed picture to */
/*                                be as tall as possible.          */
/*                                                                 */
/*            SPECIAL_FRACCOLS:   If this flag is set the "dw"     */
/*                                parameter specifies a fraction   */
/*                                of the maximum width.            */
/*                                                                 */
/*            SPECIAL_FRACROWS:   If this flag is set the "dh"     */
/*                                parameter specifies a fraction   */
/*                                of the maximum height.           */
/*                                                                 */
/*            SPECIAL_CENTER:     Set this flag if you want the    */
/*                                picture to be centered on the    */
/*                                paper.                           */
/*                                                                 */
/*            SPECIAL_ASPECT:     Set this flag if you want to use */
/*                                the correct aspect ratio of the  */
/*                                picture.                         */
/*                                                                 */
/*            SPECIAL_DENSITY1:   Set this flag if you want the    */
/*                                picture to be printed with the   */
/*                                printer's lowest resolution.     */
/*                                Lowest resolution.               */
/*                                                                 */
/*            SPECIAL_DENSITY2:   Next resolution.                 */
/*                                                                 */
/*            SPECIAL_DENSITY3:   Next resolution.                 */
/*                                                                 */
/*            SPECIAL_DENSITY4:   Next resolution.                 */
/*                                                                 */
/*            SPECIAL_DENSITY5:   Next resolution.                 */
/*                                                                 */
/*            SPECIAL_DENSITY6:   Next resolution.                 */
/*                                                                 */
/*            SPECIAL_DENSITY7:   Use the printer's highest        */
/*                                resolution.                      */
/*                                                                 */
/*            SPECIAL_NOFORMFEED: Set this flag if you do not want */
/*                                that the paper is ejected after  */
/*                                each time you have printed       */
/*                                graphics.                        */
/*                                                                 */
/*            SPECIAL_TRUSTME:    Set this flag if you do not want */
/*                                the printer to reset any param-  */
/*                                eters while printing.            */

/* Declare how the printer request block look like: */
union printerIO
{
  struct IOStdReq ios;
  struct IODRPReq iodrp;
  struct IOPrtCmdReq iopc;
};

/* Declare a pointer to our reply port: */
struct MsgPort *replymp = NULL;

/* Declare a pointer our printer request block: */
union printerIO *printer_req = NULL;

/* Store the printer device error here: */
UWORD printer_dever = TRUE;

/* Dumps a RastPort to the printer: */
BYTE PrintRastPort(
  union printerIO *ioreq,
  struct RastPort *rp,
  struct ColorMap *cm,
  ULONG modes,
  UWORD source_x,
  UWORD source_y,
  UWORD source_w,
  UWORD source_h,
  LONG dest_w,
  LONG dest_h,
  UWORD special
);

void CleanPrinter()
{
  if( !printer_dever )
    CloseDevice( printer_req );

  if( printer_req )
    DeleteExtIO( printer_req, sizeof(union printerIO) );

  if( replymp )
    DeletePort( replymp);
}

/* PrtError() tells the user what went wrong. You give it the error code */
/* you received, and PrtError() will print a short description of the    */
/* problem. Useful when debugging. (Printer errors)                      */
/*                                                                       */
/* Synopsis: PrtError( error );                                          */
/*                                                                       */
/* error:    (BYTE) The error value you want to have explained.          */

void PrtError( BYTE error )
{
  switch( error )
  {
    /* EXEC error messages: (defined in "exec/errors.h") */
    case IOERR_OPENFAIL:
      printf( "Could not open the device!\n" );
      break;

    case IOERR_ABORTED:
      printf( "The request was aborted!\n" );
      break;

    case IOERR_NOCMD:
      printf( "Unknown Command!\n" );
      break;

    case IOERR_BADLENGTH:
      printf( "Bad length of the command - data!\n" );


    /* Printer Device errors: (defined in "devices/printer.h") */
    case PDERR_CANCEL:
      printf( "User cancelled the request!\n" );
      break;

    case PDERR_NOTGRAPHICS:
      printf( "The printer does not support graphics!\n" );
      break;

    case PDERR_BADDIMENSION:
      printf( "The printer dimension is not valid!\n" );
      break;


    case PDERR_INTERNALMEMORY:
      printf( "Not enough memory for the internal printer functions!\n" );
      break;

    case PDERR_BUFFERMEMORY:
      printf( "Not enough memory for the print buffer!\n" );
      break;

    default:
      printf( "An unknown error was reported! Error nr: %d\n", error );
  }
}

BYTE PrintRastPort(
  union printerIO *ioreq,
  struct RastPort *rp,
  struct ColorMap *cm,
  ULONG modes,
  UWORD source_x,
  UWORD source_y,
  UWORD source_w,
  UWORD source_h,
  LONG dest_w,
  LONG dest_h,
  UWORD special
)
{
  /* We want to dump a RastPort to the printer: */
  ioreq->iodrp.io_Command = PRD_DUMPRPORT;

  /* Set a pointer to the RastPort structure: */
  ioreq->iodrp.io_RastPort = rp;

  /* Set a pointer to the ColorMap structure: */
  ioreq->iodrp.io_ColorMap = cm;

  /* Set the "display" modes: */
  ioreq->iodrp.io_Modes = modes;

  /* X position of the source: */
  ioreq->iodrp.io_SrcX = source_x;

  /* Y position of the source: */
  ioreq->iodrp.io_SrcY = source_y;

  /* Width of the source: */
  ioreq->iodrp.io_SrcWidth = source_w;

  /* Height of the source: */
  ioreq->iodrp.io_SrcHeight = source_h;

  /* The width of the printed picture: */
  ioreq->iodrp.io_DestCols = dest_w;

  /* The height of the printed picture: */
  ioreq->iodrp.io_DestRows = dest_h;

  /* Set the special printing commands: */
  ioreq->iodrp.io_Special = special;


  /* Do our request, and return 0 if everything is OK, else */
  /* return an error number: (This is a task sleep.)        */
  return( (BYTE) DoIO( ioreq ) );
}

void Printer
(
  struct RastPort *rp,
  struct ColorMap *cm,
  ULONG modes,
  UWORD source_x,
  UWORD source_y,
  UWORD source_w,
  UWORD source_h,
  LONG dest_w,
  LONG dest_h,
  UWORD special
)
{
  /* Get a reply port: (No name, priority 0) */
  replymp = (struct MsgPort *)
    CreatePort( NULL, 0 );

    /*
  if( !replymp )
    clean_up( 0, "Could not create the reply port!" );
    */

  /* Create the printer request block: */
  printer_req = (union printerIO *)
    CreateExtIO( replymp, sizeof(union printerIO) );

    /*
  if( !printer_req )
    clean_up( 0, "Not enough memory for the printer request block!" );
    */

  /* Open the Printer Device: */
  printer_dever = OpenDevice( "printer.device", 0, printer_req, 0 );

  /*
  if( printer_dever )
    clean_up( 0, "Could not open the Printer Device!" );
  */

  /* Dump a RastPort to the printer: */
  error = PrintRastPort
  (
    printer_req,      /* Pointer to the printer request block. */
    rp,        /* Pointer to the RastPort.              */
    cm,       /* Pointer to the ColorMap structure.    */
    modes,            /* Special display modes.                */
    source_x,                /* Start at X position 0.                */
    source_y,                /* Start at Y position 0.                */
    source_w,            /* The width of the display.             */
    source_h,           /* The height of the display.            */
    0,                /* The width of the printout.            */
    0,                /* The height of the printout.           */
    /* Since we set the sice below with help of the special    */
    /* printing modes, we do not need to set any width or      */
    /* height of the printout.                                 */
    SPECIAL_FULLCOLS| /* Special printing modes. Full width,   */
    SPECIAL_ASPECT    /* and correct aspect ratio.             */
  );
  if( error )
    PrtError( error );

  /* Test 2:                          */
  /* Use our own (very strange) size. */
  buffer[5]='2';

  /* Send some text to the printer: (Will be translated) */
  error = PrintText( printer_req, buffer, 7 );
  if( error )
    PrtError( error );

  /* Dump a RastPort to the printer: */
  error = PrintRastPort
  (
    printer_req,      /* Pointer to the printer request block. */
    rast_port,        /* Pointer to the RastPort.              */
    colour_map,       /* Pointer to the ColorMap structure.    */
    modes,            /* Special display modes.                */
    0,                /* Start at X position 0.                */
    0,                /* Start at Y position 0.                */
    width,            /* The width of the display.             */
    height,           /* The height of the display.            */
    200,              /* The width of the printout.            */
    600,              /* The height of the printout.           */
    SPECIAL_CENTER    /* Special printing modes. Center the    */
                      /* picture on the paper.                 */
  );
  if( error )
    PrtError( error );

  /* Clean up and quit: */
  CleanPrinter();
}


