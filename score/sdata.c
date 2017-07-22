#include "seq:include.h"

#ifdef SCORE

#define UW static UWORD
#define RD __chip
#define MAXS 158

#include <exec/types.h>         /* Data types.             */

#include "prog:son/a1.c"
#include "prog:son/a2.c"
#include "prog:son/a3.c"
#include "prog:son/a4.c"
#include "prog:son/a5.c"
#include "prog:son/a6.c"
#include "prog:son/a7.c"
#include "prog:son/a8.c"
#include "prog:son/a9.c"
#include "prog:son/a10.c"
#include "prog:son/a11.c"
#include "prog:son/a13.c"
#include "prog:son/a14.c"
#include "prog:son/a15.c"
#include "prog:son/a16.c"
#include "prog:son/a17.c"
#include "prog:son/a18.c"
#include "prog:son/a19.c"
#include "prog:son/a20.c"
#include "prog:son/a22.c"
#include "prog:son/a23.c"
#include "prog:son/a24.c"
#include "prog:son/a25.c"
#include "prog:son/a26.c"
#include "prog:son/a27.c"
#include "prog:son/a28.c"
#include "prog:son/a29.c"
#include "prog:son/a31.c"
#include "prog:son/a32.c"
#include "prog:son/a33.c"
#include "prog:son/a34.c"
#include "prog:son/a35.c"
#include "prog:son/a36.c"
#include "prog:son/a37.c"
#include "prog:son/a38.c"
#include "prog:son/a40.c"
#include "prog:son/a41.c"
#include "prog:son/a42.c"
#include "prog:son/a43.c"
#include "prog:son/a44.c"
#include "prog:son/a45.c"
#include "prog:son/a46.c"
#include "prog:son/a47.c"
#include "prog:son/a48.c"
#include "prog:son/a49.c"
#include "prog:son/a50.c"
#include "prog:son/a51.c"
#include "prog:son/a52.c"
#include "prog:son/a53.c"
#include "prog:son/a54.c"
#include "prog:son/a55.c"
#include "prog:son/a56.c"
#include "prog:son/a57.c"
#include "prog:son/a58.c"
#include "prog:son/a59.c"
#include "prog:son/a60.c"
#include "prog:son/a61.c"
#include "prog:son/a64.c"
#include "prog:son/a65.c"
#include "prog:son/a66.c"
#include "prog:son/a68.c"
#include "prog:son/a69.c"
#include "prog:son/a70.c"
#include "prog:son/a71.c"
#include "prog:son/a72.c"
#include "prog:son/a73.c"
#include "prog:son/a74.c"
#include "prog:son/a75.c"
#include "prog:son/a76.c"
#include "prog:son/a77.c"
#include "prog:son/a78.c"
#include "prog:son/a79.c"
#include "prog:son/a80.c"
#include "prog:son/a81.c"
#include "prog:son/a82.c"
#include "prog:son/a83.c"
#include "prog:son/a84.c"
#include "prog:son/a85.c"
#include "prog:son/a86.c"
#include "prog:son/a87.c"
#include "prog:son/a88.c"
#include "prog:son/a89.c"
#include "prog:son/a90.c"
#include "prog:son/a91.c"
#include "prog:son/a92.c"
#include "prog:son/a93.c"
#include "prog:son/a94.c"
#include "prog:son/a95.c"
#include "prog:son/a96.c"
#include "prog:son/a97.c"
#include "prog:son/a98.c"
#include "prog:son/a99.c"
#include "prog:son/a100.c"
#include "prog:son/a101.c"
#include "prog:son/a102.c"
#include "prog:son/a103.c"
#include "prog:son/a104.c"
#include "prog:son/a105.c"
#include "prog:son/a106.c"
#include "prog:son/a107.c"
#include "prog:son/a108.c"
#include "prog:son/a109.c"
#include "prog:son/a111.c"
#include "prog:son/a112.c"
#include "prog:son/a113.c"
#include "prog:son/a114.c"
#include "prog:son/a115.c"
#include "prog:son/a116.c"
#include "prog:son/a117.c"
#include "prog:son/a118.c"
#include "prog:son/a119.c"
#include "prog:son/a121.c"
#include "prog:son/a122.c"
#include "prog:son/a123.c"
#include "prog:son/a125.c"
#include "prog:son/a126.c"
#include "prog:son/a127.c"
#include "prog:son/a128.c"
#include "prog:son/a129.c"
#include "prog:son/a130.c"
#include "prog:son/a131.c"
#include "prog:son/a132.c"
#include "prog:son/a134.c"
#include "prog:son/a135.c"
#include "prog:son/a137.c"
#include "prog:son/a138.c"
#include "prog:son/a139.c"
#include "prog:son/a141.c"
#include "prog:son/a142.c"
#include "prog:son/a143.c"
#include "prog:son/a144.c"
#include "prog:son/a145.c"
#include "prog:son/a146.c"
#include "prog:son/a147.c"
#include "prog:son/a148.c"
#include "prog:son/a149.c"
#include "prog:son/a150.c"
#include "prog:son/a151.c"
#include "prog:son/a152.c"
#include "prog:son/a153.c"
#include "prog:son/a154.c"
#include "prog:son/a155.c"
#include "prog:son/a156.c"
#include "prog:son/a157.c"
#include "prog:son/a158.c"

far struct SCOREIMAGE scoreimages[MAXS]=
{
a1,48 ,78 ,
a2,48 ,39 ,
a3,32 ,24 ,
a4,32, 24,
a5,16 ,36 ,
a6,32, 79,
a7,32, 79,
a8,64, 63,
a9,48 ,59 ,
a10,64 ,63 ,
a11,48 , 63,
a13,48 , 59,
a14, 48, 59,
a15, 64, 63,
a16, 64, 62,
a17, 48, 63,
a18, 48, 64,
a19, 80, 41,
a20, 48, 39,
a22, 48, 38,
a23, 80, 195,
a24, 32, 54,
a25, 80, 74,
a26, 48, 108,
a27, 80, 191,
a28, 80,98,
a29, 96, 249,
a31, 64, 51,
a32, 48, 43,
a33, 32, 43,
a34, 48, 5,
a35, 16, 14,
a36, 32, 72,
a37, 32, 71,
a38, 48, 72,
a40, 64, 37,
a41, 112, 115,
a42, 32, 53,
a43, 48, 127,
a44, 112, 146,
a45, 64, 122,
a46, 128, 53,
a47, 48, 142,
a48, 128, 84,
a49, 32, 13,
a50, 48, 144,
a51, 32, 54,
a52, 48, 109,
a53, 48, 112,
a54, 48, 60,
a55, 48, 36,
a56, 48, 179,
a57, 64, 25,
a58, 48, 39,
a59, 48, 147,
a60, 32, 143,
a61, 128, 66,
a64, 16, 36,
a65, 80, 216,
a66, 48, 39,
a68, 48, 287,
a69, 48, 151,
a70, 48, 37,
a71, 48, 45,
a72, 64, 63,
a73, 48, 45,
a74, 48, 217,
a75, 48, 43,
a76, 48, 43,
a77, 48, 151,
a78, 80, 163,
a79, 48, 51,
a80, 80, 144,
a81, 80, 54,
a82, 80, 142,
a83, 112, 84,
a84, 48, 101,
a85, 80, 53,
a86, 96, 51,
a87, 64, 65,
a88, 96, 52,
a89, 64, 96,
a90, 48, 146,
a91, 96, 73,
a92, 48, 123,
a93, 32, 17,
a94, 48, 60,
a95, 32, 57,
a96, 48, 160,
a97, 80, 142,
a98, 48, 128,
a99, 48, 45,
a100, 64, 38,
a101, 32, 71,
a102, 32, 24,
a103, 48, 46,
a104, 48, 29,
a105, 80, 72,
a106, 64, 71,
a107, 80, 174,
a108, 48, 37,
a109, 80, 129,
a111, 48, 43,
a112, 48, 142,
a113, 64, 73,
a114, 48, 43,
a115, 80, 167,
a116, 32, 36,
a117, 48, 67,
a118, 80, 209,
a119, 48, 165,
a121, 32, 71,
a122, 16, 54,
a123, 48, 128,
a125, 48, 44,
a126, 48, 76,
a127, 48, 18,
a128, 48, 192,
a129, 96, 39,
a130, 48, 108,
a131, 112, 52,
a132, 96, 51,
a134, 96, 47,
a135, 48, 144,
a137, 48, 43,
a138, 32, 146,
a139, 80, 146,
a141, 48, 60,
a142, 80, 5,
a143, 48, 127,
a144, 80, 55,
a145, 32, 96,
a146, 64, 73,
a147, 112, 52,
a148, 80, 84,
a149, 80, 142,
a150, 32, 77,
a151, 16, 146,
a152, 48, 144,
a153, 32, 43,
a154, 48, 117,
a155, 48, 108,
a156, 96, 36,
a157, 32, 101,
a158, 64,133
};

struct BitMap far sbits[MAXS];         /* Ausgangsbitmap */
struct BitMap far tbits[MAXS];         /* Zoombitmap */

void InitScoreBitMaps()
{
UBYTE i;

for(i=0;i<MAXS;i++)   /* Ausgangsbitmap setzen */
{
InitBitMap(&sbits[i],1,scoreimages[i].br,scoreimages[i].hh);
sbits[i].Planes[0]=(PLANEPTR)scoreimages[i].mem;
}

for(i=0;i<MAXS;i++)   /* Zoom setzen */
{
InitBitMap(&tbits[i],1,scoreimages[i].br/2,(scoreimages[i].hh/2));
tbits[i].Planes[0]=(PLANEPTR)AllocMem(tbits[i].BytesPerRow*(scoreimages[i].hh/2),MEMF_CHIP);
}

}

void FreeZoomBitMaps()
{
UBYTE i;
for(i=0;i<MAXS;i++)   /* Zoom setzen */
{
FreeMem(tbits[i].Planes[0],tbits[i].BytesPerRow*(scoreimages[i].hh/2));
}

}

#endif

