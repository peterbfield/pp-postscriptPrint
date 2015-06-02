#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#define FLG_NOEND       0x01
#define FLG_REQ         0x02
#define MAXTABLE	2048
#define MAXFLOAT	100
#define STRINGSIZE	500
#define SMALLSTRINGSIZE 100
#define VECSIZE		10000
#define NAMEDELIM	0x01
#define PARTIAL         0
#define HEADER          1
#define FULL            2

typedef struct 
{
    char		*from;
    char		*to;
} tTranslate;

#include "tree.h"
#include "prototypes.h"
