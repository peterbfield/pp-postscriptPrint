#ifdef TRACE

#ifndef _TRACES_H

#define _TRACES_H

extern int trace_1;
extern int trace_2;
extern int trace_3;
extern int trace_4;
extern int trace_5;
extern int trace_6;
extern int trace_7;
extern int trace_8;
extern int trace_9;
extern int trace_lmt;
extern int trace_debugger;
extern int trace_alloc;
extern int trace_ff;
extern int trace_data;
extern int trace_misc;
extern int trace_locks;
extern int trace_lc;			/* file locking and list control	*/
extern int trace_error;
extern int trace_memory;
extern int trace_callbacks;

								/* hnj traces */
extern int nf_traces;
extern int stf_trace;
extern int flow_trace;
extern int hnj_trace;
extern int jfi_trace;
extern int debugger_trace;
extern int hget_trace;
extern int hyp_trace;
extern int var_trace;
extern int prepage_trace;
extern int eq_trace;

#endif 

#endif
