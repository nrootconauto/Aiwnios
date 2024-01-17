//Compiler initializes glbls in AOT bin modules to zero.
//$LK,"Globals Set to Zero Here",A="FF:::/Compiler/PrsStmt.HC,Init AOT glbl to zero"$.

CJob sys_macro_head;
CTask *sys_macro_task;
CDoc *sys_clip_doc;

CTask *adam_task,*sys_focus_task;
I64 sys_num_spawned_tasks;

CTask *sys_winmgr_task,*sys_task_being_scrn_updated;
U8 *rev_bits_table, //Table with U8 bits revd
   *set_bits_table; //Table with count of set bits in a U8
CDate local_time_offset;
F64	*pow10_I64,
	sys_os_version=5.030;

CAutoCompleteDictGlbls acd;
CAutoCompleteGlbls ac;
F64 target_fps=30.;
CBlkDevGlbls	blkdev;
CCntsGlbls	cnts={1,0,2676302000,2676302,2676302000,0,0,0,FALSE}; 
CDbgGlbls	dbg;
CDevGlbls	dev;
CGridGlbls	ms_grid; //See $LK,"::/Demo/Graphics/Grid.HC"$.
CMsStateGlbls	ms,ms_last;
CKbdStateGlbls	kbd;
CKeyDevGlbls	keydev;
CMsHardStateGlbls ms_hard,ms_hard_last;
CScrnCastGlbls	scrncast;
CTextGlbls	text;

U8  *(*fp_getstr2)(I64 flags=0);
U0 (*fp_update_ctrls)(CTask *task);
CDoc *(*fp_doc_put)(CTask *task=NULL);
U0 (*fp_set_std_palette)();

I64 sys_semas[SEMA_SEMAS_NUM];

//From KStart32
CProgress sys_progresses[4];
#define progress1 sys_progresses[0].val
#define progress1_max sys_progresses[0].max
#define progress1_t0 sys_progresses[0].t0
#define progress1_tf sys_progresses[0].tf
#define progress1_desc sys_progresses[0].desc

#define progress2 sys_progresses[1].val
#define progress2_max sys_progresses[1].max
#define progress2_t0 sys_progresses[1].t0
#define progress2_tf sys_progresses[1].tf
#define progress2_desc sys_progresses[1].desc

#define progress3 sys_progresses[2].val
#define progress3_max sys_progresses[2].max
#define progress3_t0 sys_progresses[2].t0
#define progress3_tf sys_progresses[2].tf
#define progress3_desc sys_progresses[2].desc

#define progress4 sys_progresses[3].val
#define progress4_max sys_progresses[3].max
#define progress4_t0 sys_progresses[3].t0
#define progress4_tf sys_progresses[3].tf
#define progress4_desc sys_progresses[3].desc

