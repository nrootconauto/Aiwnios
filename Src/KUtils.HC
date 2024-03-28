I64 CallExtStr(U8 *str,...) {
  CHashFun *fun=HashFind(str,Fs->hash_table,HTT_FUN);
  if(!fun) return INVALID_PTR;
  return CallArgs(fun->exe_addr,argc,argv);
}
