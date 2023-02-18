#include "GrInitA.HC"
#include "Gr.HH"
#include "GrExt.HC"
#include "GrGlbls.HC"
#include "GrTextBase.HC"
#include "GrAsm.HC"
#include "GrPalette.HC"
#include "GrDC.HC"
#include "GrInitB.HC"
#include "GrMath.HC"
#include "GrScrn.HC"
#include "GrBitMap.HC"
#include "GrPrimatives.HC"
#include "GrComposites.HC"
/*
#include "ScrnCast.HC"

#define SPE_ABORT	(-1)
#define SPE_EXIT	0
#define SPE_CONT	1
#include "SpriteNew.HC"
#include "GrSpritePlot.HC"
#include "SpriteMesh.HC"
#include "SpriteBitMap.HC"
#include "SpriteCode.HC"
#include "SpriteSideBar.HC"
#include "SpriteEd.HC"
#include "SpriteMain.HC"
#include "GrEnd.HC"
*/

DrawWindowNew;
PaletteSetStd;
gr.dc->color=RED;

U0 Poodacle() {
  I64 x=0,y=0;
  while(TRUE) {
    TextPrint(Fs,1,1,RED<<4|GREEN,"HEllo World\n");
    DCFill;
    x+=10;
    y+=10;
    x%=640;
    y%=480;
    GrRect(,x,y,100,100);
    GrUpdateScrn;
    Sleep(30);
  }
}
Poodacle;
