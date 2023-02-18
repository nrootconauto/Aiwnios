U0 QueInit(CQue *head) {
  head->next=head;
  head->last=head;
}
U0 QueIns(CQue *entry,CQue *pred) 
{ //Insert item into que after predecessor.
  CQue *n=pred->next;
  entry->last=pred;
  entry->next=n;
  pred->next=entry;
  n->last=entry;
}
U0 QueInsRev(CQue *entry,CQue *succ) 
{
  //Revd insert into que.
  QueIns(entry,succ->last);
}
U0 QueRem(CQue *entry) {
  CQue *n=entry->next,*l=entry->last;
  l->next=n;
  n->last=l;
}
