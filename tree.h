/*
**
** $Log: tree.h,v $
** Revision 17.0  2010/05/15 20:53:59  peter
** new rev
**
** Revision 16.0  2006/01/17 14:02:00  peter
** New rev
**
** Revision 15.0  2005/03/23 12:34:27  peter
** New rev
**
** Revision 14.0  2002/03/01 11:34:40  pete
** New rev
**
** Revision 1.1  2002/02/12 20:37:42  dee
** Initial revision
**
 * Revision 1.1  1996/03/15  13:07:34  rjc
 * Initial revision
 *
 * Revision 1.1  1996/02/15  05:48:03  rjc
 * Initial revision
 *
 * Revision 1.2  1996/02/03  04:24:07  rjc
 * Paramiterize Makefile to allow for different platforms, and fill in for
 * Linux(tested) and SCO(untested)
 *
 * Revision 1.1  1996/02/01  21:50:  rjc
 * Initial revision
 *
**
*/

#ifndef	_TREE_FLAG
#define	_TREE_FLAG


#if defined(__STDC__) || defined(__GNUC__)
typedef	void *	tree_t;
#if !defined(__P)
#define __P(x) x
#endif
#else
typedef	char *	tree_t;
#if !defined(__P)
#define	__P(x) ()
#endif
#endif

typedef	struct	tree_s
{
      struct tree_s	*tree_l;
      struct tree_s	*tree_r;
      short		tree_b;
      tree_t		tree_p;
} tree;


void 	tree_init	__P( (tree **) );
tree_t	tree_srch	__P( (tree **, int (*)(), tree_t) );
void	tree_add	__P( (tree **, int (*)(), tree_t, void (*)()) );
int	tree_delete	__P( (tree **, int (*)(), tree_t, void (*)()) );
int	tree_trav	__P( (tree **, int (*)()) );
void	tree_mung	__P( (tree **, void (*)()) );

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif
#endif	/* _TREE_FLAG */
