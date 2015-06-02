#include "trans.h"
#include "traces.h"

int			verbose1=0;
int			verbose2=0;

/* Quick-lookup tree sets to hold trans.table and strip.table .
	Each set has MAXTREE roots, one for each possible length of
	the string to look up, and each root starts a binary tree of
	entries, all of that length.  (Thus lookup is fastest overall 
	if there's an even distribution of string-lengths from 1 to
	MAXTREE.  */
#define MAXTREE		200
tree *TransTree[MAXTREE];
tree *StripTree[MAXTREE];
int NeedTransTbl = 1;	/* Flag: Load trans.table 1st time thru. */
int NeedStripTbl = 1;	/* Flag: Load strip.table 1st time thru. */
static int		length = 0;
/* For each possible char value 0-127, keep count of table
	strings which begin with that char:  */
int		Transcount[128];
int		Stripcount[128];

int			printtest = 0;
int			sgmltype=PARTIAL;
int pdfcase=0;
int pdfstrip=0;

void errorexit(f,l,x)
   char                 *f;
   int                  l;
   int                  x;
{

   fprintf(stderr,"Error %d from %s:%d\n",x,f,l);
   exit(x);
}


char *strupper(string)
   char                 *string;
{
   char *s;
   int  nocvt=0;

   s = string;
   while (*s)
   {
      if (s[0] == '&')
        nocvt=1;
      if (s[0] == ';')
        nocvt=0;

      if (!nocvt)
        *s = toupper(*s);

      s++;
   }
   return(string);
}
char *struplow(string)
   char                 *string;
{
   char *s;
   static char casetmp[STRINGSIZE];	/* static so string usable after exit */
   char buffer[STRINGSIZE];
   char mytmp[2];
   int  suspend=0;


   memset(casetmp,0,sizeof(casetmp));
   memset(mytmp,0,sizeof(mytmp));
   memset(buffer,0,sizeof(buffer));

   strcpy(buffer,string);

   s=buffer;

   if(s[0]=='\n')
      s++;

   strncpy(casetmp,s,1);
   s++;


   while (*s)
   {
           if (s[0] == '&')
                suspend=1;
           else if (s[0] == ';' && suspend)
                suspend=0;

           else if(s[0]==' ' && s[1]=='A' && s[2]==' ')
              ;
           else if(s[0]==' ' && s[1]=='I' && toupper(s[2])=='S' && s[3]==' ')
              ;
           else if(s[0]==' ' && s[1]=='I' && toupper(s[2])=='N' && s[3]==' ')
              ;
           else if(s[0]==' ' && s[1]=='O' && toupper(s[2])=='F' && s[3]==' ')
              ;
           else if(s[0]==' ' && s[1]=='O' && toupper(s[2])=='N' && s[3]==' ')
              ;
           else if(s[0]==' ' && s[1]=='I' && toupper(s[2])=='T' && s[3]==' ')
              ;
           else if(s[0]==' ' && s[1]=='I' && toupper(s[2])=='F' && s[3]==' ')
              ;
           else if(s[0]==' ' && s[1]=='A' && toupper(s[2])=='N' && s[3]==' ')
              ;
           else if(s[0]==' ' && s[1]=='A' && toupper(s[2])=='N' && toupper(s[3])=='D' && s[4]==' ')
              ;
           else if(s[0]==' ' && s[1]=='T' && toupper(s[2])=='H' && toupper(s[3])=='E' && s[4]==' ')
              ;
           else if(s[0]==' ' && s[1]=='F' && toupper(s[2])=='O' && toupper(s[3])=='R' && s[4]==' ')
              ;


           else if (s[0] == ' ' || s[0] == '.' || s[0]=='-')
           {
                strncat(casetmp,s,1);                   /* copy char into casetmp */
                s++;
                suspend=1;
                continue;
           }

           else if (!suspend)
           {
                sprintf(mytmp,"%c",tolower(s[0]));
                strcat(casetmp,mytmp);
                s++;
                continue;
           }
           strncat(casetmp,s,1);
           s++;
           suspend=0;
    }
    strcat(casetmp,s);
    s=casetmp;
    return(s);
}

void del_uar(data)
    tTranslate 		*data;
{
    if (data)
    {
	if (data->from)
	    free(data->from);
	if (data->to)
	    free(data->to);
	free(data);
    }
}

static void tree_trav1(t, l)
   tree *t;
   int  l;
{
   int  i;

   if (!t)
      return;
   tree_trav1(t->tree_l, l+1);
   for (i=0;  i<l;  i++)
      printf("  ");
   fflush(stdout);
   printf("%8.8ld (%s)->(%s)\n", (unsigned long)t->tree_p, 
	  ((tTranslate *)t->tree_p)->from,
	  ((tTranslate *)t->tree_p)->to);
   fflush(stdout);
   tree_trav1(t->tree_r, l+1);
}

int compare(this, that)
    tTranslate		*this;
    tTranslate		*that;
{
    return(strncmp(this->from,that->from,length));
}

void LoadTable (Filename, unload, Tree, Count)
    char		*Filename;
    int			unload;
	tree		*Tree[];
	int			*Count;
{

    FILE		*fp;
    char		buffer[1024],
                        f[1024],
                        t[1024];
    char		*f1, *t1;
    int			i = 0;
    int			j = 0;

    tTranslate		*translate;
    char		tmp[1024];
    char		mytmp[1024];
    char		*p;
    char		charnum[10];
	int			count[128];

	for (i = 0; i < MAXTREE; i++) 			/* Init data structures to empty.  */
		tree_init((tree **) &(Tree[i]));
    memset(count,(int) 0, sizeof(count));

											/* Open the filter table:  */
    if ((fp = fopen(Filename,"r")) == (FILE *) NULL)
	{										/* Not there or inaccessible,  */
		memcpy (Count, count, sizeof(count));
		return;								/*  just return with empty data.  */
	}

    while (fgets(&buffer[0],sizeof(buffer)/sizeof(char),fp) != (char *) NULL)
    {


	f1=strtok(buffer,"\t");

	
	if (f1 && strcmp(f1,"PDFCASE") == 0) {
		t1=strtok((char *)NULL,"\n");
		pdfcase=atoi(t1);
	} else if (f1 && strcmp(f1,"PDFSTRIP") == 0) {
		t1=strtok((char *)NULL,"\n");
		pdfstrip=atoi(t1);
	} else if (f1 && strncmp(f1,"#",1) == 0) {
		continue;	/* comment line */
	} else if (f1) {
	   t1=strtok((char *)NULL,"\n");

	   if ( t1 && (strncmp(t1,"<<",2) == 0) && (strlen(t1) > 3) ) {
		strcpy(f,f1);
		p=t1;
		p+=2;
		memset(charnum,0,sizeof(charnum));
		sprintf(charnum,"%c",atoi(p));
		strcpy(t,charnum);
	   } else if (f1 && t1) {
	      strcpy(f,f1);
	      strcpy(t,t1);
	   } else {
	      continue;
	   }
        } else {
	   continue;
	}


	if ((translate = (tTranslate *) 
	     malloc(sizeof(tTranslate))) == (tTranslate *) NULL)
	    errorexit(__FILE__,__LINE__,errno);

	if ((translate->from = (char *) 
	     malloc((length = strlen(f))+1)) == (char *) NULL)
	    errorexit(__FILE__,__LINE__,errno);

	if ((translate->to   = (char *) 
	     malloc(strlen(t)+1)) == (char *) NULL)
	    errorexit(__FILE__,__LINE__,errno);

	if (strcmp(t,"<null>") == 0)
		strcpy(t,"");
	strcpy(translate->from,f);
	strcpy(translate->to,t);

	if (length >= MAXTREE)
	    length = MAXTREE-1;
	count[(int) *translate->from]++;
	
	tree_add((tree **) &(Tree[length]),compare,translate,del_uar);

	if ((translate = (tTranslate *) 
	     malloc(sizeof(tTranslate))) == (tTranslate *) NULL)
	    errorexit(__FILE__,__LINE__,errno);

	if ((translate->from = (char *) 
	     malloc((length = strlen(f))+1)) == (char *) NULL)
	    errorexit(__FILE__,__LINE__,errno);

	if ((translate->to   = (char *) 
	     malloc(strlen(t)+1)) == (char *) NULL)
	    errorexit(__FILE__,__LINE__,errno);


memset(mytmp,0,sizeof(mytmp));
for (j=0; j<=strlen(f); j++)
{
	memset(tmp,0,sizeof(tmp));
	if (f[j] == '*' || f[j] == '@')
	{
		sprintf(tmp,"%c",f[j]);
		strcat(mytmp,tmp);
		j++;
		sprintf(tmp,"%c",toupper(f[j]));  
		strcat(mytmp,tmp);
		
	}
	else
	{
		sprintf(tmp,"%c",f[j]);  
		strcat(mytmp,tmp);
	}

}

	strcpy(translate->from,mytmp);
	strcpy(translate->to,t);

	if (length >= MAXTREE)
	    length = MAXTREE-1;
	count[(int) *translate->from]++;
	
	tree_add((tree **) &(Tree[length]),compare,translate,del_uar);

    }
    fclose(fp);

	memcpy (Count, count, sizeof(count));
}


char *TranslateTable(SourceBuff)
   char	        	*SourceBuff;
{
char                        output[VECSIZE];
	
    char		*ip,
                        *op;
    tTranslate		translate;
    tTranslate		*fnd;
    char		*fp;
    char *p;
    int			gotit;
    char                namebuf[1024];
    char		bufftemp[10000];
	static char permout[129];	/* static so string usable after exit */

#ifdef TRACE
	if (debugger_trace)
	printf("Trans: NeedTransTbl=%d, NeedStripTbl=%d\n",
			NeedTransTbl, NeedStripTbl);
#endif
	if (NeedTransTbl)
	{
	    strcpy(namebuf,"/Penta/.default/userdata/.filters/trans.table");
		LoadTable (namebuf, 0, TransTree, Transcount);
		NeedTransTbl = 0;
	}

#ifdef TRACE
	if (debugger_trace)
	{
		printf("TransTree:\n");
		for (gotit = 0; gotit < MAXTREE; gotit++) 
		{
			printf(" %p ",TransTree[gotit]);
			if ((gotit+1)%10 == 0)
				printf ("\n");
		}
		printf ("\n");
	}
#endif
   memset(output,0,sizeof(output));

   ip = SourceBuff;
   op = output;
   while (*ip)      { 

	    if (Transcount[(int)*ip] == 0)
	    {
		*op++ = *ip++;
	    }
	    else
	    {
		translate.from = ip;
		gotit = 0;
		
		for ( length = MAXTREE-1; length > 0; length--)
		{
		    if ((fnd = (tTranslate *) 
			tree_srch((tree **) &(TransTree[length]),
				  compare,&translate)))
		    {
			fp = fnd->to;

			if (strncmp(fp,"!!",2) != 0) {
			   while (*fp)
			       *op++ = *fp++;
			}
			ip += length;
			gotit = 1;
			break;

		    }
		}
		if (gotit == 0)
		    *op++ = *ip++;
	    }
	}

#ifdef TRACE
	if (debugger_trace)
	printf("Trans: Done trans, str='%s'\n", output);
#endif
memset(SourceBuff,0,sizeof(SourceBuff));
strcpy(SourceBuff,output);
ip=SourceBuff;
memset(output,0,sizeof(output));
op = output;

   /* Translating using the strip table */

	if (NeedStripTbl)
	{
	    strcpy(namebuf,"/Penta/.default/userdata/.filters/strip.table");
		LoadTable (namebuf, 0, StripTree, Stripcount);
		NeedStripTbl = 0;
	}

   while (*ip)      {

            if (Stripcount[(int)*ip] == 0)
            {
                *op++ = *ip++;
            }
            else 
            {
                translate.from = ip;
                gotit = 0;

                for ( length = MAXTREE-1; length > 0; length--)
                {
                    if ((fnd = (tTranslate *)
                        tree_srch((tree **) &(StripTree[length]),
                                  compare,&translate)))
                    {
			/* Found start of strip string  - set ignore until fnd->to is found */
			/* Get End strip string*/
                        fp = fnd->to;
                        ip += length;

			p=strstr(ip,fp);
			if (p != (char *)NULL)  {
				ip=p;
				ip += strlen(fp);
			} else {
			 ip[0]='\0';
			}

                        gotit = 1;
                        break;

                    }
                }

                if (gotit == 0)
                    *op++ = *ip++;
            }
        }
#ifdef TRACE
	if (debugger_trace)
		printf("Trans: Done strip, str='%s'\n", output);
#endif

op=output;


		/* Strip preceding section numbers */
		memset(bufftemp,0,sizeof(bufftemp));
		switch (pdfstrip) {
			case 0: /* nothing */
				strcpy(bufftemp,op);
				break;
			case 1: /* Strip section numbers */
                                p = strstr(op,". ");
                                if (p) {
                           		p+=2;
                           		if (p[0] != ' ') {
                                	strcpy(bufftemp,p);
					}
                                } else {
					strcpy(bufftemp,op);
				}
				break;
                }
op=output;
strcpy(output,bufftemp);



               /* Change Case */
                switch (pdfcase) {
                        case 0: /* nothing */
                                memset(bufftemp,0,sizeof(bufftemp));
                                strcpy(bufftemp,op);
                                break;
                        case 1: /* upper/lower case */
                                memset(bufftemp,0,sizeof(bufftemp));
                                sprintf(bufftemp,"%s",struplow(op));
                                break;
                        case 2: /* caps */
                                memset(bufftemp,0,sizeof(bufftemp));
                                sprintf(bufftemp,"%s",strupper(op));
                                break;
                        default:
                                break;
                }

/* Can't have unbalanced parens in PostScript or PDF, which
	truncation could cause, so escape all parens with '\':  */
ip = bufftemp;
op = output;
do {					/* Loop to transfer all characters:  */
	if (*ip == '(' || *ip == ')')	/* If char is a paren, and  */
		if (ip == bufftemp ||		/*  if it's 1st char of buff or  */
			*(ip-1) != '\\' )		/*  it's not already preceded by '\', then:  */
			*op++ = '\\';		/* Insert \ before the paren char.  */
	*op++ = *ip;				/* Xfer each character.  */
} while (*ip++);		/* Exit after xfering terminating null.  */

/* Truncate at 124 charnums */
if (strlen(output) > 127)
	strcpy (output+123, " ...");

strcpy (permout, output);

#ifdef TRACE
	if (debugger_trace)
	{
		int ideb = strlen(permout);
		printf("Trans exit: strlen=%d, str='%s'\n", ideb, permout);
	}
#endif

return (permout);

}  /* End TranslateTable()  */ 

