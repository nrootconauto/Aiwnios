//Mirrors KHashA from TempleOS
I64 HashStr(U8 *str)
{
	I64 res = 5381;
	while (*str)
		res += (res << 5) + res + *str++;
	return res;
}

CHash *HashFind(U8 *needle,CHashTable *table,I64 mask,I64 inst=1)
{
  I64 idx=HashStr(needle)&table->mask;
  CHash *ret;
  for(ret=table->body[idx];ret;ret=ret->next) {
    if(ret->type&mask)
      if(!StrCmp(ret->str,needle)) {
        if(!--inst) {
          ret->use_cnt++;
          goto pass;
        }
      }
  }
  if(table->next)
    ret=HashFind(needle,table->next,mask,inst);
  else
    ret=NULL;
pass:
  return ret;
}

CHash *HashSingleTableFind(U8 *needle,CHashTable *table,I64 mask,I64 inst=1)
{
  I64 idx=HashStr(needle)&table->mask;
  CHash *ret;
  for(ret=table->body[idx];ret;ret=ret->next) {
    if(ret->type&mask)
      if(!StrCmp(ret->str,needle))
        if(!--inst) {
          ret->use_cnt++;
          goto pass;
        }
  }
  ret=NULL;
pass:
  return ret;
}

CHash **HashBucketFind(U8 *str,CHashTable *table)
{
  I64 idx=HashStr(str)&table->mask;
  return &table->body[idx];
}
U0 HashAdd(CHash *h,CHashTable *table)
{
  I64 idx=HashStr(h->str)&table->mask;
  h->next=table->body[idx];
  table->body[idx]=h;
}
extern U0 HashDel(CHashSrcSym *tmph);
Bool HashRemDel(CHash *tmph,CHashTable *table,
	I64 instance=1) {
  I64 b=HashStr(tmph->str)&table->mask;
  CHash *p=NULL,*c;
  for(c=table->body[b];c;c=c->next) {
    if(c==tmph) {
      if(p) p->next=c->next;
      else table->body[b]=c->next;
      c->next=NULL;
      HashDel(c);
      return TRUE;
    }
    p=c;
  }
  return FALSE;
}
U0 HashAddAfter(CHash *tmph,CHash *to,CHashTable *) {
  CHash *next=to->next;
  to->next=tmph;
  tmph->next=next;
}
