Bool Option(I64 num,Bool val)
{//Set compiler $LK,"Option",A="FI:::/Doc/Options.DD"$ to val.
  if(val)
    return LBts(&(Fs->last_cc->opts),num);
  else
    return LBtr(&(Fs->last_cc->opts),num);
}

Bool GetOption(I64 num)
{//Get state of compiler $LK,"option",A="MN:OPTf_ECHO"$.
  return Bt(&(Fs->last_cc->opts),num);
}

U8 *CmdLinePmt()
{
  I64 i;
  U8 *res,*st;
  if (Fs->new_answer) {
    if (Fs->answer_type&~1!=RT_I0) {
      if (Fs->answer_type==RT_F64)
	"%8.6fs ansf=%15.7g\n",Fs->answer_time,Fs->answer;
      else
	"%8.6fs ans=0x%08X=%d\n",Fs->answer_time,Fs->answer,Fs->answer;
    } else {
      "%8.6fs\n",Fs->answer_time;
      Fs->answer=0;
    }
    Fs->new_answer=FALSE;
  }
  if (st=DirCur) {
    "%s",st;
    Free(st);
  }
  ">";

  LBts(&(Fs->task_flags),TASKf_CMD_LINE_PMT);
  st=GetStr(,,GSF_SHIFT_ESC_EXIT);
  LBtr(&(Fs->task_flags),TASKf_CMD_LINE_PMT);

  i=StrLen(st);
  res=MAlloc(i+1+2);
  MemCpy(res,st,i+1);
  i--;
  while (i>=0 && Bt(char_bmp_white_space,res[i]))
    i--;
  i++;
  if (i>0 && res[i-1]==';')
    res[i++]=';'; //The Lex goes one beyond
  res[i++]='\n';//#define goes to '\n'
  res[i]=0;

  Free(st);
  return res;
}
