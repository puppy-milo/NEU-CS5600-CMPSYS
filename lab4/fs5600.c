/*
 * file:        fs5600.c
 * description: skeleton file for CS 5600 system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Peter Desnoyers, November 2019
 *
 * Modified by CS5600 staff, fall 2021.
 */

#define FUSE_USE_VERSION 27
#define _FILE_OFFSET_BITS 64

#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fuse.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include "fs5600.h"

/* if you don't understand why you can't use these system calls here,
 * you need to read the assignment description another time
 */
#define stat(a,b) error do not use stat()
#define open(a,b) error do not use open()
#define read(a,b,c) error do not use read()
#define write(a,b,c) error do not use write()


// === block manipulation functions ===

/* disk access.
 * All access is in terms of 4KB blocks; read and
 * write functions return 0 (success) or -EIO.
 *
 * read/write "nblks" blocks of data
 *   starting from block id "lba"
 *   to/from memory "buf".
 *     (see implementations in misc.c)
 */
extern int block_read(void *buf, int lba, int nblks);
extern int block_write(void *buf, int lba, int nblks);

/* bitmap functions
 */
void bit_set(unsigned char *map, int i)
{
    map[i/8] |= (1 << (i%8));
}
void bit_clear(unsigned char *map, int i)
{
    map[i/8] &= ~(1 << (i%8));
}
int bit_test(unsigned char *map, int i)
{
    return map[i/8] & (1 << (i%8));
}


/*
 * Allocate a free block from the disk.
 *
 * success - return free block number
 * no free block - return -ENOSPC
 *
 * hint:
 *   - bit_set/bit_test might be useful.
 */
int alloc_blk() {
    /* Your code here */
    unsigned char bitmap[FS_BLOCK_SIZE];
    block_read(bitmap, 1, 1);
    for (int i = 3; i < FS_BLOCK_SIZE * 8; i++) {
        if (!bit_test(bitmap, i)) {
            bit_set(bitmap, i);
            block_write(bitmap, 1, 1);
            return i;
        }
    }
    return -ENOSPC;
}

/*
 * Return a block to disk, which can be used later.
 *
 * hint:
 *   - bit_clear might be useful.
 */
void free_blk(int i) {
    /* your code here*/
    unsigned char bitmap[FS_BLOCK_SIZE];
    block_read(bitmap, 1, 1);
    bit_clear(bitmap, i);
    block_write(bitmap, 1, 1);
}


// === FS helper functions ===


/* Two notes on path translation:
 *
 * (1) translation errors:
 *
 *   In addition to the method-specific errors listed below, almost
 *   every method can return one of the following errors if it fails to
 *   locate a file or directory corresponding to a specified path.
 *
 *   ENOENT - a component of the path doesn't exist.
 *   ENOTDIR - an intermediate component of the path (e.g. 'b' in
 *             /a/b/c) is not a directory
 *
 * (2) note on splitting the 'path' variable:
 *
 *   the value passed in by the FUSE framework is declared as 'const',
 *   which means you can't modify it. The standard mechanisms for
 *   splitting strings in C (strtok, strsep) modify the string in place,
 *   so you have to copy the string and then free the copy when you're
 *   done. One way of doing this:
 *
 *      char *_path = strdup(path);
 *      int inum = ... // translate _path to inode number
 *      free(_path);
 */


/* EXERCISE 2:
 * convert path into inode number.
 *
 * how?
 *  - first split the path into directory and file names
 *  - then, start from the root inode (which inode/block number is that?)
 *  - then, walk the dirs to find the final file or dir.
 *    when walking:
 *      -- how do I know if an inode is a dir or file? (hint: mode)
 *      -- what should I do if something goes wrong? (hint: read the above note about errors)
 *      -- how many dir entries in one inode? (hint: read Lab4 instructions about directory inode)
 *
 * hints:
 *  - you can safely assume the max depth of nested dirs is 10
 *  - a bunch of string functions may be useful (e.g., "strtok", "strsep", "strcmp")
 *  - "block_read" may be useful.
 *  - "S_ISDIR" may be useful. (what is this? read Lab4 instructions or "man inode")
 *
 * programing hints:
 *  - there are several functionalities that you will reuse; it's better to
 *  implement them in other functions.
 */
int sep_path(char *path, char **hierpath, char *delim, int max_levels) {
    int i = 0;
    char *token = strtok(path, delim);
    if (token == 0) return i;
    while (token && i < max_levels) { *(hierpath+i++) = token; token = strtok(0, delim); }
    if (token && i == max_levels) { return -1; }
    return i;
}

int par_path(char **hierpath, const char *src, char *par, int levels) {
    int len = 0, i = 0;
    while (i < levels - 1) { len += strlen(*(hierpath+i))+1; i++; }
    strncpy(par, src, len);
    par[len++] = '/';
    par[len] = '\0';
    return len;
}

dirent_t* find_dirent(unsigned char *buf, char *dst) {
    dirent_t *dirent = (dirent_t *)buf;
    while (dirent < (dirent_t *)(buf + FS_BLOCK_SIZE)) {
        if (dirent->valid && strcmp(dirent->name, dst) == 0) break;
        dirent++;
    }
    if (dirent >= (dirent_t *)(buf + FS_BLOCK_SIZE))
        return NULL;
    return dirent;
}

dirent_t* find_empty(unsigned char *buf) {
    dirent_t *dirent = (dirent_t *)buf;
    while (dirent < (dirent_t *)(buf + FS_BLOCK_SIZE)) {
        if (dirent->valid == 0) break;
        dirent++;
    }
    if (dirent >= (dirent_t *)(buf + FS_BLOCK_SIZE))
        return NULL;
    return dirent;
}

dirent_t* find_full(unsigned char *buf) {
    dirent_t *dirent = (dirent_t *)buf;
    while (dirent < (dirent_t *)(buf + FS_BLOCK_SIZE)) {
        if (dirent->valid == 1) break;
        dirent++;
    }
    if (dirent >= (dirent_t *)(buf + FS_BLOCK_SIZE))
        return NULL;
    return dirent;
}

int path2inum(const char *path) {
    /* your code here */
    int inode, nests, walk_nest, par_inode, par_data_inode;
    char *_path = strdup(path);
    char *hierpath[10] = {0};
    inode_t par_in, par_data_in;
    dirent_t *dirent;

    if (strlen(path) == 0 || path[0] != '/') { inode = -ENOENT; goto done; }

    nests = sep_path(_path, hierpath, "/", 10);
    if (nests < 0) { inode = -ENOENT; goto done; }
    if (nests == 0) { inode = 2; goto done; }
    
    walk_nest = 0;
    par_inode = 2;
    while (walk_nest < nests) {
        if (strlen(hierpath[walk_nest]) > 27) { inode = -ENOENT; goto done; }
        block_read((unsigned char *)&par_in, par_inode, 1);
        if (!S_ISDIR(par_in.mode) || par_in.size == 0) { inode = -ENOTDIR; goto done; }

        par_data_inode = par_in.ptrs[0];
        block_read((unsigned char *)&par_data_in, par_data_inode, 1);
        dirent = find_dirent((unsigned char *)&par_data_in, hierpath[walk_nest]);
        if (dirent == 0) { inode = -ENOENT; goto done; }

        par_inode = dirent->inode;
        walk_nest++;
    }
    inode = par_inode;

done:
    free(_path);
    return inode;
}


/* EXERCISE 2:
 * Helper function:
 *   copy the information in an inode to struct stat
 *   (see its definition below, and the full Linux definition in "man lstat".)
 *
 *  struct stat {
 *        ino_t     st_ino;         // Inode number
 *        mode_t    st_mode;        // File type and mode
 *        nlink_t   st_nlink;       // Number of hard links
 *        uid_t     st_uid;         // User ID of owner
 *        gid_t     st_gid;         // Group ID of owner
 *        off_t     st_size;        // Total size, in bytes
 *        blkcnt_t  st_blocks;      // Number of blocks allocated
 *                                  // (note: block size is FS_BLOCK_SIZE;
 *                                  // and this number is an int which should be round up)
 *
 *        struct timespec st_atim;  // Time of last access
 *        struct timespec st_mtim;  // Time of last modification
 *        struct timespec st_ctim;  // Time of last status change
 *    };
 *
 *  [hints:
 *
 *    - what you should do is mostly copy.
 *
 *    - read fs_inode in fs5600.h and compare with struct stat.
 *
 *    - you can safely treat the types "ino_t", "mode_t", "nlink_t", "uid_t"
 *      "gid_t", "off_t", "blkcnt_t" as "unit32_t" in this lab.
 *
 *    - read "man clock_gettime" to see "struct timespec" definition
 *
 *    - the above "struct stat" does not show all attributes, but we don't care
 *      the rest attributes.
 *
 *    - for several fields in 'struct stat' there is no corresponding
 *    information in our file system:
 *      -- st_nlink - always set it to 1  (recall that fs5600 doesn't support links)
 *      -- st_atime - set to same value as st_mtime
 *  ]
 */

void inode2stat(struct stat *sb, struct fs_inode *in, uint32_t inode_num)
{
    memset(sb, 0, sizeof(*sb));

    /* your code here */
    sb->st_ino = inode_num;
    sb->st_mode = in->mode;
    sb->st_nlink = 1;
    sb->st_uid = in->uid;
    sb->st_gid = in->gid;
    sb->st_size = in->size;

    if (S_ISDIR(in->mode))
        sb->st_blocks = 1;
    else
        sb->st_blocks = DIV_ROUND_UP(in->size, FS_BLOCK_SIZE);
    
    sb->st_atim.tv_sec = in->ctime;
    sb->st_mtim.tv_sec = in->mtime;
    sb->st_ctim.tv_sec = in->ctime;
}




// ====== FUSE APIs ========

/* EXERCISE 1:
 * init - this is called once by the FUSE framework at startup.
 *
 * The function should:
 *   - read superblock
 *   - check if the magic number matches FS_MAGIC
 *   - initialize whatever in-memory data structure your fs5600 needs
 *     (you may come back later when requiring new data structures)
 *
 * notes:
 *   - ignore the 'conn' argument.
 *   - use "block_read" to read data (if you don't know how it works, read its
 *     implementation in misc.c)
 *   - if there is an error, exit(1)
 */
void* fs_init(struct fuse_conn_info *conn)
{
    /* your code here */
    unsigned char buf[FS_BLOCK_SIZE];

    if (block_read(buf, 0, 1) != 0) exit(1);
    
    uint32_t *magic = (uint32_t *)buf;
    if (*magic != FS_MAGIC) exit(1);
    
    return NULL;
}


/* EXERCISE 1:
 * statfs - get file system statistics
 * see 'man 2 statfs' for description of 'struct statvfs'.
 * Errors - none. Needs to work.
 */
int fs_statfs(const char *path, struct statvfs *st)
{
    /* needs to return the following fields (ignore others):
     *   [DONE] f_bsize = FS_BLOCK_SIZE
     *   [DONE] f_namemax = <whatever your max namelength is>
     *   [TODO] f_blocks = total image - (superblock + block map)
     *   [TODO] f_bfree = f_blocks - blocks used
     *   [TODO] f_bavail = f_bfree
     *
     * it's okay to calculate this dynamically on the rare occasions
     * when this function is called.
     */

    st->f_bsize = FS_BLOCK_SIZE;
    st->f_namemax = 27;  // why? see fs5600.h

    /* your code here */
    unsigned char super[FS_BLOCK_SIZE], bitmap[FS_BLOCK_SIZE];
    int ret_code = 0;

    if (block_read(super, 0, 1) != 0) { ret_code = -EIO; goto done; }
    if (block_read(bitmap, 1, 1) != 0) { ret_code = -EIO; goto done; }
    
    super_t *super_block = (super_t *)super;
    st->f_blocks = super_block->disk_size - 2;

    uint32_t used_blocks = 0;
    for (int i = 2; i < FS_BLOCK_SIZE * 8; i++) {
        if (bit_test(bitmap, i)) used_blocks++;
    }
    st->f_bfree = st->f_blocks - used_blocks;
    st->f_bavail = st->f_bfree;

done:
    return ret_code;
    // return -EOPNOTSUPP;
}


/* EXERCISE 2:
 * getattr - get file or directory attributes. For a description of
 *  the fields in 'struct stat', read 'man 2 stat'.
 *
 * You should:
 *  1. parse the path given by "const char * path",
 *     find the inode of the specified file,
 *       [note: you should implement the helfer function "path2inum"
 *       and use it.]
 *  2. copy inode's information to "struct stat",
 *       [note: you should implement the helper function "inode2stat"
 *       and use it.]
 *  3. and return:
 *     ** success - return 0
 *     ** errors - path translation, ENOENT
 */


int fs_getattr(const char *path, struct stat *sb)
{
    /* your code here */
    int ret_code = 0, inode = path2inum(path);
    if (inode < 0) { ret_code = inode; goto done; }
    
    inode_t in;
    if (block_read((unsigned char *)&in, inode, 1) != 0) { ret_code = -EIO; goto done; }

    inode2stat(sb, &in, inode);
done:
    return ret_code;
    // return -EOPNOTSUPP;
}

/* EXERCISE 2:
 * readdir - get directory contents.
 *
 * call the 'filler' function for *each valid entry* in the
 * directory, as follows:
 *     filler(ptr, <name>, <statbuf>, 0)
 * where
 *   ** "ptr" is the second argument
 *   ** <name> is the name of the file/dir (the name in the direntry)
 *   ** <statbuf> is a pointer to the struct stat (of the file/dir)
 *
 * success - return 0
 * errors - path resolution, ENOTDIR, ENOENT
 *
 * hints:
 *   - this process is similar to the fs_getattr:
 *     -- you will walk file system to find the dir pointed by "path",
 *     -- then you need to call "filler" for each of
 *        the *valid* entry in this dir
 *   - you can ignore "struct fuse_file_info *fi" (also apply to later Exercises)
 */
int fs_readdir(const char *path, void *ptr, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi)
{
    /* your code here */
    int ret_code = 0, inode = path2inum(path);
    if (inode <= 0) { ret_code = inode; goto done; }
    
    unsigned char buf[FS_BLOCK_SIZE];

    if (block_read(buf, inode, 1) != 0) { ret_code = -EIO; goto done; }

    inode_t *in = (inode_t *)buf;
    if (!S_ISDIR(in->mode)) { ret_code = -ENOTDIR; goto done; }
    
    inode = in->ptrs[0];
    if (block_read(buf, inode, 1) != 0) { ret_code = -EIO; goto done; }
    
    dirent_t *dirent = (dirent_t *)buf;
    struct stat statbuf;
    inode_t child_in;
    while (dirent < (dirent_t *)(buf + FS_BLOCK_SIZE)) {
        if (dirent->valid) {
            if (block_read((unsigned char *)&child_in, dirent->inode, 1) != 0) continue;
            inode2stat(&statbuf, &child_in, dirent->inode);
            filler(ptr, dirent->name, &statbuf, 0);
        }
        dirent++;
    }

done:
    return ret_code;
    // return -EOPNOTSUPP;
}


/* EXERCISE 3:
 * read - read data from an open file.
 * success: should return exactly the number of bytes requested, except:
 *   - if offset >= file len, return 0
 *   - if offset+len > file len, return #bytes from offset to end
 *   - on error, return <0
 * Errors - path resolution, ENOENT, EISDIR
 */
int fs_read(const char *path, char *buf, size_t len, off_t offset,
        struct fuse_file_info *fi)
{
    /* your code here */
    int nread = 0, inode = path2inum(path);
    if (inode < 0) { nread = inode; goto done; }
    
    inode_t in;
    if (block_read((unsigned char *)&in, inode, 1) != 0 ) { nread = -EIO; goto done; }
    
    if (S_ISDIR(in.mode)) { nread = -EISDIR; goto done; }

    if (offset >= in.size) goto done;
    
    int start_block = offset / FS_BLOCK_SIZE;
    int start_offset = offset % FS_BLOCK_SIZE;
    unsigned char data[FS_BLOCK_SIZE];
    while (nread < len && nread + offset < in.size) {
        block_read(data, in.ptrs[start_block], 1);
        int pread = FS_BLOCK_SIZE - start_offset;
        if (nread + pread > len)
            pread = len - nread;
        if (nread + pread + offset > in.size)
            pread = in.size - offset - nread;
        for (int i = 0; i < pread; i++) {
            buf[nread + i] = data[start_offset + i];
        }
        nread += pread;
        start_block++;
        start_offset = 0;
    }

done:
    return nread;
    // return -EOPNOTSUPP;
}



/* EXERCISE 3:
 * rename - rename a file or directory
 * success - return 0
 * Errors - path resolution, ENOENT, EINVAL, EEXIST
 *
 * ENOENT - source does not exist
 * EEXIST - destination already exists
 * EINVAL - source and destination are not in the same directory
 *
 * Note that this is a simplified version of the UNIX rename
 * functionality - see 'man 2 rename' for full semantics. In
 * particular, the full version can move across directories, replace a
 * destination file, and replace an empty directory with a full one.
 */
int fs_rename(const char *src_path, const char *dst_path)
{
    /* your code here */
    int ret_code = 0;
    char *_src_path = strdup(src_path), *_dst_path = strdup(dst_path);
    int src_len = 0, dst_len= 0;
    char *src_tokens[10] = {0}, *dst_tokens[10] = {0};
    int src_inode, dst_inode;
    
    if (strlen(src_path) == 0 || src_path[0] != '/' || strlen(dst_path) == 0 || dst_path[0] != '/') {
        ret_code = -ENOENT; goto done;
    }

    src_inode = path2inum(src_path); dst_inode = path2inum(dst_path);

    if (src_inode < 0) { ret_code = -ENOENT; goto done; }
    if (dst_inode > 0) { ret_code = -EEXIST; goto done; }
    
    src_len = sep_path(_src_path, src_tokens, "/", 10);
    dst_len = sep_path(_dst_path, dst_tokens, "/", 10);
 
    if (src_len != dst_len) { ret_code = -EINVAL; goto done; }
    
    int length = 0;
    for (int i = 0; i < src_len - 1; i++) {
        if (strcmp(src_tokens[i], dst_tokens[i]) !=0 ) { ret_code = -EINVAL; goto done; }
        length += strlen(src_tokens[i]) + 1;
    }
    char par_path[300];
    strncpy(par_path, src_path, length);
    par_path[length++] = '/';
    par_path[length] = '\0';

    src_inode = path2inum(par_path);
    unsigned char buf[FS_BLOCK_SIZE];
    if (block_read(buf, src_inode, 1) != 0) { ret_code = -EIO; goto done; }
    inode_t *in = (inode_t *)buf;
    src_inode = in->ptrs[0];
    if (block_read(buf, src_inode, 1) != 0) { ret_code = -EIO; goto done; }

    dirent_t *dirent = find_dirent(buf, src_tokens[src_len - 1]);
    strcpy(dirent->name, dst_tokens[dst_len - 1]);
    block_write(buf, src_inode, 1);

done:
    free(_src_path);
    free(_dst_path);
    return ret_code;
    // return -EOPNOTSUPP;
}

/* EXERCISE 3:
 * chmod - change file permissions
 *
 * success - return 0
 * Errors - path resolution, ENOENT.
 *
 * hints:
 *   - You can safely assume the "mode" is valid.
 *   - notice that "mode" only contains permissions
 *     (blindly assign it to inode mode doesn't work;
 *      why? check out Lab4 instructions about mode)
 *   - S_IFMT might be useful.
 */
int fs_chmod(const char *path, mode_t mode)
{
    /* your code here */
    int ret_code = 0, inode = path2inum(path);
    if (inode <= 0) { ret_code = -ENOENT; goto done; }
    
    unsigned char buf[FS_BLOCK_SIZE];
    if (block_read(buf, inode, 1) != 0) { ret_code =  -EIO; goto done; }
    
    inode_t *in = (inode_t *)buf;
    in->mode &= 0xfffff000;
    in->mode |= 0x00000fff & mode;

    if (block_write(buf, inode, 1) != 0) { ret_code = -EIO; goto done; }
 
 done:
    return ret_code;
    // return -EOPNOTSUPP;
}


/* EXERCISE 4:
 * create - create a new file with specified permissions
 *
 * success - return 0
 * errors - path resolution, EEXIST
 *          in particular, for create("/a/b/c") to succeed,
 *          "/a/b" must exist, and "/a/b/c" must not.
 *
 * If a file or directory of this name already exists, return -EEXIST.
 * If there are already 128 entries in the directory (i.e. it's filled an
 * entire block), you are free to return -ENOSPC instead of expanding it.
 * If the name is too long (longer than 27 letters), return -EINVAL.
 *
 * notes:
 *   - that 'mode' only has the permission bits. You have to OR it with S_IFREG
 *     before setting the inode 'mode' field.
 *   - Ignore the third parameter.
 *   - you will have to implement the helper funciont "alloc_blk" first
 *   - when creating a file, remember to initialize the inode ptrs.
 */
int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    uint32_t cur_time = time(NULL);
    struct fuse_context *ctx = fuse_get_context();
    uint16_t uid = ctx->uid;
    uint16_t gid = ctx->gid;

    // get rid of compiling warnings; you should remove later
    // (void) uid, (void) gid, (void) cur_time;

    /* your code here */
    int ret_code = 0;
    char *_path = strdup(path);
    if (strlen(path) == 0 || path[0] != '/') { ret_code = -ENOENT; goto done; }
    if (path2inum(path) > 0) { ret_code = -EEXIST; goto done; }

    char *path_tokens[10] = {0};
    int path_len = sep_path(_path, path_tokens, "/", 10);

    char par_paths[300];
    par_path(path_tokens, path, par_paths, path_len);

    if (strlen(path_tokens[path_len - 1]) > 27) { ret_code = -EINVAL; goto done; }

    int inode = path2inum(par_paths);
    if (inode < 0) { ret_code = -ENOENT; goto done; }

    unsigned char buf[FS_BLOCK_SIZE];
    block_read(buf, inode, 1);

    if (!S_ISDIR(((inode_t *)buf)->mode)) { ret_code = -ENOTDIR; goto done; }

    inode = ((inode_t *)buf)->ptrs[0];
    block_read(buf, inode, 1);
    dirent_t *dirent = find_empty(buf);
    if (dirent == 0) { ret_code = -ENOSPC; goto done; }
    int f_inode = alloc_blk();
    if (f_inode < 0) { ret_code = f_inode; goto done;}
    strcpy(dirent->name, path_tokens[path_len - 1]);
    dirent->valid = 1;
    dirent->inode = f_inode;
    
    inode_t f;
    f.uid = uid;
    f.gid = gid;
    f.mode = 0;
    f.mode |= 0x00000fff & mode;
    f.mode |= 1 << 15;
    f.size = 0;
    f.ctime = cur_time;
    f.mtime = cur_time;

    block_write((unsigned char *)&f, f_inode, 1);
    block_write(buf, inode, 1);

done:
    free(_path);
    return ret_code;
    // return -EOPNOTSUPP;
}



/* EXERCISE 4:
 * mkdir - create a directory with the given mode.
 *
 * Note that 'mode' only has the permission bits. You
 * have to OR it with S_IFDIR before setting the inode 'mode' field.
 *
 * success - return 0
 * Errors - path resolution, EEXIST
 * Conditions for EEXIST are the same as for create.
 *
 * hint:
 *   - there is a lot of similaries between fs_mkdir and fs_create.
 *     you may want to reuse many parts (note: reuse is not copy-paste!)
 */
int fs_mkdir(const char *path, mode_t mode)
{
    uint32_t cur_time = time(NULL);
    struct fuse_context *ctx = fuse_get_context();
    uint16_t uid = ctx->uid;
    uint16_t gid = ctx->gid;

    // get rid of compiling warnings; you should remove later
    // (void) uid, (void) gid, (void) cur_time;

    /* your code here */
    int ret_code = 0;
    char *_path = strdup(path);
    if (strlen(path) == 0 || path[0] != '/') { ret_code = -ENOENT; goto done; }
    if (path2inum(path) > 0) { ret_code = -EEXIST; goto done; }

    char *path_tokens[10] = {0};
    int path_len = sep_path(_path, path_tokens, "/", 10);
    
    char par_paths[300];
    par_path(path_tokens, path, par_paths, path_len);

    if (strlen(path_tokens[path_len - 1]) > 27) { ret_code = -EINVAL; goto done; }

    int inode = path2inum(par_paths);
    if (inode < 0) { ret_code = -ENOENT; goto done; }
    
    inode_t par_in, par_data_in;
    block_read((unsigned char *)&par_in, inode, 1);
    if (!S_ISDIR(par_in.mode)) { ret_code = -ENOTDIR; goto done; }

    int par_data_inode = par_in.ptrs[0];
    block_read((unsigned char *)&par_data_in, par_data_inode, 1);

    dirent_t *dirent = find_empty((unsigned char *)&par_data_in);
    if (dirent == 0) { ret_code = -ENOSPC; goto done; }

    int sub_inode = alloc_blk();
    if (sub_inode < 0) { ret_code = sub_inode; goto done; }

    int sub_data_inode = alloc_blk();
    if (sub_data_inode < 0) { ret_code = sub_data_inode; free_blk(sub_inode); goto done; }
    inode_t sub_data_in;
    memset(&sub_data_in, 0, sizeof(inode_t));
    block_write((unsigned char *)&sub_data_in, sub_data_inode, 1);
    
    inode_t sub_in;
    sub_in.uid = uid;
    sub_in.gid = gid;
    sub_in.mode = 0;
    sub_in.mode |= 0x00000fff & mode;
    sub_in.mode |= 1 << 14;
    sub_in.size = 4096;
    sub_in.ctime = cur_time;
    sub_in.mtime = cur_time;
    sub_in.ptrs[0] = sub_data_inode;
    block_write((unsigned char *)&sub_in, sub_inode, 1);

    dirent->valid = 1;
    strcpy(dirent->name, path_tokens[path_len - 1]);
    dirent->inode = sub_inode;
    block_write((unsigned char *)&par_data_in, par_data_inode, 1);

done:
    free(_path);
    return ret_code;
    // return -EOPNOTSUPP;
}


/* EXERCISE 5:
 * unlink - delete a file
 *  success - return 0
 *  errors - path resolution, ENOENT, EISDIR
 *
 * hint:
 *   - you will have to implement the helper funciont "free_blk" first
 *   - remember to delete all data blocks as well
 *   - remember to update "mtime"
 */
int fs_unlink(const char *path)
{
    /* your code here */
    int ret_code = 0;
    char *_path = strdup(path);
    if (strlen(path) == 0 || path[0] != '/') { ret_code = -ENOENT; goto done; }

    char *path_tokens[10] = {0};
    int path_len = sep_path(_path, path_tokens, "/", 10);
    char par_paths[300];
    par_path(path_tokens, path, par_paths, path_len);

    int par_inode = path2inum(par_paths);
    if (par_inode < 0) { ret_code = -ENOENT; goto done; }
 
    inode_t par_in, par_data_in;
    block_read((unsigned char *)&par_in, par_inode, 1);
    if(!S_ISDIR(par_in.mode)) { ret_code = -ENOTDIR; goto done; }
    
    int par_data_inode = par_in.ptrs[0];
    block_read((unsigned char *)&par_data_in, par_data_inode, 1);

    dirent_t *dirent = find_dirent((unsigned char *)&par_data_in, path_tokens[path_len - 1]);
    if (dirent == 0) { ret_code = -ENOENT; goto done; }
    
    inode_t file_in; int file_inode = dirent->inode;
    block_read((unsigned char *)&file_in, file_inode, 1);
    if (S_ISDIR(file_in.mode)) { ret_code = -EISDIR; goto done; }

    for (int i = 0; i < DIV_ROUND_UP(file_in.size, FS_BLOCK_SIZE); i++) { free_blk(file_in.ptrs[i]); }
    free_blk(file_inode);

    dirent->valid = 0;
    block_write((unsigned char *)&par_data_in, par_data_inode, 1);

    uint32_t cur_time = time(NULL);
    par_in.mtime = cur_time;
    block_write((unsigned char *)&par_in, par_inode, 1);

done:
    free(_path);
    return ret_code;
    // return -EOPNOTSUPP;
}

/* EXERCISE 5:
 * rmdir - remove a directory
 *  success - return 0
 *  Errors - path resolution, ENOENT, ENOTDIR, ENOTEMPTY
 *
 * hint:
 *   - fs_rmdir and fs_unlink have a lot in common; think of reuse the code
 */
int fs_rmdir(const char *path)
{
    /* your code here */
    int ret_code = 0;
    char *_path = strdup(path);
    if (strlen(path) == 0 || path[0] != '/') { ret_code = -ENOENT; goto done; }

    char *path_tokens[10] = {0};
    int path_len = sep_path(_path, path_tokens, "/", 10);
    if (path_len == 0) { ret_code = -ENOENT; goto done; }

    char par_paths[300];
    par_path(path_tokens, path, par_paths, path_len);

    int par_inode = path2inum(par_paths);
    if (par_inode < 0) { ret_code = -ENOENT; goto done; }

    inode_t par_in, par_data_in;
    block_read((unsigned char *)&par_in, par_inode, 1);
    if (!S_ISDIR(par_in.mode)) { ret_code = -ENOTDIR; goto done; }

    int par_data_inode = par_in.ptrs[0];
    block_read((unsigned char *)&par_data_in, par_data_inode, 1);
    dirent_t *dirent = find_dirent((unsigned char *)&par_data_in, path_tokens[path_len - 1]);
    if (dirent == 0) { ret_code = -ENOENT; goto done; }

    int sub_inode = dirent->inode;
    inode_t sub_in;
    block_read((unsigned char *)&sub_in, sub_inode, 1);
    if (!S_ISDIR(sub_in.mode)) { ret_code = -ENOTDIR; goto done; }
    
    int sub_data_inode = sub_in.ptrs[0];
    inode_t sub_data_in;
    block_read((unsigned char *)&sub_data_in, sub_data_inode, 1);

    dirent_t *k = find_full((unsigned char *)&sub_data_in);
    if (k) { ret_code = -ENOTEMPTY; goto done; }

    free_blk(sub_data_inode);
    free_blk(sub_inode);
    dirent->valid = 0;
    block_write((unsigned char *)&par_data_in, par_data_inode, 1);
    par_in.mtime = time(NULL);
    block_write((unsigned char *)&par_in, par_inode, 1);

done:
    free(_path);
    return ret_code;
    // return -EOPNOTSUPP;
}

/* EXERCISE 6:
 * write - write data to a file
 * success - return number of bytes written. (this will be the same as
 *           the number requested, or else it's an error)
 *
 * Errors - path resolution, ENOENT, EISDIR, ENOSPC
 *  return EINVAL if 'offset' is greater than current file length.
 *  (POSIX semantics support the creation of files with "holes" in them,
 *   but we don't)
 *  return ENOSPC when the data exceed the maximum size of a file.
 */
int fs_write(const char *path, const char *buf, size_t len,
         off_t offset, struct fuse_file_info *fi)
{
    /* your code here */
    int ret_code, inode;
    inode = path2inum(path);
    if (inode < 0) { ret_code = inode; goto done; }

    inode_t in;
    block_read((unsigned char *)&in, inode, 1);

    if (S_ISDIR(in.mode)) { ret_code = -EISDIR; goto done; }

    if (offset > in.size) { ret_code = -EINVAL; goto done; }

    if (offset + len > FS_BLOCK_SIZE * NUM_PTRS_INODE) { ret_code = -ENOSPC; goto done; }
    
    size_t nwrite = len;
    size_t block_ini = offset / FS_BLOCK_SIZE;
    size_t block_offset = offset % FS_BLOCK_SIZE;
    char buffer[FS_BLOCK_SIZE];
    while (nwrite > 0) {
        if (block_ini >= DIV_ROUND_UP(in.size, FS_BLOCK_SIZE)) {
            in.ptrs[block_ini] = alloc_blk();
            if (in.ptrs[block_ini] < 0) { ret_code = in.ptrs[block_ini]; goto done; }
        }
        block_read(buffer, in.ptrs[block_ini], 1);
        if (nwrite > FS_BLOCK_SIZE - block_offset) {
            memcpy(buffer + block_offset, buf, FS_BLOCK_SIZE-block_offset);
            nwrite -= FS_BLOCK_SIZE - block_offset;
            buf += FS_BLOCK_SIZE - block_offset;
            block_offset = 0;
        } else {
            memcpy((char*)(buffer + block_offset), buf, nwrite);
            buf += nwrite;
            nwrite = 0;
        }
        block_write(buffer, in.ptrs[block_ini], 1);
        block_ini++;
    }
    if (offset + len > in.size) in.size = offset + len;
    in.mtime = time(NULL);
    block_write((unsigned char *)&in, inode, 1);   
    ret_code = (int) len; 
done:
    return ret_code;
    // return -EOPNOTSUPP;
}

/* EXERCISE 6:
 * truncate - truncate file to exactly 'len' bytes
 * note that CS5600 fs only allows len=0, meaning discard all data in this file.
 *
 * success - return 0
 * Errors - path resolution, ENOENT, EISDIR, EINVAL
 *    return EINVAL if len > 0.
 */
int fs_truncate(const char *path, off_t len)
{
    /* you can cheat by only implementing this for the case of len==0,
     * and an error otherwise.
     */
    if (len != 0) {
        return -EINVAL;        /* invalid argument */
    }

    /* your code here */
    int ret_code = 0, inode = path2inum(path);
    if (inode <= 0) { ret_code = inode; goto done; }
    
    inode_t in;
    block_read((unsigned char *)&in, inode, 1);

    if (S_ISDIR(in.mode)) { ret_code = -EISDIR; goto done; }

    for (int i = 0; i < DIV_ROUND_UP(in.size, FS_BLOCK_SIZE); i++) free_blk(in.ptrs[i]);        

    in.size = 0;
    block_write((unsigned char *)&in, inode, 1);
    
done:
    return ret_code;
    // return -EOPNOTSUPP;
}

/* EXERCISE 6:
 * Change file's last modification time.
 *
 * notes:
 *  - read "man 2 utime" to know more.
 *  - when "ut" is NULL, update the time to now (i.e., time(NULL))
 *  - you only need to use the "modtime" in "struct utimbuf" (ignore "actime")
 *    and you only have to update "mtime" in inode.
 *
 * success - return 0
 * Errors - path resolution, ENOENT
 */
int fs_utime(const char *path, struct utimbuf *ut)
{
    /* your code here */
    int inode = path2inum(path);
    if (inode <= 0) return inode;
    
    inode_t in;
    block_read((unsigned char *)&in, inode, 1);

    if (S_ISDIR(in.mode)) return -EISDIR;

    if (!ut) in.mtime = time(NULL); else in.mtime = (uint32_t) ut->modtime;
    block_write((unsigned char *)&in, inode, 1);
    return 0;
    // return -EOPNOTSUPP;

}



/* operations vector. Please don't rename it, or else you'll break things
 */
struct fuse_operations fs_ops = {
    .init = fs_init,            /* read-mostly operations */
    .statfs = fs_statfs,
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .read = fs_read,
    .rename = fs_rename,
    .chmod = fs_chmod,

    .create = fs_create,        /* write operations */
    .mkdir = fs_mkdir,
    .unlink = fs_unlink,
    .rmdir = fs_rmdir,
    .write = fs_write,
    .truncate = fs_truncate,
    .utime = fs_utime,
};

