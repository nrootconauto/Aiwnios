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
