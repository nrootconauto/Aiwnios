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
  U64 font=text.font[ch&0xff],color=gr.to_8_colors[(ch>>8)&0xff];
  U64 m;
  U64 *to_8_bits=gr.to_8_bits;
  if(ch&(1<<ATTRf_UNDERLINE)) {
    font|=0xFF00000000000000;
  }
  m=to_8_bits[font&255];
  font>>=8;
  dst(U64*)[0]=dst(U64*)[0]&~m|color&m;
  dst+=width_internal;
  m=to_8_bits[font&255];
  font>>=8;
  dst(U64*)[0]=dst(U64*)[0]&~m|color&m;
  dst+=width_internal;
  m=to_8_bits[font&255];
  font>>=8;
  dst(U64*)[0]=dst(U64*)[0]&~m|color&m;
  dst+=width_internal;
  m=to_8_bits[font&255];
  font>>=8;
  dst(U64*)[0]=dst(U64*)[0]&~m|color&m;
  dst+=width_internal;
  m=to_8_bits[font&255];
  font>>=8;
  dst(U64*)[0]=dst(U64*)[0]&~m|color&m;
  dst+=width_internal;
  m=to_8_bits[font&255];
  font>>=8;
  dst(U64*)[0]=dst(U64*)[0]&~m|color&m;
  dst+=width_internal;
  m=to_8_bits[font&255];
  font>>=8;
  dst(U64*)[0]=dst(U64*)[0]&~m|color&m;
  dst+=width_internal;
  m=to_8_bits[font&255];
  font>>=8;
  dst(U64*)[0]=dst(U64*)[0]&~m|color&m;
  dst+=width_internal;
}
