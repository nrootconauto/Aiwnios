#help_index "Graphics/Misc"
Bool IsPixCovered0(CTask *task,I64 x,I64 y) {//No clipping
  x>>=3;
  y>>=3;
  y*=80;//TEXT_COLS; Changed by nroot TEXT_COLS refers to text.cols,so i hardcoded it
  if(task->win_z_num>=gr.win_z_buf[y+x])
    return FALSE;
  else
    return TRUE;
}
U0 GrRopEquU8NoClipping(I64 ch,U8 *dst,I64 width_internal) {
  U64 font=text.font[ch&0xff],color=gr.to_8_colors[(ch>>8)&0xff],cnt;
  U64 m,t;
  if(ch&(1<<ATTRf_UNDERLINE)) {
    font|=0xFF00000000000000;
  }
  for(cnt=0;cnt!=8;cnt++) {
    m=gr.to_8_bits[font&255];
    font>>=8;
    t=dst(U64*)[0];
    t=t&~m|color&m;
    dst(U64*)[0]=t;
    dst+=width_internal;
  }
}
