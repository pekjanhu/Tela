#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLINE 4096
#define MAXFILE 100
#define MAXFILENAME 1024
static int Verbose = 0;
static char *Suffix = ".L";
static long meminuse = 0;

struct Tmsg;

typedef struct Tmsg {
	char *fn;
	int lineno;
	char *msg;
	struct Tmsg *next;
} Tmsg;

Tmsg *d[MAXFILE] = {0};
static int Nfiles = 0;

static void usage(void)
{
	fprintf(stderr,"usage: mkcclisting [-v] [-s .suffix] {files.V}\n");
	fprintf(stderr,"       makes Cray Standard C compiler compilation listing\n");
	fprintf(stderr,"       files using the raw messages .V files written by compiler.\n");
	fprintf(stderr,"Options:\n");
	fprintf(stderr,"-v         verbose mode\n");
	fprintf(stderr,"-s suffix  output file suffix, default .L\n");
	fprintf(stderr,"\nFor example, mkcclisting *.V will generate a .L file corresponding\n");
	fprintf(stderr,"to all source files. The .L files will appear in the same directory\n");
	fprintf(stderr,"levels as the .c files.\n");
	exit(1);
}

static void memerr(void) {
	fprintf(stderr,"*** mkcclisting: Not enough memory\n");
	exit(1);
}

static char* strdup2(char *s) {
	int L = strlen(s);
	char *p = (char *)malloc((L+1)*sizeof(char));
	if (!p) memerr();
	strcpy(p,s);
	return p;
}

static int parse(FILE *fp, Tmsg* mp)
/* Read two lines from fp. The first one is of the form:

   cc-8075 cc: VECTOR File = tree.C, Line = 67

   The second line is the message itself.
   The File and Line fields are grabbed from the first line and
   stored in Tmsg structure. The second line is also stored.
   Returns 1 on success, 0 on error.
*/
{
	int i,j,L2;
	char line1[MAXLINE+1], line2[MAXLINE+1];
	char *File = "File = ";
	char *Line = "Line = ";
	char *filepos, *linepos;
	char filestr[MAXLINE+1], linestr[MAXLINE+1];
	fgets(line1,MAXLINE,fp);
/*	if (Verbose) fprintf(stdout,"Line1 : %s",line1);*/
	if (feof(fp)) return 0;
	fgets(line2,MAXLINE,fp);
/*	if (Verbose) fprintf(stdout,"Line2 : %s",line2);*/
	filepos = strstr(line1,File);
	if (!filepos) {
		filepos = strstr(line1,"INLINE ");
		if (!filepos) {fprintf(stderr,"*** '%s' pattern not found in message file\n",File); return 0;}
		i = (filepos - &line1[0]) + strlen("INLINE ");
	} else
		i = (filepos - &line1[0]) + strlen(File);
	j = 0;
	while (line1[i] && line1[i]!=',') filestr[j++] = line1[i++];
	filestr[j] = '\0';
	linepos = strstr(line1,Line);
	if (!linepos) {fprintf(stderr,"*** '%s' pattern not found in message file\n",Line); return 0;}
	i = (linepos - &line1[0]) + strlen(Line);
	j = 0;
	while (line1[i] && isdigit(line1[i])) linestr[j++] = line1[i++];
	linestr[j] = '\0';
	mp->fn = strdup(filestr);
	meminuse+= strlen(mp->fn);
	L2 = strlen(line2);
	if (line2[L2-1] == '\n') line2[L2-1] = '\0';
	mp->msg = strdup(line2);
	meminuse+= strlen(mp->msg);
	/*if (Verbose) fprintf(stdout,"atoi(%s)\n",linestr);*/
	mp->lineno = atoi(linestr);
	return 1;
}

static void ProcessVfile(char *fn)
{
	char s[MAXLINE+1];
	FILE *fp;
	Tmsg msg, *ptr,*newptr;
	int line,j,found;
	fp = fopen(fn,"r");
	if (!fp) {fprintf(stderr,"*** Cannot open '%s'\n",fn); exit(2);}
	line = 0;
	if (Verbose) fprintf(stdout,"Processing '%s'\n",fn);
	while (1) {
		line+=2;
		if (!parse(fp,&msg)) {
			if (feof(fp)) break;
			fprintf(stderr,"*** Syntax error at line %d or %din .V file '%s'\n",line,line-1,fn);
			exit(3);
		}
		if (feof(fp)) break;
		found = 0;
		for (j=0; j<Nfiles; j++)
			if (!strcmp(msg.fn,d[j]->fn)) {
				found = 1;
				break;
			}
		newptr = (Tmsg*)malloc(sizeof(Tmsg));
		if (!newptr) memerr();
		meminuse+= sizeof(Tmsg);
		newptr->fn = msg.fn;
		newptr->lineno = msg.lineno;
		newptr->msg = msg.msg;
		if (found) {
			newptr->next = d[j];
			d[j] = newptr;
		} else {
			newptr->next = 0;
			d[Nfiles++] = newptr;
			if (Nfiles >= MAXFILE) {fprintf(stderr,"*** Too many (> %d) referred source files\n",MAXFILE-1); exit(4);}
		}
	}
	fclose(fp);
}

static void OutputData(void)
{
	int j;
	Tmsg *p;
	for (j=0; j<Nfiles; j++) {
		fprintf(stdout,"\"%s\":\n",d[j]->fn);
		for (p=d[j]; p; p=p->next)
			fprintf(stdout,"  Line %d : %s\n",p->lineno,p->msg);
	}
}

static void ProcessSourceFile(Tmsg *p)
{
	int line;
	Tmsg *q;
	char Lname[MAXFILENAME+1], s[MAXLINE+1];
	FILE *cfile, *Lfile;
	char *dotpos;
	cfile = fopen(p->fn,"r");
	if (!cfile) {fprintf(stderr,"%%%%%% Cannot open source file '%s' - ignoring\n",p->fn ? p->fn : "(null)"); return;}
	strcpy(Lname,p->fn);
	dotpos = strrchr(Lname,'.');
	if (dotpos && dotpos > &Lname[0] && *(dotpos+1)=='c' || *(dotpos+1)=='C') *dotpos = '\0';
	strcat(Lname,Suffix);
	Lfile = fopen(Lname,"w");
	if (!Lfile) {fprintf(stderr,"*** Cannot open L-file '%s'\n",Lname); exit(6);}
	line = 0;
	fprintf(Lfile,"----------------------------------------------------------\n");
	fprintf(Lfile,"  Cray Standard C V>=3.0 compiler listing\n");
	fprintf(Lfile,"  Produced by mkcclisting.c V2.0 (written by PJ 22.5.1995)\n");
	fprintf(Lfile,"----------------------------------------------------------\n");
	if (Verbose) fprintf(stdout,"- Source '%s', L-file '%s'\n",p->fn,Lname);
	while (1) {
		fgets(s,MAXLINE,cfile);
		if (feof(cfile)) break;
		line++;
		fprintf(Lfile,"%6d  %s",line,s);
		for (q=p; q; q=q->next)
			if (q->lineno == line) fprintf(Lfile,"******  %s\n",q->msg);
	}
	fclose(Lfile);
	fclose(cfile);
}

int main(int argc, char *argv[])
{
	int i,j;
	for (i=1; i<argc; i++)
		if (argv[i][0] == '-') {
			if (strchr("vs",argv[i][1])) {
				switch (argv[i][1]) {
				case 'v':
					Verbose = 1;
					break;
				case 's':
					if (argv[i][2])
						Suffix = argv[i]+2;
					else
						Suffix = argv[++i];
					break;
				default:
					fprintf(stderr,"%%%%%% Unknown flag '%c'\n",argv[i][1]);
					break;
				}
			}
		} else break;
	if (i >= argc) usage();
	for (; i<argc; i++) ProcessVfile(argv[i]);
	/*if (Verbose) OutputData();*/
	if (Verbose) fprintf(stdout,"Memory in use = %ld bytes\n",meminuse);
	for (j=0; j<Nfiles; j++) ProcessSourceFile(d[j]);
	return 0;
}
