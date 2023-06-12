/********************************************************************
  BasicDSP
  Copyright 2006-2007 
  Pieter-Tjerk de Boer PA3FWM pa3fwm@amsat.org
  Niels Moseley PE1OIT n.a.moseley@alumnus.utwente.nl
  License: GPLv2
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "wavstreamer.h"

#include "core.h"

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

#ifdef WIN32
#define strcasecmp(x,y) stricmp(x,y)
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#endif

float sweeprate=0;    // Hz/sec
float sweep_freq=100,sweep_phase=0;
int impulse_phase=1;

int inputsource=0;

int stopped=0;

bool remove_dc = false;

float samplerate=48000;
float virtsamplerate=8000;
float prev_virtsamplerate=0;

WavStreamer wavstreamer;

#define Maxlinelen 2048

void Exit(const char *s)
{
   printf("%s\n",s);
   exit(1);
}

float *sliderptr[NUMSLIDERS]={&sweep_freq, NULL, NULL, NULL, NULL};

float left_channel_max=0;
float right_channel_max=0;


//////////////////////////////////////////////////////////////////////////
// the stuff is for error reporting during compile:

int erroroffset1,erroroffset2;
char *errorstr;


class ParserException
{
public:
   int erroroffset1;
   int erroroffset2;
   char *errorstr;
};

// Exits3's arguments: 
// s1 points to an error string
// o1 and o2 are offset of begin and end of the offending code
// erroroffset0 is offset of string being compiled now inside string which was fed to compile_one_line()
#define Exits3(s1,o1,o2) { ParserException e; e.erroroffset1=o1+erroroffset0; e.erroroffset2=o2+erroroffset0; e.errorstr = s1; throw e; }

// Exits5 only gets a pointer to the error string, and will mark the entire line as offending
#define Exits5(s1) { ParserException e; e.erroroffset1 = 0; e.erroroffset2 = 32767; e.errorstr = s1; throw e; }

//////////////////////////////////////////////////////////////////////////
// definitions for the "byte code" representation of the user's code

typedef union {
   int i;
   float f;
} Progstep;

#define maxprogstep 16384
Progstep prog[maxprogstep];
int progsteps=0;

int recordstep(int a)
{
   if (progsteps>=maxprogstep) Exits5("Program too long");
   prog[progsteps++].i=a;
   return 0;
}

int recordstep_f(float a)
{
   if (progsteps>=maxprogstep) Exits5("Program too long");
   prog[progsteps++].f=a;
   return 0;
}

#define P_add 1
#define P_sub 2
#define P_mul 3
#define P_div 4
#define P_sin 5
#define P_cos 6
#define P_const 7
#define P_print 8
#define P_mod1 9
#define P_abs 10
#define P_sin1 11
#define P_cos1 12
#define P_round 13
#define P_sqrt 14
#define P_tan 15
#define P_tanh 16
#define P_pow 17

// the following opcodes use the lower 16 bits for further identifying a variable or FIR
#define P_writevar 0x81000000
#define P_readvar  0x82000000
#define P_fir      0x83000000
#define P_biquad   0x84000000


//////////////////////////////////////////////////////////////////////////
// definitions related to user's variables

#define maxvarnamelen 19
#define maxvars 500

typedef struct {
   char name[maxvarnamelen+1];
   float val;
} Variable;

Variable vars[maxvars];
int nxtvar=0;

Variable var_zero = {"", 0};

int findvarnr(char *p)
{
   int i;
   for (i=0;i<nxtvar;i++)
      if (strcasecmp(p,vars[i].name)==0) return i;
   return -1;
}

Variable *findvar(char *p)
{
   int i;
   for (i=0;i<nxtvar;i++)
      if (strcasecmp(p,vars[i].name)==0) return vars+i;
   return NULL;
}

int newvarnr(char *name,int erroroffset0)
{
   int i;
   if (nxtvar>=maxvars) Exits3("Too many variables",0, strlen(name));
   if (strlen(name)>maxvarnamelen) Exits3("Variable name too long",0,strlen(name));
   for (i=0;i<(signed)strlen(name);i++) 
      if (!isalnum(name[i]) && name[i]!='_') 
         Exits3("Illegal character in variable name",i,i+1);
   strcpy(vars[nxtvar].name,name);
   vars[nxtvar].val=0;
   nxtvar++;
   return nxtvar-1;
}

//////////////////////////////////////////////////////////////////////////
// definitions related to user-defined Biquad filters

#define maxbiquads 16

typedef struct {
   float state[4];
   int type;     // can be 5 or 6, being the number of parameters
} Biquad;

Biquad biquads[maxbiquads];
int nxtbiquad=0;

int newbiquad(int type)
{
   if (nxtbiquad==maxbiquads) Exits5("Too many Biquad filters");
   biquads[nxtbiquad].state[0] = 0;
   biquads[nxtbiquad].state[1] = 0;
   biquads[nxtbiquad].state[2] = 0;
   biquads[nxtbiquad].state[3] = 0;
   biquads[nxtbiquad].type = type;
   nxtbiquad++;
   return nxtbiquad-1;
}

int execbiquad(int n, float *stack)
{
   Biquad *p = biquads+n;

   if (p->type==5) {
      float out = 0;
      float sum_temp = 0;
      float newsample;

      stack-=5;

      newsample = stack[-1];
      sum_temp = newsample * stack[0]         // gain*input
                 - p->state[0] * stack[1]     // - a1*state0
                 - p->state[1] * stack[2];    // - a2*state1

      out = sum_temp + p->state[0] * stack[3] +	  // sum_temp + b1*state0
            p->state[1] * stack[4];		  // + b2*state1

      p->state[1] = p->state[0];			  // update filter states
      p->state[0] = sum_temp;

      stack[-1]=out;  // write output sample in appropriate place onto stack
      return 5;       // change of stackpointer

   } else {

      // type==6
      // note: type 5 and 6 are not fundamentally different and could actually
      // be handled by the same calculation with different coefficients.
      float out;
      float a0,a1,a2,b0,b1,b2;
      stack-=6;
      a0 = stack[0];
      a1 = stack[1];
      a2 = stack[2];
      b0 = stack[3];
      b1 = stack[4];
      b2 = stack[5];
      out = (b0/a0) * stack[-1] + (b1/a0) * p->state[0] + (b2/a0) * p->state[1]
                                - (a1/a0) * p->state[2] - (a2/a0) * p->state[3];
      p->state[1] = p->state[0];
      p->state[0] = stack[-1];
      p->state[3] = p->state[2];
      p->state[2] = out;

      stack[-1] = out;
      return 6;
   }
}

//////////////////////////////////////////////////////////////////////////
// definitions related to user-defined FIR filters

#define maxfirs 16
#define maxfirlen 256   // must be a power of 2
#define firmask (maxfirlen-1)

typedef struct {
   int len;
   int ptr;
   int symmetric;   // 0 if not symmetric, 1 if symmetric with odd length, 2 if symmetric with even length
   float data[maxfirlen];
} Fir;

Fir firs[maxfirs];
int nxtfir=0;

int newfir(void)
{
   int i;
   if (nxtfir==maxfirs) Exits5("Too many FIR filters");
   firs[nxtfir].len=0;
   firs[nxtfir].ptr=0;
   firs[nxtfir].symmetric=0;
   for (i=0;i<maxfirlen;i++) firs[nxtfir].data[i]=0;
   nxtfir++;
   return nxtfir-1;
}

int execfir(int n,float *stack)
{
   int i;
   float out=0;
   Fir *f=firs+n;
   stack-=f->len;

   float newsample = stack[-1];
   f->data[f->ptr]=newsample;
   switch (f->symmetric) {
      case 0: 
         for (i=0;i<f->len;i++) 
            out += stack[i] * f->data[(f->ptr-i)&firmask]; 
         break;
      case 1: 
         for (i=0;i<f->len-1;i++) 
            out += stack[i] * ( f->data[(f->ptr-i)&firmask] + f->data[(f->ptr+i-2*f->len+2)&firmask] ); 
         out += stack[f->len-1] * f->data[(f->ptr-f->len+1)&firmask];
         break;
      case 2: 
         for (i=0;i<f->len;i++) 
            out += stack[i] * ( f->data[(f->ptr-i)&firmask] + f->data[(f->ptr+i-2*f->len+1)&firmask] ); 
         break;
   }
   f->ptr=(f->ptr+1)&firmask;
   stack[-1]=out;  // write output sample in appropriate place onto stack
   return f->len;  // change of stackpointer
}


//////////////////////////////////////////////////////////////////////////
// the compiler

#define sepchars " ;\r\n\t"

char *nextcomma(char *p)
{
   int depth=0;
   while (*p) {
      switch (*p) {
         case ',':
            if (depth==0) return p;
            break;
         case '(':
	    depth++;
	    break;
         case ')':
	    depth--;
	    break;
      }
      p++;
   }
   return NULL;
}

// eval() evaluates a string containing an expression; evaluation here means writing appropriate steps into the program memory
void eval(const char *expr,int erroroffset0)
{
   char s[Maxlinelen];
   char *p,*ph=NULL;
   int i=0;
   strcpy(s,expr);
//printf("eval: ### %s ###\n",expr);

   // first, search for parentheses
   // in s[], erase everything between the outermost parentheses
   // let ph point to the last closing parenthesis
   p=s;
   while (*p!=0) {
      if (*p=='(') i++;
      if (*p==')') {i--;ph=p;}
      if (i>0) *p=' ';
      p++;
   }
   if (i!=0) Exits3("Mismatched parentheses: ",0,strlen(expr));

   // next, look in s[] for the right-most + or -, or if there is none, for the right-most * or /
   // note though that a - may also occur inside a numerical constant, we need to skip those
   // let p point to this right-most operator
   {
      int n;
      float a;
      char *p_plusminus=NULL;
      char *p_muldiv=NULL;

      p=s;

      // we go over the string from left to right, searching for operators
      // everytime we find an operator, we use sscanf() to check whether there's a numerical constant to right of it,
      // and if so, we move our pointer to the end of this numerical constant;
      // thus, we skip + and - embedded in such constants
     
      n=0;
      if (sscanf(p,"%f%n",&a,&n)) p+=n;      // skip possible initial numerical constant
      while (*p) {
         if (*p=='*' || *p=='/') {
            p_muldiv = p;
            n=0;
            p++;
            if (sscanf(p,"%f%n",&a,&n)) p+=n;   // skip possible numerical constant after * or / operator
         } else if (*p=='+' || *p=='-') {
            p_plusminus = p;
            n=0;
            p++;
            if (sscanf(p,"%f%n",&a,&n)) p+=n;   // skip possible numerical constant after + or - operator
         } else {
            p++;
         }
      }

      if (p_plusminus) p=p_plusminus;
      else p=p_muldiv;

   }

   if (p) {
      // if an operator was found:
      // split the expression at that point, and evaluate the halves recursively

      int unaryminus=0;
      i=*p;
      strcpy(s,expr);
      *p++=0;

      if (i=='-') {
         // check for unary minus: in that case the left argument is empty
         const char *q;
         q=s;
         while (isspace(*q)) q++;
         if (*q==0) unaryminus=1;
      }
      if (unaryminus) { recordstep(P_const); recordstep_f(0); }
      else eval(s,erroroffset0+0);   // evaluate the part to the left of the operator

      eval(p,erroroffset0+(p-s));    // evaluate the part to the right of the operator

      switch (i) {
         case '+': recordstep(P_add); break;
         case '-': recordstep(P_sub); break;
         case '*': recordstep(P_mul); break;
         case '/': recordstep(P_div); break;
      }
   } else {
      // if no operator was found, check whether there were parentheses:
      if (!ph) {
         // no parentheses: then we must have a numeric constant, or a reference to a variable, or nonsense...
         int v;
         float a;
         strcpy(s,expr);
         if ((p=strtok(s,sepchars))==NULL) Exits3("Missing argument",0,strlen(expr));         // the nonsense case
         if ((v=findvarnr(p))>=0) recordstep(P_readvar+v);     // reference to an existing variable           
         else if (sscanf(p,"%f",&a)==1) {                      // numeric constant
            recordstep(P_const); 
            recordstep_f(a); 
         } else {                                              // reference to a new variable
            v=newvarnr(p,p-s+erroroffset0); 
            recordstep(P_readvar+v); 
         }
         if ((p=strtok(NULL,sepchars))) Exits3("Missing operator",p-s-1,p-s);
         
      } else {
         // there were parentheses:
         int step=0; 
         int multiarg=0;
         char *ph0,*q;
         int nargs=0;
         strcpy(s,expr);
         ph0=strchr(s,'(');     // find the opening parenthesis; ph still points to closing parenthesis
         *ph0=0; *ph=0;         // remove both parentheses
         // check whether there is a word before the opening parenthesis:
         if ((p=strtok(s,sepchars))!=NULL) {
            // if so, try to recognize it as a function name:
            if (!strcasecmp(p,"sin")) step=P_sin;
            else if (!strcasecmp(p,"cos")) step=P_cos;
            else if (!strcasecmp(p,"tan")) step=P_tan;
            else if (!strcasecmp(p,"tanh")) step=P_tanh;
            else if (!strcasecmp(p,"sin1")) step=P_sin1;
            else if (!strcasecmp(p,"cos1")) step=P_cos1;            
            else if (!strcasecmp(p,"mod1")) step=P_mod1;
            else if (!strcasecmp(p,"abs")) step=P_abs;
            else if (!strcasecmp(p,"sqrt")) step=P_sqrt;
            else if (!strcasecmp(p,"pow")) { step=P_pow; multiarg=1; }
            else if (!strcasecmp(p,"fir")) { step=P_fir; multiarg=1; }
            else if (!strcasecmp(p,"firsymodd")) { step=P_fir+1; multiarg=1; }
            else if (!strcasecmp(p,"firsymeven")) { step=P_fir+2; multiarg=1; }
            else if (!strcasecmp(p,"biquad")) { step=P_biquad; multiarg=1; }
            else if (!strcasecmp(p,"round")) { step=P_round; multiarg=1; }
            else Exits3("Unknown function:",p-s,ph0-s);
         }
         // next, evaluate the part between parenthesis
         // first, check whether there are multiple arguments
         q=nextcomma(ph0+1);
         if (q) {
            if (!multiarg) {
               if (step) Exits3("Multiple arguments to single-argument function",ph0-s,ph-s)
               else Exits3("Comma not allowed here",q-s,q-s);
            } else {
               *q=0;
            }
         }
         // evaluate the (first) argument
         eval(ph0+1,erroroffset0+(ph0+1-s));

	 while (q) {
            char *qq=q+1;
            q=nextcomma(qq);
	    if (q) *q=0;
            eval(qq,erroroffset0+(ph0+1-s));
	    nargs++;
	 }

         // record this step
         if ((step&0xffff0000)==0) {
            if (step) recordstep(step);
         } else if ((step&0xffff0000)==P_fir) {
            int k=newfir();
            Fir *f=firs+k;
            f->symmetric=step&0xffff;
            if (nargs>=maxfirlen || (nargs*2>=maxfirlen && f->symmetric)) Exits3("FIR filter too long: ",ph0-s,ph-s);
	    f->len=nargs;
            recordstep(P_fir+k);
         } else if ((step&0xffff0000)==P_biquad) {
            int k;
            if (nargs!=5 && nargs!=6) Exits3("Biquad needs 5 or 6 coefficients",ph0-s,ph-s);
            k=newbiquad(nargs);
            recordstep(P_biquad+k);
         } else if ((step&0xffff0000)==P_round) {
            if (nargs!=2) Exits3("Round needs 2 arguments",ph0-s,ph-s);
            recordstep(P_round);
         } else if ((step&0xffff0000)==P_pow) {
            if (nargs!=2) Exits3("Pow need 2 arguments",ph0-s,ph-s);
            recordstep(P_pow);
         }
      }
   }
}



// eeval() compiles assignment statements, and print statements as a special case
int eeval(const char *s)
{
   char *p1;
   char *p;
   int v;
   char ss[Maxlinelen];
   int erroroffset0=0;

   // if an exception occurs in the try-block.. 
   // we catch it in the 'catch' block.. 
   try {
      if (strlen(s)>=Maxlinelen) Exits3("Line too long",0,strlen(s));
   
      strcpy(ss,s);

      // if it's an assignment statement, there must be an '=' sign
      p1=strchr(ss,'=');
   
      if (!p1) {
         p=ss;
         while (*p>0 && *p<=' ') p++;  
         if (*p==0) return 0;   // skip lines consisting of just whitespace
         if (strncasecmp(p,"print",5)==0 && isspace(p[5])) {
            // found a print statement: evaluate the rest of the string
            eval(p+6,p+6-ss);
            recordstep(P_print);
            return 0;
         }
         // else, report error:
         Exits5("Missing = sign");
      }

      // it is an assignment, then:
    
      // evaluate the part after '=' :
      eval(p1+1,p1+1-ss);
   
      // interpret the part before the '=' as the name of the variable to be assigned
      p=ss;
      while (isspace(*p)) p++;
      while (isspace(p1[-1])) p1--;
      *p1=0;
      if ((v=findvarnr(p))<0) v=newvarnr(p,p-ss);
      recordstep(P_writevar+v);
   
//      { int i; for (i=0;i<progsteps;i++) printf("%08x\n",prog[i].i); }
   
      return 0;
   }
   catch(ParserException e)
   {
      // if we end up here, there was a parse/compile error.
      if (e.erroroffset1==e.erroroffset2) { e.erroroffset1--; e.erroroffset2++; }
      if (e.erroroffset1<0) e.erroroffset1=0;
      if (e.erroroffset2>(int)strlen(s)) e.erroroffset2=strlen(s);
      erroroffset1=e.erroroffset1;
      erroroffset2=e.erroroffset2;
      errorstr=e.errorstr;
      return 1;
   }
}


//////////////////////////////////////////////////////////////////////////
// FIR filters for resampling between 8000 and 48000 Hz

float lpf_fir_coef[31] = {
  2.2140983E-4, -3.2178184E-5, -5.0303485E-4, -0.0011379705, -0.0017972591,
  -0.0022674825, -0.0023027621, -0.0016901116, -3.2620327E-4, 0.0017137628,
  0.004136959, 0.0064425073, 0.007989937, 0.008123389, 0.006333524,
  0.0024258716, -0.0033433076, -0.010202051, -0.016896976, -0.021837411,
  -0.02333654, -0.019914225, -0.010607534, 0.004772324, 0.025493862,
  0.04990995, 0.075626075, 0.09981363, 0.119619675, 0.13260402,
  0.137125
};


#define Maxrfirlen 128  // must be power of 2
#define RFirmask (Maxrfirlen-1)
typedef struct {
   int len;
   float *coef;
   int phase;
   float buf[Maxrfirlen];
} RFir;

RFir fir_out_l={61,lpf_fir_coef};
RFir fir_out_r={61,lpf_fir_coef};
RFir fir_in_l={61,lpf_fir_coef};
RFir fir_in_r={61,lpf_fir_coef};


void rfir_init(RFir *f)
{
   int i;
   f->phase=0;
   for (i=0;i<Maxrfirlen;i++) f->buf[i]=0;
}

float rfir(float in,RFir *f)
{
   int i,j,k;
   float out;
   f->buf[f->phase]=in;
   j=f->phase;
   k=(f->phase-f->len+1)&RFirmask;
   f->phase = (f->phase+1)&RFirmask;
   out=0;
   for (i=0;i<f->len/2;i++) {
      out+=f->coef[i]*(f->buf[j]+f->buf[k]);
      j = (j-1)&RFirmask;
      k = (k+1)&RFirmask;
   }
   if (j==k) out+=f->coef[i]*f->buf[j];
   return out;
}


//////////////////////////////////////////////////////////////////////////
// execution of the user's program

Variable *va_in=NULL;
Variable *va_inl=NULL;
Variable *va_inr=NULL;
Variable *va_out=NULL;
Variable *va_outl=NULL;
Variable *va_outr=NULL;

Variable *va_samplerate=NULL;


float var_to_gui[NUM_VAR_TO_GUI][VAR_TO_GUI_LEN];
char var_to_gui_varname[NUM_VAR_TO_GUI][maxvarnamelen+1];
Variable *var_to_gui_var[NUM_VAR_TO_GUI];

int var_to_gui_idx=0;
int var_to_gui_idx_prevpulse=0;
int var_to_gui_idx_prevprevpulse=0;

// Win32 fix: Visual C++ does not have a 'round' function in math.h
#ifdef WIN32
inline double round( double d )
{
	return floor( d + 0.5 );
}
#endif

// execute() executes the user's compiled program once
void execute(float inl,float inr,float *outl,float *outr)
{
   int i;     // program counter
   int j=0;   // stack pointer
   float stack[2048];

   if (va_in) va_in->val=(inl+inr)/2;
   if (va_inl) va_inl->val=inl;
   if (va_inr) va_inr->val=inr;
   
   for (i=0;i<progsteps;i++) {
      if (prog[i].i&0x80000000) {
         int n=prog[i].i&0xffff;
         switch (prog[i].i&0xff000000) {
            case P_readvar: stack[j++]=vars[n].val; break;
            case P_writevar: vars[n].val=stack[--j]; break;
            case P_fir: j-=execfir(n,stack+j); break;
            case P_biquad: j-=execbiquad(n, stack+j); break;
         }
      } else {
         switch (prog[i].i) {
            case P_add: j--; stack[j-1]+=stack[j]; break;
            case P_sub: j--; stack[j-1]-=stack[j]; break;
            case P_mul: j--; stack[j-1]*=stack[j]; break;
            case P_div: j--; stack[j-1]/=stack[j]; break;
            case P_sin: stack[j-1]=sin(stack[j-1]); break;
            case P_tan: stack[j-1]=tan(stack[j-1]); break;
            case P_tanh: stack[j-1]=tanh(stack[j-1]); break;
            case P_cos: stack[j-1]=cos(stack[j-1]); break;
            case P_sin1: stack[j-1]=sin(2*M_PI*stack[j-1]); break;
            case P_cos1: stack[j-1]=cos(2*M_PI*stack[j-1]); break;
            case P_const: stack[j++]=prog[i+1].f; i++; break;
            case P_print: printf("%f\n",stack[--j]); break;
            case P_mod1: stack[j-1]=stack[j-1]-(int)stack[j-1]; break;
            case P_abs: stack[j-1]=fabs(stack[j-1]); break;
            case P_sqrt: stack[j-1]=sqrt(stack[j-1]); break;
            case P_round: j--; if (stack[j]!=0) stack[j-1]=stack[j]*round(stack[j-1]/stack[j]); break;
            case P_pow: j--; stack[j-1]=pow(stack[j-1],stack[j]); break;
         }
      }
      if (j>2044) Exit("Stack overflow");
   }

   if (va_out) { *outl=va_out->val; *outr=va_out->val; }
   if (va_outl) *outl=va_outl->val; 
   if (va_outr) *outr=va_outr->val; 
   for (i=0;i<NUM_VAR_TO_GUI;i++)
      if (var_to_gui_var[i]) var_to_gui[i][var_to_gui_idx] = var_to_gui_var[i]->val;
   var_to_gui_idx = (var_to_gui_idx+1)&(VAR_TO_GUI_LEN-1);
}



int resample_8_48=1;

// process_samples() executes the user's program for nsamp samples
void process_samples(short int buf_in[], short int buf_out[],int nsamp)
{
   static float inl_dc=0, inr_dc=0;
   static float outl=0,outr=0;
   float inl,inr;
   int j;
   static int resample_phase=0;

   float labs, lmax=0, rabs, rmax=0;
   for (j=0;j<nsamp;j++) {
      inl=((float)buf_in[j*2+0])/32767;
      inr=((float)buf_in[j*2+1])/32767;

      if (remove_dc)
      {
        inl_dc = 1e-30 + 0.999*inl_dc + 0.001*inl;
        inr_dc = 1e-30 + 0.999*inr_dc + 0.001*inr;
        inl-= inl_dc;
        inr-= inr_dc;
      }

      if (resample_8_48) {
         inl = rfir(inl,&fir_in_l);
         inr = rfir(inr,&fir_in_r);
      }

      if (resample_phase==0) {
         switch(inputsource) 
         {
         case INPUT_SINE:
            inl=inr=sin(sweep_phase*2.0*M_PI);
            sweep_phase+=sweep_freq/virtsamplerate;
            if (sweep_phase>=1) sweep_phase--;
            sweep_freq+=sweeprate/virtsamplerate;
            if (sweep_freq>virtsamplerate/2) sweep_freq=100;
            break;
         case INPUT_QUADSINE:
            inl=cos(sweep_phase*2.0*M_PI);
            inr=sin(sweep_phase*2.0*M_PI);
            sweep_phase+=sweep_freq/virtsamplerate;
            if (sweep_phase>=1) sweep_phase--;
            sweep_freq+=sweeprate/virtsamplerate;
            if (sweep_freq>virtsamplerate/2) sweep_freq=100;
            break;
         case INPUT_NOISE:
            {
               int i;
               inl=0;
               for (i=0;i<12;i++) inl+=((float)rand()/RAND_MAX-0.5)/6;
               inr=inl;
            }
            break;
         case INPUT_WAVFILE:
            {
               float fl_buffer[2];
               wavstreamer.FillBuffer(fl_buffer, 1);
               inl = fl_buffer[0];
               inr = fl_buffer[1];
            }
            break;
         case INPUT_IMPULSE:
            if (--impulse_phase<=0) {
               inl=inr=1;
               impulse_phase=(int)round(virtsamplerate/10);    // hard-coded 10 Hz impulse rate
               if (impulse_phase>VAR_TO_GUI_LEN/2) impulse_phase=VAR_TO_GUI_LEN/2;   // however, if needed increase the impulse rate such that at least two impulses fit in the buffer
               var_to_gui_idx_prevprevpulse = var_to_gui_idx_prevpulse;
               var_to_gui_idx_prevpulse = var_to_gui_idx;
            } else inl=inr=0;
            break;
         default:
            inl=((float)buf_in[j*2+0])/32767;
            inr=((float)buf_in[j*2+1])/32767;
            break;
         }

         if (!stopped) execute(inl,inr,&outl,&outr);

         labs = fabs(outl); if (labs>lmax) lmax=labs;
         rabs = fabs(outr); if (rabs>rmax) rmax=rabs;

/*
         var_to_gui_l[var_to_gui_idx] = outl/2;
         var_to_gui_r[var_to_gui_idx] = outr/2;
         var_to_gui_idx = (var_to_gui_idx+1)&(VAR_TO_GUI_LEN-1);
*/
      
         if (outl>1) outl=1;
         if (outl<-1) outl=-1;
         if (outr>1) outr=1;
         if (outr<-1) outr=-1;

      }

      if (!resample_8_48) {
         buf_out[j*2+0]=(int)(outl*32767);
         buf_out[j*2+1]=(int)(outr*32767);
         resample_phase=0;
      } else {
         resample_phase = (resample_phase+1)%6;
         buf_out[j*2+0]=(int)(rfir(outl,&fir_out_l)*32767);
         buf_out[j*2+1]=(int)(rfir(outr,&fir_out_r)*32767);
      }
   }

   if (va_samplerate && virtsamplerate!=va_samplerate->val)
      virtsamplerate = va_samplerate->val;

   if (virtsamplerate!=prev_virtsamplerate) {
      prev_virtsamplerate=virtsamplerate;
      if (virtsamplerate==8000) {
         samplerate=48000;
         resample_8_48=1;
      } else {
         samplerate=virtsamplerate;
         resample_8_48=0;
      }
   }

   left_channel_max = lmax;
   right_channel_max = rmax;
}


int compile_one_line(const char *s)
{
   if (!strchr("#;'`",s[0])) return eeval(s);
   return 0;
}

void compile_start(void)
{
   stopped=1;
   progsteps=0;
   nxtvar=0;
   nxtfir=0;
   nxtbiquad=0;
}

void compile_complete(void)
{
   int i;
   char s[16];

   Variable *va;

   va_in=findvar("in");

   va_inl=findvar("inl");
   if (!va_inl) va_inl=findvar("ini");

   va_inr=findvar("inr");
   if (!va_inr) va_inr=findvar("inq");

   va_out=findvar("uit");
   if (!va_out) va_out=findvar("out");

   va_outl=findvar("uitl");
   if (!va_outl) va_outl=findvar("outl");
   if (!va_outl) va_outl=findvar("uiti");
   if (!va_outl) va_outl=findvar("outi");

   va_outr=findvar("uitr");
   if (!va_outr) va_outr=findvar("outr");
   if (!va_outr) va_outr=findvar("uitq");
   if (!va_outr) va_outr=findvar("outq");

   va_samplerate=findvar("samplerate");

   if (va_samplerate) va_samplerate->val=virtsamplerate;

   for (i=1;i<NUMSLIDERS;i++) {
      sprintf(s,"slider%i",i);
      va=findvar(s); if (va) sliderptr[i]=&va->val; else sliderptr[i]=NULL;
   }

   for (i=0;i<NUM_VAR_TO_GUI;i++) {
      var_to_gui_setvar(i, var_to_gui_varname[i]);
   }

   rfir_init(&fir_out_l);
   rfir_init(&fir_out_r);
   rfir_init(&fir_in_l);
   rfir_init(&fir_in_r);

   stopped=0;
}

void core_stop(void)
{
   stopped=1;
}

void var_to_gui_setvar(int i,const char *varname)
{
   strncpy(var_to_gui_varname[i],varname,maxvarnamelen);
   var_to_gui_var[i]=findvar(var_to_gui_varname[i]);
   if (!var_to_gui_var[i]) {
      if (!strcmp(varname,"uitl") || !strcmp(varname,"outl")) {
         if (va_outl) var_to_gui_var[i]=va_outl;
         else var_to_gui_var[i]=va_out;
      }
      if (!strcmp(varname,"uitr") || !strcmp(varname,"outr")) {
         if (va_outr) var_to_gui_var[i]=va_outr;
         else var_to_gui_var[i]=va_out;
      }
      if (!var_to_gui_var[i]) 
         var_to_gui_var[i] = &var_zero;
   }
}

