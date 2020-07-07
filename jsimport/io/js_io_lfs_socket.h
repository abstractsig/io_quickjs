/*
 *
 * 
 *
 * LICENSE
 * =======
 * See end of file for license terms.
 *
 */
#ifndef js_io_lfs_socket_H_
#define js_io_lfs_socket_H_
#include "lfs.h"

typedef struct {
	int (*read_register) (io_socket_t*,uint32_t,uint32_t*);
	int (*write_register) (io_socket_t*,uint32_t,uint32_t);
	
	int (*read_content) (io_socket_t*,uint32_t,uint8_t*,uint32_t);
	int (*write_content) (io_socket_t*,uint32_t,uint8_t const*,uint32_t);
	int (*erase_block) (io_socket_t*,uint32_t);
} io_block_memory_interface_t;


// not really js specific ...

#ifdef IMPLEMENT_JS_IO
//-----------------------------------------------------------------------------
//
// implementation
//
//-----------------------------------------------------------------------------

int js_io_lfs_read (struct lfs_config const*,lfs_block_t,lfs_off_t,void*,lfs_size_t);
int js_io_lfs_write (struct lfs_config const*,lfs_block_t,lfs_off_t,const void*,lfs_size_t);
int js_io_lfs_erase (struct lfs_config const*,uint32_t);
int lfs_sync (struct lfs_config const*);


/*
 *-----------------------------------------------------------------------------
 *
 * lfs_read --
 *
 *-----------------------------------------------------------------------------
 */
int
js_io_lfs_read (
	struct lfs_config const *dc,
	lfs_block_t				block,
	lfs_off_t				offset,
	void						*buffer,
	lfs_size_t				size
) {

	return 0;
}

int
lfs_write (
	struct lfs_config const *dc,
	lfs_block_t block,
	lfs_off_t offset,
	const void *buffer,
	lfs_size_t size
) {

	return 0;
}

int
lfs_erase (struct lfs_config const *dc,uint32_t block) {
	return 0;
}

int 
lfs_sync (struct lfs_config const *dc) {
	return 0;
}



#endif /* IMPLEMENT_JS_IO */
#endif
