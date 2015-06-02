#ifndef _COLOR_H

#define _COLOR_H

#include "llist.h"

#define NO_COLOR "none"

#define NOT_ALLOC -1

#define NUM_PLANES 7

#define MAX_COLOR 65535

#define MAX_COLOR_LABEL 60
#define MAX_COLOR_NAME 41 /* Should agree with MaxColname+1 in color_ed.uil */
#define MAX_SHAPE      20
#define HUNDR_PER      100
#define MAX_AI         20
#define SPOT "spot"
#define PROCESS "process"
#define tTRUE "true"
#define tFALSE "false"
#define SPOT_VALUE 0
#define PROC_VALUE 1
#define MAX_AV_COLORS 256
#define CRIT_PLANES   8 /* Critical Number of planes;
   We should use different schemes if we have more than
   CRIT_PLANES planes */

typedef struct {
	int color_num;
	char shape[MAX_SHAPE];
	int plate;
	int angle;
	int screen;
	unsigned short cyan;
	unsigned short magenta;
	unsigned short yellow;
	unsigned short black;
	float percent;
	int tint;
	char name[MAX_COLOR_NAME];
	unsigned int type;
	unsigned int overprint;
	LLIST *plates;
} COLOR_INFO;

typedef struct {
	unsigned short cyan;
	unsigned short magenta;
	unsigned short yellow;
	unsigned short black;
	char name[MAX_COLOR_NAME];
} PAL_COLOR_INFO;

typedef struct {
	int plate_num;
	char shape[MAX_SHAPE];
	int angle;
	int screen;
	float percent;
} ct_plate;

typedef struct {
	int plate_num;
	char plate_name[MAX_COLOR_NAME];
} ct_platename;

typedef struct {
	int plate_num;
	char *colors;
} ct_aisep_custom;

typedef struct {
	int plate_num;
	int type;
} ct_aisep;

typedef struct {
	int gfgcolor;
	int gbgcolor;
	int gbwoverprint;
	LLIST *plate_names;   /* List of ct_platenum keeping plate labels */
	LLIST *ListofColors;  /* List of COLOR_INFO keeping colors        */
	LLIST *AI_Sep;        /* List of ct_aisep keeping process type    */
	LLIST *AI_Sep_CUSTOM_COLORS; /* List of ct_aisep_custom keeping custom colors */
} C_TABLE;

#endif
