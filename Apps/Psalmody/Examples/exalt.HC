//5 has graphics

F64 t0=Beat;

class Obj
{
  F64 x,y,dx,dy;
};

U0 ObjAnimate(Obj *tmpo,F64 dt)
{
  tmpo->x+=dt*tmpo->dx;
  tmpo->y+=dt*tmpo->dy;
  tmpo->dy+=2*dt;
}

#define SPARKS_NUM	128
class FireWork
{
  FireWork *next,*last;
  F64 t0,t_burst,t_death;
  CColorROPU32	color,pad;
  Obj rocket,sparks[SPARKS_NUM]
} fireworks;

CColorROPU32 colors[5]={BLUE,RED,PURPLE,GREEN,YELLOW};

U0 FireWorkNew(F64 t)
{
  F64 é,d;
  FireWork *res=CAlloc(sizeof(FireWork));
  res->t0=t;
  res->t_burst=res->t0+4;
  res->t_death=res->t_burst+4;
  res->color=colors[Rand*5];
  res->rocket.x=344;
  res->rocket.y=70;
  é=0.5*(Rand-0.5);
  d=2*Rand+17.0;
  res->rocket.dx=d*Sin(é);
  res->rocket.dy=-d*Cos(é);
  QueIns(res,fireworks.last);
}

U0 FireWorkDel(FireWork *tmpf)
{
  QueRem(tmpf);
  Free(tmpf);
}

U0 FireWorkBurst(FireWork *tmpf)
{
  I64 i;
  F64 é,d;
  for (i=0,é=0;i<SPARKS_NUM;i++,é+=2*ã/SPARKS_NUM) {
    tmpf->sparks[i].x=tmpf->rocket.x;
    tmpf->sparks[i].y=tmpf->rocket.y;
    d=Rand+1.5;
    d=d*d/4;
    tmpf->sparks[i].dx=4*Cos(é)*d;
    tmpf->sparks[i].dy=4*Sin(é)*d;
  }
}

U0 FireWorkAnimate(FireWork *tmpf,F64 t,F64 dt)
{
  I64 i;
  if (t<tmpf->t_burst)
    ObjAnimate(&tmpf->rocket,dt);
  else if (t-dt<=tmpf->t_burst) {
    ObjAnimate(&tmpf->rocket,dt);
    FireWorkBurst(tmpf);
  } else if (t<tmpf->t_death)
    for (i=0;i<SPARKS_NUM;i++)
      ObjAnimate(&tmpf->sparks[i],dt);
  else
    FireWorkDel(tmpf);
}

U0 FireWorkDraw(CDC *dc,F64 t,FireWork *tmpf)
{
  I64 i;
  if (t<tmpf->t_burst) {
    dc->color=YELLOW;
    GrPlot(dc,tmpf->rocket.x,tmpf->rocket.y);
  } else {
    dc->color=tmpf->color;
    for (i=0;i<SPARKS_NUM;i++)
      GrPlot(dc,tmpf->sparks[i].x,tmpf->sparks[i].y);
  }
}

U0 DrawIt(CTask *,CDC *dc)
{
  static F64 last_t=Beat-t0;
  F64 t=Beat-t0,d;
  FireWork *tmpf;
  for (d=20;d<32;d+=1)
    if (t>d && last_t<=d)
      FireWorkNew(t);
  tmpf=fireworks.next;
  while (tmpf!=&fireworks) {
    FireWorkDraw(dc,t,tmpf);
    FireWorkAnimate(tmpf,t,t-last_t);
    tmpf=tmpf->next;
  }
  last_t=t;
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=BLACK<<4+WHITE;
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;

  MusicSettingsRst;
  music.tempo=2.5;
  try {
    while (!ScanKey) {
      t0=Beat;
      QueInit(&fireworks);
      DocClear;
      "$$BG,BLACK$$$$WHITE$$";
      Play("5eED4sBABAqG5etGECFD4B5ECFC4BReB5E",
	    "Come\0and\0sing\0\0a\0\0long.\0\0\0"
	    "\0\0\0\0\0\0\0\0\n\0\0\0\0");
      Play("5ED4sBABAqG5etGECFD4B5ECFC4BReB5E",
	    "Of\0fer\0praise\0\0to\0\0God.\0\0\0"
	    "\0\0\0\0\0\0\0\0\n\0\0\0\0");
      Play("4qB5eDF4qG5eF4RqB5etFFG4eA5CDF",
	    "Up\0in\0His\0hea\0ven\0\0He\0is\0\0"
	    "\0glor\0i\0ous.\n\0\0");
      Play("4qB5eDF4qG5eF4RqB5etFFG4eA5CDF",
	    "Break\0in\0to\0song.\0\0\0Strike-\0up\0\0\0"
	    "the\0chor\0us.\n\0\0");
      QueDel(&fireworks);
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
