#ifndef _LPM_H
#define _LPM_H

#define LPMfloat 1

#define MAXMSGLEN				1024
#define LPM_MAXNAME 			128
#define LPM_MAXMESG 			MAXMSGLEN

#define LPM_Feature				1
#define LPM_Option				2

#define LPM_DesignMaster		0
#define LPM_EditMaster  		1
#define LPM_EditMasterNet  		2
#define LPM_MaxFeature			3
#define LPM_FloatingKey			(1<<30)

#define LPM_ColorMaster			0
#define LPM_CropAndScaleSite	1
#define LPM_OPISite				2
#define LPM_MathMaster			3
#define LPM_MasterPage			4
#define LPM_PentaExpressSite	5
#define LPM_SearchMaster		6
#define LPM_SpellCheck			7
#define LPM_KernMaster			8
#define LPM_SGML				9
#define LPM_SGMLToolkit			9
#define LPM_StyleTags			10
#define LPM_EditTrace			11
#define LPM_ImportLANSite		12
#define LPM_ImpoStrip			13
#define LPM_GatorBoxMac300		14
#define LPM_PWS1				15
#define LPM_PWS2				16
#define	LPM_SciTexSite			17
#define	LPM_KernMath			18
#define	LPM_AutoTabSite			19
#define	LPM_SGMLPub				20
#define	LPM_Rev14				21
#define LPM_HypCanadian			52
#define LPM_HypDanish			53
#define LPM_HypDutch			54
#define LPM_HypDutch2			55
#define LPM_HypEnglish			56
#define LPM_HypFrench			57
#define LPM_HypGerman			58
#define LPM_HypItalian			59
#define LPM_HypPortuguese		60
#define LPM_HypSpanish			61
#define LPM_HypSwedish			62
#define LPM_MaxOption			63

extern char lpm_errmsg[LPM_MAXMESG];

/* verify that lpm is running */
extern int lpm_init (void);

/* submit command to program at default (5) priority */
extern int lpm_que (char *program, char *command);

/* submit command to program with priority */
extern int lpm_pque (int priority, char *program, char *command);

/* notify lpm that program [id] completed
 * a command submittied using lpm_que().
 * xval is completion status.
 */
extern int lpm_done (int id, int xval);

/* get a pending message from lpm */
extern char *lpm_getmsg ();

/* send message from process [pid] to specified host */
extern int lpm_sendmsg (char *host, int pid, char *message);

/* get status of process [pid] from specified host
 * return: 0=process is still running; 1=process has died.
 */
extern int lpm_checkpid (char *host, int pid);

/* notify lpm of client's exit */
extern int lpm_bye (void);

/* request key for feature [opt], type = LPM_Feature
 * confirm avaibility of option [opt], type = LPM_Option
 */
extern unsigned int lpm_query (int opt, int type);
#define lpm_feature(feature)	lpm_query ((feature), LPM_Feature)
#define lpm_option(option)		lpm_query ((option), LPM_Option)

/* parse message retrieved using lpm_getmsg() from command proc */
extern int lpm_retval (int *retval, char *msg, char *proc);

/* ask lpm to kill program */
extern int lpm_preset (char *host, char *prog, int wait);

/* verify lpm license(s) still valid -- see lc_utils.c */
extern int lpm_verify (void);

#endif
