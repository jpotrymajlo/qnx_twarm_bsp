/*
 * $QNXLicenseC: 
 * Copyright 2007, 2008 QNX Software Systems.  
 *  
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $ 
 */

#ifndef __NTO_CAM_H_INCLUDED
#define __NTO_CAM_H_INCLUDED

#include <stddef.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/iomsg.h>
#include <sys/resmgr.h>
#include <sys/queue.h>
#include <sys/syspage.h>
#include <sys/slogcodes.h>
#include <sys/cam_device.h>

#include <cam.h>
#include <xpt.h>
#include <ntoscsi.h>

#define XSTR(x)	#x
#define CAM_STRINGIZE(x) XSTR(x)

// Useful TAILQ manifests possibly not in sys/queue.h
#ifndef TAILQ_EMPTY
	#define	TAILQ_EMPTY(head) ((head)->tqh_first == NULL)
#endif
#ifndef TAILQ_FIRST
	#define	TAILQ_FIRST(head) ((head)->tqh_first)
#endif
#ifndef TAILQ_LAST
	#define	TAILQ_LAST(head) ((head)->tqh_last)
#endif
#ifndef TAILQ_NEXT
	#define	TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)
#endif
#ifndef TAILQ_PREV
	#define TAILQ_PREV(elm, field) ((elm)->field.tqe_prev)
#endif

typedef struct _devobj_extension			DEVEXT;
typedef struct _devobj						DEVOBJ;
typedef struct _pdrvr						PDRVR;
typedef struct _peripheral_driver_extension	PDRVR_EXT;
typedef struct _scsi_req_block				SRB;

#define MAX_RETRIES					10
#define SCSI_START_UNIT_TIMEOUT		30
#define SCSI_LOAD_TIMEOUT			30
#define BLOCK_SIZE					512
#define BOUNCE_BUF_SIZE				32768
#define UNLOCK_MEDIA				0
#define LOCK_MEDIA					1
#define LOAD_MEDIA					1
#define EJECT_MEDIA					0
#define MAX_TARGET					16		// Max number of SCSI targets
#define MAX_WIDE_TARGET				16		// Max number of wide SCSI targets
#define MAX_NARROW_TARGET			8		// Max number of narrow SCSI targets
#define MAX_LUN						8		// Max number of LUNs per target
#define CMD_6_BYTE					0
#define CMD_10_BYTE					1
#define MAX_IO_BLKS					255
#define MAX_SG						255

int cam_cdb( CCB *);
#define cam_cdb( ccb )				( (CDB *)ccb->cam_cdb_io.cam_cdb_bytes )

void cam_set_pdrvr( CCB *, void *);
#define cam_set_pdrvr( ccb, pdrv )	( ccb->cam_pdrv_ptr = (char *)pdrv )

void cam_set_cbf( CCB *, void *);
#define cam_set_cbf( ccb, cbf )		( ccb->cam_cbfcnp = (void *)cbf )

void cam_data_ptr( CCB_SCSIIO *, void *);
#define cam_data_ptr( ccb, data )	if( ccb->cam_ch.cam_flags & CAM_DATA_PHYS ) {	\
										off64_t	_off;								\
										mem_offset64( (void *)(data), NOFD, 1, &_off, NULL ); \
										ccb->cam_data.cam_data_ptr = (paddr_t)_off;	\
									}												\
									else											\
										ccb->cam_data.cam_data_ptr = (paddr_t)data


#define cam_clear_ccb(_x)			( memset( ((char *)(_x)) +							\
									offsetof(struct ccb_header, cam_func_code), 0x00,	\
									_x->cam_ch.cam_ccb_len -							\
									offsetof(struct ccb_header, cam_func_code)) )

void cam_set_sense(CCB *, void *);
#define cam_set_sense(ccb, sense)	( ((CCB_SCSIIO *)ccb)->cam_sense_ptr =	\
									(unsigned char *)(sense),				\
									(((CCB_SCSIIO *)ccb)->cam_sense_len =	\
									sizeof(*(sense))) )

void cam_target(CCB_SCSIIO *, DEVEXT *);
#define cam_target(ccb, dext)		( (ccb)->cam_ch.cam_path_id		= (dext)->path_id,		\
									  (ccb)->cam_ch.cam_target_id	= (dext)->target,		\
									  (ccb)->cam_ch.cam_target_lun	= (dext)->lun,			\
									  (ccb)->cam_ch.cam_flags		= (dext)->cam_flags,	\
									  (ccb)->cam_timeout			= (dext)->timeout,		\
									  (ccb)->cam_ch.cam_func_code	= XPT_SCSI_IO,			\
									  (dext)->cam_flags				&= ~( CAM_INITIATE_SYNC | CAM_DIS_SYNC ) )

typedef struct _cam_entry CAM_ENTRY;

typedef struct _cam_entry {
	int		nfuncs;
	int		(*scsi_scan_bus)( int path );
	long	(*scsi_scan_devs)( int type, int (*func)( CCB_PATHINQ *, CCB_GETDEV *));
	DEVOBJ	*(*scsi_create_device)( CCB_PATHINQ *ccb_path, CCB_GETDEV *ccb_getdev, int ext_size, int max_io );
	long	(*scsi_setasync_cb)( ASYNC_INFO **alist, CCB_SETASYNC *ccb );
	CCB		*(*scsi_ccb_alloc)(void);
	void	(*scsi_ccb_free)(CCB *);
	long	(*scsi_action)( CCB *ccb );
	void	*(*scsi_malloc)( int size, int mflags );
	void	(*scsi_free)( void *addr, int size );
	int		(*scsi_io)( void *hdl, ioreq_t *ioreq );
	int		(*scsi_abort)( void *hdl, ioreq_t *ioreq, unsigned flags );
	int		(*scsi_devctl)(void *hdl, resmgr_context_t *ctp, int ioflag, ioreq_t *tag, unsigned long dcmd, void *data, size_t size, int *ret );
	int		(*scsi_interpret_sense)( DEVOBJ *dobj, CCB_SCSIIO *ccb, int *status, int rcount );
	int		(*scsi_action_wait)( DEVOBJ *dobj, CCB_SCSIIO *ccb, int retries );
	int		(*scsi_release_simq)( DEVOBJ *dobj );
	int		(*scsi_read_capacity)( DEVOBJ *dobj );
	int		(*scsi_mode_sense)( DEVOBJ *dobj, int page, char *mdata, int len );
	int		(*scsi_mode_select)( DEVOBJ *dobj, char *pdata, int len, int save );
	void	*(*scsi_mode_page)( char *msdata, int page, int len, int type );
	int		(*scsi_reserve)( DEVOBJ *dobj );
	int		(*scsi_release)( DEVOBJ *dobj );
	int		(*scsi_prevent)( DEVOBJ *dobj, int prevent );
	int		(*scsi_synchronize_cache)( DEVOBJ *dobj );
	int		(*scsi_iomsg)( void *hdl, resmgr_context_t *ctp, int ioflag, ioreq_t *tag, io_msg_t *msg );
} cam_entry_t;

struct _scsi_req_block {
 	CCB_SCSIIO		ccb;
	DEVOBJ			*dobj;
	ioreq_t			*ioreq;
	int				nblks;
	mdl_t			*mdl;
	unsigned		rcount;			// retry count
	unsigned		b_blkoff;		// block offset in ioreq
	SG_ELEM			sg_list[ MAX_SG ];
	SCSI_SENSE		sense;
	ioreq_t			bioreq;
	mdl_t			bmdl;
	int				bmd_size;
};

// This extension structure is specific to each peripheral driver.
// Peripheral drivers that use blkio can use the following structure.
struct _peripheral_driver_extension {
	PDRVR					*pdrv;
	char					*prefix;
	int						dcount;			// device count
	char					*async_buffer;	// requires 22 bytes for AEN
	const io_entry_t		*io_funcs;
	cam_entry_t				*cam_funcs;
	void					*dll;			// io-blk dll handle
	int						argc;
	char					**argv;
	int						verbosity;
	int						rsvd[3];
};

// This structure is common to all peripheral drivers.  It must
// contain at least the following members at the start of the
// structure.
struct _pdrvr
{
	PDRVR_EXT				*pext;
	TAILQ_HEAD(,_devobj)	dlist;			// linked list of devices
	int						pflags;			// various flags
	int						rsvd[3];
};
#define PDRVR_FLG_MEDIA_CHECK		0x01

struct _devobj {
	PDRVR					*pdrvr;			// pointer to peripheral driver
	TAILQ_ENTRY(_devobj)	dlink;			// pointer to next device
	void					*dext;			// device extension
	void					*dentry;
	int						rsvd[2];
};

// This structure may be different for each peripheral type.
// Devices that require io-blk can use the following structure.
// Note:  Device extenstions must include at least the common
// members at the start of the structure.
#define DEXT_EFLG_NOSYNC	0x04
struct _devobj_extension {
	DEVOBJ					*dobj;			// Device object
	int						eflags;			// extension flags
	int						path_id;		// Cam path id to device
	int						target;			// SCSI target id of device
	int						lun;			// SCSI logical unit number
	char					device_type;	// Peripherial Device Type
	char					starvation;
	char					retries;
	char					rsvd[1];
	int						cam_flags;		// CAM flags ie CAM_DIS_DISCONNECT, CAM_DIS_SYNC
	int						timeout;		// Device timeout (in seconds)
	int						locks;			// Number of media locks
	int						media_changes;	// Number of media changes
	int						dflags;			// Device flags ie read only, no media
	cam_devinfo_t			dinfo;			// Device info (geometry)
	int						max_io_blks;	// Maximum blocks device can handle
	int						max_sg;			// Maximum scatter/gather hba can handle
	int						mflags;			// Memory allocation flags
	ioque_t					queue;			// Request queue
	ioque_t					aqueue;			// Active request queue
	int						max_active;		// Maximum number of active srbs
	void					*srb_pool;		// IO request pool
	int						(*error)( DEVOBJ *, CCB_SCSIIO *, int *, int * );
	void					*handle;		// handle (ie io-blk handle from mount)
	void					*mext;			// Model extension
	int						rsvd1[2];
};

CCB		*scsi_ccb_alloc(void);
void	scsi_ccb_free(CCB *ccb);
long	scsi_action(CCB *ccb);
long	scsi_scan_devs(int type, int (*func)( CCB_PATHINQ *, CCB_GETDEV *));
int		scsi_scan_bus(int path);
long	scsi_setasync_cb( ASYNC_INFO **alist, CCB_SETASYNC *ccb );
void	*scsi_malloc( int size, int mflags );
void	scsi_free( void *addr, int size );
int		scsi_io( void *hdl, ioreq_t *ioreq );
int		scsi_iomsg( void *hdl, resmgr_context_t *ctp, int ioflag, ioreq_t *tag, io_msg_t *msg );
int		scsi_abort( void *hdl, ioreq_t *ioreq, unsigned flags );
int		scsi_devctl( void *hdl, resmgr_context_t *ctp, int ioflag, ioreq_t *tag, unsigned long dcmd, void *data, size_t size, int *ret );
int		scsi_action_wait( DEVOBJ *dobj, CCB_SCSIIO *ccb, int retries );
int		scsi_release_simq( DEVOBJ *dobj );
int		scsi_read_capacity( DEVOBJ *dobj );
int 	scsi_log_sense( DEVOBJ *dobj, int page, char *ldata, int opt, int pc, int parm_ptr, int len );
int		scsi_log_select( DEVOBJ *dobj, char *pdata, int len, int pc, int opt );
int		scsi_mode_sense( DEVOBJ *dobj, int page, char *mdata, int len );
int		scsi_mode_select( DEVOBJ *dobj, char *pdata, int len, int save );
void	*scsi_mode_page( char *msdata, int page, int len, int type );
int		scsi_reserve( DEVOBJ *dobj );
int		scsi_release( DEVOBJ *dobj );
int		scsi_synchronize_cache( DEVOBJ *dobj );
int		scsi_test_ready( DEVOBJ *dobj );
int		scsi_interpret_sense( DEVOBJ *dobj, CCB_SCSIIO *ccb, int *status, int rcount );
int		scsi_inquiry( DEVOBJ *dobj, SCSI_INQUIRY *inq, int inq_len );
int		scsi_unit_ready( DEVOBJ *dobj );
int		scsi_removal( DEVOBJ *dobj, int prevent );
DEVOBJ	*scsi_create_device( CCB_PATHINQ *ccb_path, CCB_GETDEV *ccb_getdev, int ext_size, int max_io );
int scsi_destroy_device( DEVOBJ *dobj );
const char *typetostr( int type );
ssize_t cam_slogf( int opcode, int severity, int verbosity, const char *fmt, ... );

#ifdef CAM_MEM_DEBUG
extern void _cam_free( void *p, char *f, int l );
extern void *_cam_calloc( size_t n, size_t sz, char *f, int l );
extern void *_cam_realloc( void *p, size_t sz, char *f, int l );

#define cam_free( _p ) _cam_free( (_p), __FILE__, __LINE__ )
#define cam_calloc( _n, _sz ) _cam_calloc( (_n), (_sz), __FILE__, __LINE__ )
#define cam_realloc( _p, _sz ) _cam_realloc( (_p), (_sz), __FILE__, __LINE__ )
#else
#define cam_free		free
#define cam_calloc		calloc
#define cam_realloc		realloc

#endif

#endif

