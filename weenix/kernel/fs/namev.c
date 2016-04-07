/******************************************************************************/
/* Important Fall 2015 CSCI 402 usage information:                            */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

#include "kernel.h"
#include "globals.h"
#include "types.h"
#include "errno.h"

#include "util/string.h"
#include "util/printf.h"
#include "util/debug.h"

#include "fs/dirent.h"
#include "fs/fcntl.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"

/* This takes a base 'dir', a 'name', its 'len', and a result vnode.
 * Most of the work should be done by the vnode's implementation
 * specific lookup() function, but you may want to special case
 * "." and/or ".." here depnding on your implementation.
 *
 * If dir has no lookup(), return -ENOTDIR.
 *
 * Note: returns with the vnode refcount on *result incremented.
 */
int
lookup(vnode_t *dir, const char *name, size_t len, vnode_t **result)
{
        /*NOT_YET_IMPLEMENTED("VFS: lookup");*/
	
	dbg(DBG_PRINT,"Entering namev.c->lookup\n");
	
	KASSERT(NULL != dir);
	dbg(DBG_PRINT,"(GRADING2A 2.a) dir = %ld\n)",(long)dir->vn_vno);	

	KASSERT(NULL != name);
	dbg(DBG_PRINT,"(GRADING2A 2.a) name = %s\n",name);


	KASSERT(NULL != result);
	dbg(DBG_PRINT,"(GRADING2A 2.a) result is not null \n");

	if(dir->vn_ops->lookup==NULL || !S_ISDIR(dir->vn_mode))
	{
		dbg(DBG_PRINT,"Not a directory as lookup() null or S_ISDIR null\n");
		return -ENOTDIR;
	}	

	if(len > NAME_LEN)
	{
		dbg(DBG_ERROR,"The filename is too long.\n");
		return -ENAMETOOLONG;
	}

	/* this condition could be added later*/
	/*if(len==0)
	{
		*result=vget(dir->vn_fs,dir->vn_vno);
		dbg(DBG_PRINT,"\n*result set as len=0");
		return 0;
	}
	*/	

	
	return dir->vn_ops->lookup(dir,name,len,result);

	dbg(DBG_PRINT,"Exiting namev.c->lookup\n");

        return 0;
}


/* When successful this function returns data in the following "out"-arguments:
 *  o res_vnode: the vnode of the parent directory of "name"
 *  o name: the `basename' (the element of the pathname)
 *  o namelen: the length of the basename
 *
 * For example: dir_namev("/s5fs/bin/ls", &namelen, &name, NULL,
 * &res_vnode) would put 2 in namelen, "ls" in name, and a pointer to the
 * vnode corresponding to "/s5fs/bin" in res_vnode.
 *
 * The "base" argument defines where we start resolving the path from:
 * A base value of NULL means to use the process's current working directory,
 * curproc->p_cwd.  If pathname[0] == '/', ignore base and start with
 * vfs_root_vn.  dir_namev() should call lookup() to take care of resolving each
 * piece of the pathname.
 *
 * Note: A successful call to this causes vnode refcount on *res_vnode to
 * be incremented.
 */
int
dir_namev(const char *pathname, size_t *namelen, const char **name,
          vnode_t *base, vnode_t **res_vnode)
{
       /* NOT_YET_IMPLEMENTED("VFS: dir_namev");*/
	dbg(DBG_PRINT,"Entering namev.c->dir_namev\n");
	
	KASSERT(NULL != pathname);
	dbg(DBG_PRINT,"(GRADING2A 2.b) pathname = %s\n",pathname);
	KASSERT(NULL != namelen);	
	dbg(DBG_PRINT,"(GRADING2A 2.b) namelen = not NULL\n",);
	KASSERT(NULL != name);
	dbg(DBG_PRINT,"(GRADING2A 2.b) name not NULL\n");
	KASSERT(NULL != res_vnode);
	dbg(DBG_PRINT,"(GRADING2A 2.b) res_vnode not NULL\n");
	
	if(base==NULL)
		base=curproc->p_cwd;

	if(pathname[0]=='/')
		base=vfs_root_vn;
	
	int ptr=0,i=0;
	
	if(pathname[0]=='/')
		ptr++;	 
		
	while(1)
	{
		for(i=0;pathname[ptr+i]!='/';i++)
		{
			if(pathname[ptr+i]=='\0')
			{
				if (i > NAME_LEN)
				{
					dbg(DBG_PRINT, "The filename is too long.\n");
					return -ENAMETOOLONG;
				}
				
				*res_vnode=vget(base->vn_fs,base->vn_vno);
				
				KASSERT(NULL != *res_vnode);
				dbg(DBG_PRINT,"(GRADING2A 2.b) pointer to vnode(res_vnode) not NULL\n");
				*name=&pathname[ptr];
				*namelen=i;
				return 0;			
				

				/*dbg(DBG_PRINT,"");*/	
			}
		}
		
		/*Do a lookup for the individual v_nodes of the path*/
		
		int errcode;
		errcode=lookup(base,&pathname[ptr],i,res_vnode);	
		
		if(errcode<0)
			return errcode;

		/*if lookup returns 0	*/	
		vput(*res_vnode);		
		base=*res_vnode;
		ptr+=i+1;
				

	}
	
	dbg(DBG_PRINT,"Exiting namev.c->dir_namev\n");

        return 0;
}

/* This returns in res_vnode the vnode requested by the other parameters.
 * It makes use of dir_namev and lookup to find the specified vnode (if it
 * exists).  flag is right out of the parameters to open(2); see
 * <weenix/fcntl.h>.  If the O_CREAT flag is specified, and the file does
 * not exist call create() in the parent directory vnode.
 *
 * Note: Increments vnode refcount on *res_vnode.
 */
int
open_namev(const char *pathname, int flag, vnode_t **res_vnode, vnode_t *base)
{
        /*NOT_YET_IMPLEMENTED("VFS: open_namev");*/

	dbg(DBG_PRINT,"Entering namev.c->open_namev\n");
	
	dbg(DBG_PRINT,"open_namev: pathname = %s\n",pathname);
	
	if(strlen(pathname) <= 0)
	{
	dbg(DBG_PRINT,"open_namev(): PathName is too short\n");
	return -EINVAL;
	}
		
	if(strlen(pathname) > MAXPATHLEN)
	{
	dbg(DBG_PRINT,"open_namev(): PathName is too long\n");
	return -ENAMETOOLONG;
	}
	

	int retvar; 
	const char *name = NULL;
	size_t len = 0;

	/*call dri_namev to get path in res_vnode*/
	retvar= dir_namev(pathname,&len,&name,base,res_vnode);
		
	vput(*res_vnode);

	/*lookup on that path*/
	retvar=lookup(*res_vnode,name,len,res_vnode);

	if(retvar==-ENOENT && flag==O_CREAT)
	{
		KASSERT(NULL != (*res_vnode)->vn_ops->create);
		dbg(DBG_PRINT,"(GRADING2A 2.c) *res_vnode->vn_ops->create is not NULL\n");
		retvar = (*res_vnode)->vn_ops->create(*res_vnode,name,len,res_vnode);
		return retvar;
	}
	

	dbg(DBG_PRINT,"Exiting namev.c->open_namev\n");
	
        return 0;
}

#ifdef __GETCWD__
/* Finds the name of 'entry' in the directory 'dir'. The name is writen
 * to the given buffer. On success 0 is returned. If 'dir' does not
 * contain 'entry' then -ENOENT is returned. If the given buffer cannot
 * hold the result then it is filled with as many characters as possible
 * and a null terminator, -ERANGE is returned.
 *
 * Files can be uniquely identified within a file system by their
 * inode numbers. */
int
lookup_name(vnode_t *dir, vnode_t *entry, char *buf, size_t size)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_name");
        return -ENOENT;
}


/* Used to find the absolute path of the directory 'dir'. Since
 * directories cannot have more than one link there is always
 * a unique solution. The path is writen to the given buffer.
 * On success 0 is returned. On error this function returns a
 * negative error code. See the man page for getcwd(3) for
 * possible errors. Even if an error code is returned the buffer
 * will be filled with a valid string which has some partial
 * information about the wanted path. */
ssize_t
lookup_dirpath(vnode_t *dir, char *buf, size_t osize)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_dirpath");

        return -ENOENT;
}
#endif /* __GETCWD__ */
