#include "stdio.h"
#include "stdlib.h"
#include "constvar.h"
//#define AnaTypeLex
//#define AnaTypeSyn

extern TERMINAL nextToken();
extern void renewLex();
static int match (int t);
static int strcompare(char *sstr, char *tstr);	//比较两个串
static IDTABLE* InstallID();		//在符号表中为curtoken_str建立一个条目
static IDTABLE* LookupID();			//在符号表中查找curtoken_str
static void FreeExit();
static int cast2int(EXPVAL exp);		//将exp的值转换为int类型
static char cast2char(EXPVAL exp);		//将exp的值转换为char类型
static int Prod_FUNC();
static int Prod_S();
static int Prod_D();
static int Prod_L(int type);
static int Prod_T();
static int Prod_A();
static EXPVAL Prod_B();
static EXPVAL Prod_B1(EXPVAL val);
static EXPVAL Prod_TB();
static EXPVAL Prod_TB1(EXPVAL val);
static EXPVAL Prod_FB();
static EXPVAL Prod_E();
static EXPVAL Prod_E1(EXPVAL val);
static EXPVAL Prod_TE();
static EXPVAL Prod_TE1(EXPVAL val);
static EXPVAL Prod_F();
static EXPVAL Prod_K();
static EXPVAL Prod_K3(EXPVAL val1);
static EXPVAL Prod_K2();
static EXPVAL Prod_K1(EXPVAL val1);

extern FILE *sFile;
static TERMINAL lookahead;
static int curtoken_num;
static char curtoken_char;
static char curtoken_str[MAXTOKENLEN];
static IDTABLE *IDTHead=NULL;
static int run_status=1;	//0；程序不执行；1:程序正常执行；2:跳过当前结构后继续执行
int is_loop=0;
int is_break=0;
int is_continue=0;

void SyntaxAnalysis()
{
#if defined(AnaTypeLex)
//testing lexical analysis
	TERMINAL token;
	token=nextToken();
	while (token.token!=ERR)
	{	if (token.token==SYN_NUM)
			printf("LEX: %d,%d\n",token.token,token.tokenVal.number);
		else if (token.token==SYN_ID)
			printf("LEX: %d,%s\n",token.token,token.tokenVal.str);
			printf("LEX: %d\n",token.token);
		token=nextToken();
	}
#else
//syntax analysis
	lookahead=nextToken();
	if (Prod_FUNC()==ERR)
		printf("PROGRAM HALT!\n");
	FreeExit();

#endif
}

static int match (int t)
{
	char *p,*q;
	if (lookahead.token == t)
	{	if (t==SYN_NUM)
			curtoken_num=lookahead.tokenVal.number;
		else if (t==SYN_ID)
        {
		    for (p=lookahead.tokenVal.str,q=curtoken_str;(*q=*p)!='\0';p++,q++);
        }
        else if (t==SYN_CHAR)
        {
            curtoken_char=lookahead.tokenVal.character;
        }
        lookahead = nextToken( );
	}
	else{
        printf("FreeExit\n");
		FreeExit();}
	return(0);
}

static int strcompare(char *sstr, char *tstr)
{
	while (*sstr==*tstr && *sstr!='\0') { sstr++; tstr++; }
	if (*sstr=='\0' && *tstr=='\0')	return(0);
	else return(ERR);
}

static IDTABLE* InstallID()
{
	IDTABLE *p,*q;
	char *a,*b;
	p=IDTHead; q=NULL;
	while (p!=NULL && strcompare(curtoken_str,p->name)==ERR)
	{
		q=p;
		p=p->next;
	}
	if (p!=NULL)
		return(NULL);
	else
	{
		p=(IDTABLE*)malloc(sizeof(IDTABLE));
		if (q==NULL)
			IDTHead=p;
		else
			q->next=p;
		p->next=NULL;
		for (a=curtoken_str,b=p->name;(*b=*a)!='\0';a++,b++);
		return(p);
	}
}

static IDTABLE* LookupID()
{
	IDTABLE *p;
	p=IDTHead;
	while (p!=NULL && strcompare(curtoken_str,p->name)==ERR)
		p=p->next;
	if (p==NULL)
		return(NULL);
	else
		return(p);
}

static void FreeExit()
{
	IDTABLE *p,*q;
	//释放链表空间
	p=IDTHead;
	while ((q=p)!=NULL)
	{	p=p->next;
		#if defined(AnaTypeSyn)
		printf("NAME:%s, TYPE:%d, ",q->name,q->type);
		if (q->type==ID_INT)
			printf("VALUE:%d\n",q->val.intval);
		else if (q->type==ID_CHAR)
			printf("VALUE:%c\n",q->val.charval);
		else
			printf("\n");
		#endif
		free(q);
	}
	exit(0);
}

static int cast2int(EXPVAL exp)
{
	if (exp.type==ID_INT)
		return(exp.val.intval);
	else if (exp.type==ID_CHAR)
		return((int)(exp.val.charval));
}

static char cast2char(EXPVAL exp)
{
	if (exp.type==ID_INT)
		return((char)(exp.val.intval));
	else if (exp.type==ID_CHAR)
		return(exp.val.charval);
}

static int Prod_FUNC()
{
	IDTABLE *p;
	match(SYN_ID);
	if (strcompare(curtoken_str,"main")==ERR) FreeExit();
	p=InstallID();
	p->type=ID_FUN;
	#if defined(AnaTypeSyn)
	printf("SYN: FUNC-->main() {S}\n");
	#endif
	match(SYN_PAREN_L);
	match(SYN_PAREN_R);
	match(SYN_BRACE_L);
	Prod_S();
	match(SYN_BRACE_R);
	return(0);
}

static int Prod_S()
{
    if(is_loop!=0 && is_break==1)
    {
        run_status=2;
    }
    if(is_loop!=0 && is_continue==1)
    {
        run_status=2;
    }
	long file_index;
	EXPVAL exp,tmp;
	int bval;
    tmp.type=ID_INT;
	if (lookahead.token==SYN_INT || lookahead.token==SYN_CHAR)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->D S\n");
		#endif
		Prod_D();
		Prod_S();
	}
	else if (lookahead.token==SYN_ID)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->A S\n");
		#endif
	    Prod_A();
		Prod_S();
	}
	else if (lookahead.token==SYN_SHOW)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->show(K); S\n");
		#endif
		match(SYN_SHOW);
		match(SYN_PAREN_L);
		exp=Prod_K();
		match(SYN_PAREN_R);
		match(SYN_SEMIC);//problem aru!
		if (run_status==1)
		    if (exp.type==ID_INT)
				printf("%d\n",exp.val.intval);
			else if (exp.type==ID_CHAR)
				printf("%c\n",exp.val.charval);
		Prod_S();
	}
    else if (lookahead.token==SYN_BREAK)
    {
        #if defined(AnaTypeSyn)
        printf("SYN: S-->break;S\n");
        #endif
        match(SYN_BREAK);
        if(is_loop!=0 && run_status==1)
        {
            is_break=1;
        }
        match(SYN_SEMIC);
        Prod_S();
    }
    else if (lookahead.token==SYN_CONTINUE)
    {
        #if defined(AnaTypeSyn)
        printf("SYN: S-->continue;S\n");
        #endif
        match(SYN_CONTINUE);
        if(is_loop!=0 && run_status==1)
        {
            is_continue=1;
        }
        match(SYN_SEMIC);
        Prod_S();
    }
	else if (lookahead.token==SYN_IF)
	{
        int tmp_status=run_status;
		#if defined(AnaTypeSyn)
		printf("SYN: S-->if (K) {S} [else {S}] S\n");
		#endif
		match(SYN_IF);
		match(SYN_PAREN_L);
		tmp=Prod_K();
        bval=cast2int(tmp);
		match(SYN_PAREN_R);
	    if (run_status==1 && bval==0) {run_status=2;}
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		if (lookahead.token==SYN_ELSE)
		{
			match(SYN_ELSE);
			if (run_status==1) run_status=2;
			else if (run_status==2) run_status=1;
			match(SYN_BRACE_L);
			Prod_S();
			match(SYN_BRACE_R);
			if (run_status==2) run_status=1;
		}
        run_status=tmp_status;
		Prod_S();
	}
	else if (lookahead.token==SYN_WHILE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->while(K) {S} S\n");
		#endif
        is_loop+=1;
		match(SYN_WHILE);
		file_index=ftell(sFile)-1;
		match(SYN_PAREN_L);
		tmp=Prod_K();
        bval=cast2int(tmp);
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) run_status=2;
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		if (run_status==1 && is_break!=1)
		{	fseek(sFile,file_index,SEEK_SET);
			renewLex();
            lookahead.token=SYN_WHILE;
	    }
        else if(run_status==2 && is_continue==1)
		{	fseek(sFile,file_index,SEEK_SET);
			renewLex();
            lookahead.token=SYN_WHILE;
            run_status=1;
	    }
		else if (run_status==2)
        {run_status=1;}
        if (is_break==1)
        {
            is_break=0;
        }
        if(is_continue==1)
        {
            is_continue=0;
        }
        is_loop-=1;
		Prod_S();
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S--> \n");
		#endif
	}
	return(0);
}

static int Prod_D()
{
	int type;
	IDTABLE *p;
	EXPVAL exp;
	#if defined(AnaTypeSyn)
	printf("SYN: D-->T id [=K] L;\n");
	#endif
	type=Prod_T();
	match(SYN_ID);
	p=InstallID();
	p->type=type;
	if (lookahead.token==SYN_SET)
	{
		match(SYN_SET);
		exp=Prod_K();
		if (run_status==1)
		{	if (type==ID_INT)
				p->val.intval=cast2int(exp);
			else if (type==ID_CHAR)
				p->val.charval=cast2char(exp);
		}
	}
	Prod_L(type);
	match(SYN_SEMIC);
	return(0);
}

static int Prod_L(int type)
{
	IDTABLE *p;
	EXPVAL exp;
	if (lookahead.token==SYN_COMMA)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: L-->, id [=K] L\n");
		#endif
		match(SYN_COMMA);
		match(SYN_ID);
		p=InstallID();
		p->type=type;
		if (lookahead.token==SYN_SET)
		{
			match(SYN_SET);
			exp=Prod_K();
			if (run_status==1)
			{	if (type==ID_INT)
					p->val.intval=cast2int(exp);
				else if (type==ID_CHAR)
					p->val.charval=cast2char(exp);
			}
		}
		Prod_L(type);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: L--> \n");
		#endif
	}
	return(0);
}

static int Prod_T()
{
	if (lookahead.token==SYN_INT)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: T-->int\n");
		#endif
		match(SYN_INT);
		return(ID_INT);
	}
	else if (lookahead.token==SYN_CHAR)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: T-->char\n");
		#endif
		match(SYN_CHAR);
		return(ID_CHAR);
	}
	else
		FreeExit();
	return(0);
}

static int Prod_A()
{
	IDTABLE *p;
	EXPVAL exp;
	#if defined(AnaTypeSyn)
	printf("SYN: A-->id=E;\n");
	#endif
	match(SYN_ID);
	p=LookupID();
	match(SYN_SET);
	exp=Prod_K();
	match(SYN_SEMIC);
	if (run_status==1)
	{	if (p->type==ID_INT)
			p->val.intval=cast2int(exp);
		else if (p->type==ID_CHAR)
			p->val.charval=cast2char(exp);
	}
	return(1);
}
// Change in my version


static EXPVAL Prod_K()
{
	EXPVAL val1,val2;
	#if defined(AnaTypeSyn)
	printf("SYN: K-->K2 K1\n");
	#endif
	val1=Prod_K2();
	val2=Prod_K1(val1);
    int x=cast2int(val2);
	return(val2);
}

static EXPVAL Prod_K3(EXPVAL val1)
{
	EXPVAL val2,val;
	int i1,i2;
	char c1,c2;
	if (lookahead.token==SYN_ADD)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: K3-->+K2 K3\n");
		#endif
		match(SYN_ADD);
		val2=Prod_K2();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1+i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1+c2;
			}
		val=Prod_K3(val);
	}
	else if (lookahead.token==SYN_SUB)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1-->-TE E1\n");
		#endif
		match(SYN_SUB);
		val2=Prod_K2();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1-i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1-c2;
			}
		val=Prod_K3(val);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: K3--> \n");
		#endif
		val=val1;
	}
	return(val);
}

static EXPVAL Prod_K2()
{
	EXPVAL val1,val2;
	#if defined(AnaTypeSyn)
	printf("SYN: K2-->B K1\n");
	#endif
	val1=Prod_B();
	val2=Prod_K1(val1);
	return(val2);
}

static EXPVAL Prod_K1(EXPVAL val1)

{
	EXPVAL val2,val;
	int i1,i2;
	char c1,c2;
	if (lookahead.token==SYN_MUL)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: K1-->*B K1\n");
		#endif
		match(SYN_MUL);
		val2=Prod_B();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1*i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1*c2;
			}
		val=Prod_K1(val);
	}
	else if (lookahead.token==SYN_DIV)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: K1-->/F K1\n");
		#endif
		match(SYN_DIV);
		val2=Prod_B();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1/i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1/c2;
			}
		val=Prod_K1(val);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: K1--> \n");
		#endif
		val=val1;
	}
	return(val);
}

static EXPVAL Prod_B()
{
    EXPVAL val,tmp;
	int bval1,bval2;
    val.val.intval=ID_INT;
	if(lookahead.token==SYN_PAREN_L)
	{
	    #if defined(AnaTypeSyn)
	printf("SYN: B-->(B)\n");
	#endif
	match(SYN_PAREN_L);
	val=Prod_B();
	match(SYN_PAREN_R);
	return(val);
	}
	else{
	#if defined(AnaTypeSyn)
	printf("SYN: B-->TB B1\n");
	#endif
    tmp=Prod_TB();
	bval1=cast2int(tmp);
    val=Prod_B1(tmp);
	return(val);}

}

static EXPVAL Prod_B1(EXPVAL val1)
{
	int bval2,bval1;
    EXPVAL val,tmp;
    val.type=ID_INT;
    bval1=cast2int(val1);
	if (lookahead.token==SYN_OR)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: B1-->|| TB B1\n");
		#endif
		match(SYN_OR);
        tmp=Prod_TB();
		bval2=tmp.val.intval;
		bval1=(run_status==1 && (bval1==1 || bval2==1)) ? 1 : 0;
        if(val1.type==ID_CHAR){val1.val.charval=bval1;}
        else if(val1.type==ID_INT) {val1.val.intval=bval1;}
		val=Prod_B1(val1);
		return(val);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: B1--> \n");
		#endif
		return(val1);
	}
}

static EXPVAL Prod_TB()
{
    EXPVAL val,tmp;
	int bval1,bval2;
    val.type=ID_INT;
	#if defined(AnaTypeSyn)
	printf("SYN: TB-->FB TB1\n");
	#endif
    tmp=Prod_FB();
	bval1=tmp.val.intval;
	val=Prod_TB1(tmp);
	return(val);
}

static EXPVAL Prod_TB1(EXPVAL val1)
{
	int bval2;
    int bval1;
    EXPVAL val,tmp;
    val.type=ID_INT;
    bval1=cast2int(val1);
	if (lookahead.token==SYN_AND)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TB1-->&&FB TB1\n");
		#endif
		match(SYN_AND);
        tmp=Prod_FB();
		bval2=cast2int(tmp);
		bval1=(run_status==1 && (bval1==1 && bval2==1)) ? 1 : 0;
        if(val1.type==ID_INT) val1.val.intval=bval1;
        else if(val1.type==ID_CHAR) val1.val.charval=bval1;
		tmp=Prod_TB1(val1);
        bval2=tmp.val.intval;
        val.val.intval=bval2;
        return(val);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TB1--> \n");
		#endif
        return(val1);
	}
}

static EXPVAL Prod_FB()
{
	int bval;
	EXPVAL val1,val2;
    EXPVAL val,tmp;
	int ival1,ival2;

    val.type=ID_INT;
	if (lookahead.token==SYN_NOT)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: FB-->!B\n");
		#endif
		match(SYN_NOT);
		tmp=Prod_B();
        bval=tmp.val.intval;
	    val.val.intval=(run_status==1 ? 1-bval : 0);
        return(val);
	}
	else if (lookahead.token==SYN_TRUE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: FB-->TRUE\n");
		#endif
		match(SYN_TRUE);
		val.val.intval=(run_status==1 ? 1 : 0);
        return(val);
	}
	else if (lookahead.token==SYN_FALSE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: FB-->FALSE\n");
		#endif
		match(SYN_FALSE);
		val.val.intval=(run_status==1 ? 0 : 0);
        return(val);
	}
	else if (lookahead.token==SYN_ID || lookahead.token==SYN_NUM || lookahead.token==SYN_PAREN_L ||lookahead.token==SYN_CHAR)
	{
		val1=Prod_E();
		if (run_status==1) ival1=cast2int(val1);
		if (lookahead.token==SYN_LT)
		{	
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E<E\n");
			#endif
			match(SYN_LT);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				val.val.intval=(ival1<ival2 ? 1 : 0);
                return(val);
			}
			else
				val.val.intval=(0);
                return(val);
		}
		else if (lookahead.token==SYN_LE)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E<=E\n");
			#endif
			match(SYN_LE);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
			    val.val.intval=(ival1<=ival2 ? 1 : 0);
        return(val);
			}
			else
				val.val.intval=(0);
        return(val);
		}
		else if (lookahead.token==SYN_GT)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E>E\n");
			#endif
			match(SYN_GT);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				val.val.intval=(ival1>ival2 ? 1 : 0);
        return(val);
			}
			else
				val.val.intval=(0);
        return(val);
		}
		else if (lookahead.token==SYN_GE)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E>=E\n");
			#endif
			match(SYN_GE);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				val.val.intval=(ival1>=ival2 ? 1 : 0);
        return(val);
			}
			else
				val.val.intval=(0);
        return(val);
		}
		else if (lookahead.token==SYN_EQ)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E==E\n");
			#endif
			match(SYN_EQ);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				int x=(ival1==ival2 ? 1 : 0);
                val.val.intval= x;
        return(val);
			}
			else
				val.val.intval=(0);
        return(val);
		}
		else if (lookahead.token==SYN_NE)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E!=E\n");
			#endif
			match(SYN_NE);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				val.val.intval=(ival1!=ival2 ? 1 : 0);
        return(val);
			}
			else
				val.val.intval=(0);
        return(val);
		}
		else
		{
			if (run_status==1)
            {
                if(val1.type==ID_CHAR)
                {
                    val.type=ID_CHAR;
                    val.val.charval=ival1;
                }
                else{
                val.val.intval=ival1;}
        return(val);}
			else
            {	val.val.intval=(0);
        return(val);}
		}

	}
	else
	{	FreeExit();
		val.val.intval=(0);
        return(val);
	}
}

static EXPVAL Prod_E()
{
	EXPVAL val1,val2;
	#if defined(AnaTypeSyn)
	printf("SYN: E-->TE E1\n");
	#endif
	val1=Prod_TE();
	val2=Prod_E1(val1);
    int x=cast2int(val2);
	return(val2);
}

static EXPVAL Prod_E1(EXPVAL val1)
{
	EXPVAL val2,val;
	int i1,i2;
	char c1,c2;
	if (lookahead.token==SYN_ADD)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1-->+TE E1\n");
		#endif
		match(SYN_ADD);
		val2=Prod_TE();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1+i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1+c2;
			}
		val=Prod_E1(val);
	}
	else if (lookahead.token==SYN_SUB)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1-->-TE E1\n");
		#endif
		match(SYN_SUB);
		val2=Prod_TE();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1-i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1-c2;
			}
		val=Prod_E1(val);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1--> \n");
		#endif
		val=val1;
	}
	return(val);
}

static EXPVAL Prod_TE()
{
	EXPVAL val1,val2;
	#if defined(AnaTypeSyn)
	printf("SYN: TE-->F TE1\n");
	#endif
	val1=Prod_F();
	val2=Prod_TE1(val1);
	return(val2);
}

static EXPVAL Prod_TE1(EXPVAL val1)
{
	EXPVAL val2,val;
	int i1,i2;
	char c1,c2;
	if (lookahead.token==SYN_MUL)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TE1-->*F TE1\n");
		#endif
		match(SYN_MUL);
		val2=Prod_F();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1*i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1*c2;
			}
		val=Prod_TE1(val);
	}
	else if (lookahead.token==SYN_DIV)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TE1-->/F TE1\n");
		#endif
		match(SYN_DIV);
		val2=Prod_F();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1/i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1/c2;
			}
		val=Prod_TE1(val);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TE1--> \n");
		#endif
		val=val1;
	}
	return(val);
}

static EXPVAL Prod_F()
{
	EXPVAL val;
	static IDTABLE *p;
    char symb;
	if (lookahead.token==SYN_NUM)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->num\n");
		#endif
		match(SYN_NUM);
		val.type=ID_INT;
		val.val.intval=curtoken_num;
	}
	else if (lookahead.token==SYN_ID)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->id\n");
		#endif
		match(SYN_ID);
		p=LookupID();
		val.type=p->type;
		val.val=p->val;
	}
	else if (lookahead.token==SYN_PAREN_L)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->(K)\n");
		#endif
		match(SYN_PAREN_L);
		val=Prod_K();
		match(SYN_PAREN_R);
	}
    else if (lookahead.token=SYN_CHAR)
    {
		#if defined(AnaTypeSyn)
		printf("SYN: F-->CHAR\n");

		#endif
        match(SYN_CHAR);
        val.type=ID_CHAR;
        val.val.charval=curtoken_char;
    }
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->K\n");

		#endif
		FreeExit();
	}
		return(val);
}
