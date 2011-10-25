/*
 * $QNXLicenseC: 
 * Copyright 2008, QNX Software Systems.  
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




// Module Description:  MMC command processing

#include <sim_mmc.h>

void mmc_sense( SIM_HBA *hba, int sense, int asc, int ascq )
{
	SIM_MMC_EXT	*ext = (SIM_MMC_EXT *)hba->ext;
	CCB_SCSIIO	*ccb = ext->nexus;
	SCSI_SENSE	*sptr = (SCSI_SENSE *)ccb->cam_sense_ptr;
	ccb->cam_ch.cam_status  = CAM_REQ_CMP_ERR;
	ccb->cam_scsi_status    = SCS_CHECK;

	if( sptr ) {
		memset( sptr, 0, sizeof( *sptr ) );
		ccb->cam_ch.cam_status  |= CAM_AUTOSNS_VALID;
		sptr->error	= 0x70;					// Error code
		sptr->sense = sense;				// Sense key
		sptr->asc	= asc;					// Additional sense code (Invalid field in CDB)
		sptr->ascq	= ascq;					// Additional sense code qualifier
	}
																
}

void mmc_reset( SIM_HBA *hba )
{
	SIM_MMC_EXT		*ext = (SIM_MMC_EXT *)hba->ext;
	mmc_io_t		mmc;

	memset( &mmc, 0, sizeof( mmc ) );
	mmc.mmc_tid			= MMC_TARGET_MMC;
	mmc.mmc_cmd			= MMC_GO_IDLE_STATE;

	mmc_io(ext->bshdl, &mmc);
}

void mmc_error( SIM_HBA *hba, int mmc_status )
{
	SIM_MMC_EXT	*ext = (SIM_MMC_EXT *)hba->ext;
	CCB_SCSIIO	*ccb = ext->nexus;

	switch( mmc_status ) {
		case MMC_DATA_OVERRUN:
			ccb->cam_ch.cam_status = CAM_DATA_RUN_ERR;
			break;
		case MMC_NOT_PRESENT:
			mmc_sense( hba, SK_NOT_RDY, ASC_MEDIA_NOT_PRESENT, 0 );
			break;
		case MMC_TIMEOUT:
		case MMC_COMMAND_FAILURE:
			mmc_reset( hba );
			ccb->cam_ch.cam_status = CAM_CMD_TIMEOUT;
			break;
		case MMC_READ_ERROR:			// CRC errors
		case MMC_WRITE_ERROR:
			ccb->cam_ch.cam_status = CAM_CMD_TIMEOUT;
			break;
		default:
			ccb->cam_ch.cam_status = CAM_CMD_TIMEOUT;
			break;
	}
}

void mmc_unit_ready( SIM_HBA *hba )
{
	SIM_MMC_EXT	*ext;

	ext = (SIM_MMC_EXT *)hba->ext;

	if( ext->eflags & MMC_EFLAG_VALID ) {
// since we can't tell if the media changed, tell the filesystems that it has
		mmc_sense( hba, SK_UNIT_ATN, ASC_MEDIUM_CHANGED, ASCQ_UNKNOWN_CHANGED );
	}
	else {
		mmc_sense( hba, SK_NOT_RDY, ASC_MEDIA_NOT_PRESENT, 0 );
	}
}

void mmc_inquiry( SIM_HBA *hba )
{
	SIM_MMC_EXT		*ext;
	CCB_SCSIIO		*ccb;
	SCSI_INQUIRY	*iptr;
	uint32_t		cid[4];
	mmc_io_t		mmc;

	ext		= (SIM_MMC_EXT *)hba->ext;
	ccb		= ext->nexus;
	iptr	= (SCSI_INQUIRY *)ccb->cam_data.cam_data_ptr;

	memset( &mmc, 0, sizeof( mmc ) );
	mmc.mmc_tid			= MMC_TARGET_MMC;
	mmc.mmc_cmd			= MMC_SEND_CID;
	mmc.mmc_data		= (_uint8 *)cid;

	if (mmc_io(ext->bshdl, &mmc)) {
		mmc_error(hba, mmc.mmc_status);
		return;
	}

	memset( iptr, 0, sizeof( *iptr ) );

	iptr->peripheral	= D_DIR_ACC | INQ_QUAL_AVAIL;
	iptr->rmb			= CAM_FALSE;	// not removable
//	iptr->rmb			= CAM_TRUE;		// removable
	iptr->version		= 1;
	iptr->adlen			= 32;

	/* Vendor ID */
	iptr->vend_id[0]    = 'S';
	iptr->vend_id[1]    = 'D';
	iptr->vend_id[2]    = ':';
	iptr->vend_id[3]    = '#';
	ultoa(cid[3] >> 24, &iptr->vend_id[4], 10);
	/* Product ID */
#if 1
	iptr->prod_id[4]    = cid[2] & 0xff;
	iptr->prod_id[3]    = (cid[2] >> 8) & 0xff;
	iptr->prod_id[2]    = (cid[2] >> 16) & 0xff;
	iptr->prod_id[1]    = (cid[2] >> 24) & 0xff;
	iptr->prod_id[0]    = cid[3] & 0xff;
#else
	strncpy(iptr->prod_id, &cid[2], 5);
#endif

	/* Product revision level */
	iptr->prod_rev[0]	= (cid[1] >> 28) + '0';
	iptr->prod_rev[1]	= '.';
	iptr->prod_rev[2]	= ((cid[1] >> 24) & 0x0f) + '0';

	iptr->vend_spc[0]	= 0;				// Vendor Specific

	ccb->cam_ch.cam_status	= CAM_REQ_CMP;
	ccb->cam_scsi_status	= SCS_GOOD;
}

// this routine is only needed if the driver reports a not ready condition
void mmc_spindle( SIM_HBA *hba )
{
	SIM_MMC_EXT	*ext;
	CCB_SCSIIO	*ccb;

	ext = (SIM_MMC_EXT *)hba->ext;
	ccb = ext->nexus;

	switch( ccb->cam_cdb_io.cam_cdb_bytes[4] ) {
		case MMC_SPINDLE_STOP:
		case MMC_SPINDLE_START:
			ccb->cam_ch.cam_status	= CAM_REQ_CMP;
			ccb->cam_scsi_status	= SCS_GOOD;
			break;
		case MMC_SPINDLE_EJECT:
		case MMC_SPINDLE_LOAD:
		default:
			mmc_sense( hba, SK_ILLEGAL, 0x24, 0 );
			break;
	}
}

void mmc_capacity( SIM_HBA *hba )
{
	SIM_MMC_EXT		*ext;
	READ_CAPACITY	*cptr;
	CCB_SCSIIO		*ccb;
	uint32_t		csd[4];
	_uint32			csize;
	_uint32			csizem;
	_uint32			bsize;
	mmc_io_t		mmc;

	ext		= (SIM_MMC_EXT *)hba->ext;
	ccb		= ext->nexus;
	cptr	= (READ_CAPACITY *)ccb->cam_data.cam_data_ptr;

	memset( &mmc, 0, sizeof( mmc ) );
	mmc.mmc_tid			= MMC_TARGET_MMC;
	mmc.mmc_cmd			= MMC_SEND_CSD;
	mmc.mmc_data		= (_uint8 *)csd;

	if( mmc_io( ext->bshdl, &mmc ) ) {
		mmc_error( hba, mmc.mmc_status );
		return;
	}

	if (csd[3] & 0x40000000) { // SD version 2
		bsize  = 512;	// fixed block size for V2
		csize  = ((csd[1] >> 16) | ((csd[2] & 0x3F) << 16)) + 1;
		csizem = 1024;	// fixed 1K for V2
	} else {	// SD version 1
		bsize  = 1 << ((csd[2] >> 16) & 0x0F);
		csize  = ((csd[1] >> 30) | ((csd[2] & 0x3FF) << 2)) + 1;
		csizem = 1 << (((csd[1] >> 15) & 0x07) + 2);

		// force to 512 byte block
		if (bsize > 512 && (bsize % 512) == 0) {
			uint32_t ts = bsize / 512;
			csize = csize * ts;
			bsize = bsize / ts;
		}
	}

	ext->mmc_blksize		= bsize;
	//reserve 1 block
	cptr->lba				= ENDIAN_BE32(csize * csizem-1);
	cptr->blk_size			= ENDIAN_BE32(bsize);
	ccb->cam_ch.cam_status	= CAM_REQ_CMP;
	ccb->cam_scsi_status	= SCS_GOOD;
}

int mmc_get_cid( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_MMC_EXT	*ext;
	MMCSD_CID		*cid;
	mmc_io_t		mmc;
	_uint32	 		data[4];

	ext		= (SIM_MMC_EXT *)hba->ext;
	cid= (MMCSD_CID *)ccb->cam_devctl_data;
	
	ccb->cam_devctl_status	= EOK;
	memset( &mmc, 0, sizeof( mmc ) );
	mmc.mmc_tid		= MMC_TARGET_MMC;
	mmc.mmc_cmd	= MMC_SEND_CID;
	mmc.mmc_data	= (_uint8 *)data;


	if( mmc_io( ext->bshdl, &mmc ) ) {
		ccb->cam_devctl_status = -1;
		return( CAM_REQ_CMP );
	}
	if(cid->flags==MMCSD_FULL_CID){
		memcpy(cid->cid.full_cid.cid, data, 16);

	}else{
		cid->cid.parsed_cid.psn = (data[0] >>24) & 0xFF;
		cid->cid.parsed_cid.psn |= (data[1] & 0xFFFFFF)<<8;
		cid->cid.parsed_cid.oid = (data[3]>>8) & 0xFFFF;
		cid->cid.parsed_cid.mdt = (data[0]>>8) & 0xFFF;
		cid->cid.parsed_cid.pnm[4] = data[2] & 0xFF;
		cid->cid.parsed_cid.pnm[3] = (data[2]>>8) & 0xFF;
		cid->cid.parsed_cid.pnm[2] = (data[2]>>16) & 0xFF;
		cid->cid.parsed_cid.pnm[1] = (data[2]>>24) & 0xFF;
		cid->cid.parsed_cid.pnm[0]=  data[3] & 0xFF;
		cid->cid.parsed_cid.prv = (data[1]>>24) & 0xFF;
		cid->cid.parsed_cid.mid = (data[3]>>24) & 0xFF;
		cid->cid.parsed_cid.crc7 = (data[0]>>1) & 0x7F;
	}	
	return( CAM_REQ_CMP );
}

void mmc_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb)
{
	int				status;

	switch( ccb->cam_devctl_dcmd ) {
		case DCMD_MMCSD_GET_CID:
			status = mmc_get_cid( hba, ccb );
			break;

		default:
			status = CAM_REQ_CMP;
			break;
	}
	ccb->cam_ch.cam_status	= status;
}

paddr_t mmc_mphys( CCB_SCSIIO *ccb, paddr_t dptr, int sgi )
{
	mdl_t		*mdl;
	int			cnt;
	ioreq_t		*ioreq;
	off64_t		_off;

	if (ccb->cam_ch.cam_flags & CAM_DATA_PHYS)
		return (dptr);

	if (ioreq = (ioreq_t *)ccb->cam_req_map) {
		mdl = ioreq->mdl;
		if (mdl[sgi].vaddr == (caddr_t)dptr)
			return (mdl[sgi].paddr);
 
		for (cnt = ioreq->nmdl; cnt; cnt--, mdl++) {
			if (mdl->vaddr == (caddr_t)dptr)
				return (mdl->paddr);
		}
	}

	mem_offset64((void *)(dptr), NOFD, 1, &_off, NULL);

	return ((paddr_t)_off);
}

void mmc_rw( SIM_HBA *hba, int dir )
{
	SIM_MMC_EXT	*ext;
	CCB_SCSIIO	*ccb;
	SG_ELEM		*sge;
	_uint32		blkno;
	mmc_io_t	mmc;

	ext		= (SIM_MMC_EXT *)hba->ext;
	ccb		= ext->nexus;
	blkno	= UNALIGNED_RET32( &ccb->cam_cdb_io.cam_cdb_bytes[2] );
	blkno	= ENDIAN_BE32( blkno );

	if ((ext->eflags & MMC_EFLAG_NODMA) && (ccb->cam_ch.cam_flags & CAM_DATA_PHYS)) {
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "mmc_rw : CAM_DATA_PHYS - dma off" );
		mmc_error( hba, CAM_PROVIDE_FAIL );
		return;
	}

	memset(&mmc, 0, sizeof(mmc));
	mmc.mmc_tid	= MMC_TARGET_MMC;

	if (ccb->cam_ch.cam_flags & CAM_SCATTER_VALID) {
		uint32_t	sgi, s_addr = 0, d_len = 0;

		if (ext->eflags & MMC_EFLAG_SG) {
			mmc.mmc_data		= (uint8_t *)ccb->cam_data.cam_sg_ptr;
			mmc.mmc_data_len	= ccb->cam_sglist_cnt;
			mmc.mmc_arg		= blkno;
			mmc.mmc_flags		= MMCIO_SG;
			mmc.mmc_cmd		= dir == CAM_DIR_IN ? MMC_READ_MULTIPLE_BLOCK : MMC_WRITE_MULTIPLE_BLOCK;

			if (mmc_io(ext->bshdl, &mmc)) {
				mmc_error(hba, mmc.mmc_status);
				return;
			}
		} else {
			for (sgi = 0, sge = ccb->cam_data.cam_sg_ptr; sgi < ccb->cam_sglist_cnt; sgi++, sge++) {
				if (ext->eflags & MMC_EFLAG_NODMA) {
					if (s_addr) {
						if ((s_addr + d_len) == sge->cam_sg_address) {
							d_len += sge->cam_sg_count;
							if (sgi < ccb->cam_sglist_cnt - 1)
								continue;
						} else
							sgi--, sge--;
					} else {
						s_addr = sge->cam_sg_address;
						d_len = sge->cam_sg_count;
						if (sgi < ccb->cam_sglist_cnt - 1)
							continue;
					}

					mmc.mmc_flags	= MMCIO_VIRT;
					mmc.mmc_data = (uint8_t *)s_addr;
					s_addr = 0;		/* reset starting address */
				} else {
					mmc.mmc_data = (uint8_t *)mmc_mphys( ccb, sge->cam_sg_address, sgi );
					d_len	     = sge->cam_sg_count;
				}

				mmc.mmc_arg		= blkno;
				mmc.mmc_data_len	= d_len;
				if (d_len == ext->mmc_blksize)
					mmc.mmc_cmd     = dir == CAM_DIR_IN ? MMC_READ_SINGLE_BLOCK : MMC_WRITE_BLOCK;
				else
					mmc.mmc_cmd     = dir == CAM_DIR_IN ? MMC_READ_MULTIPLE_BLOCK : MMC_WRITE_MULTIPLE_BLOCK;

				if (mmc_io(ext->bshdl, &mmc)) {
					mmc_error(hba, mmc.mmc_status);
					return;
				}

				blkno += d_len / ext->mmc_blksize;
			}
		}
	} else {
		if (ext->eflags & MMC_EFLAG_NODMA) {
			mmc.mmc_data	= (_uint8 *)ccb->cam_data.cam_data_ptr;
			mmc.mmc_flags	= MMCIO_VIRT;
		} else
			mmc.mmc_data	= (_uint8 *)mmc_mphys( ccb, ccb->cam_data.cam_data_ptr, 0 );
		mmc.mmc_arg			= blkno;
		mmc.mmc_data_len	= ccb->cam_dxfer_len;
		if (ccb->cam_dxfer_len == ext->mmc_blksize)
			mmc.mmc_cmd     = dir == CAM_DIR_IN ? MMC_READ_SINGLE_BLOCK : MMC_WRITE_BLOCK;
		else
			mmc.mmc_cmd     = dir == CAM_DIR_IN ? MMC_READ_MULTIPLE_BLOCK : MMC_WRITE_MULTIPLE_BLOCK;

		if (mmc_io(ext->bshdl, &mmc)) {
			mmc_error(hba, mmc.mmc_status);
			return;
		}
	}

	ccb->cam_ch.cam_status	= CAM_REQ_CMP;
	ccb->cam_scsi_status	= SCS_GOOD;
}

// interpret SCSI commands
void mmc_io_cmds( SIM_HBA *hba )
{
	SIM_MMC_EXT	*ext = (SIM_MMC_EXT *)hba->ext;
	CCB_SCSIIO	*ccb = ext->nexus;

	switch( ccb->cam_cdb_io.cam_cdb_bytes[0] ) {
		case SC_UNIT_RDY:
			mmc_unit_ready( hba );
			break;

		case SC_INQUIRY:
			mmc_inquiry( hba );
			break;

		case SC_RD_CAP:
			mmc_capacity( hba );
			break;

		case SC_SPINDLE:
			mmc_spindle( hba );
			break;

		case SC_SYNC:
			ccb->cam_ch.cam_status  = CAM_REQ_CMP;
			ccb->cam_scsi_status    = SCS_GOOD;
			break;

		case SC_WRITE10:
			mmc_rw( hba, CAM_DIR_OUT );
			break;

		case SC_READ10:
			mmc_rw( hba, CAM_DIR_IN );
			break;

		default:
			mmc_sense( hba, SK_ILLEGAL, 0x24, 0 );
			break;
	}
}

// start executing a new CCB
void mmc_start_ccb( SIM_HBA *hba )
{
	SIM_MMC_EXT		*ext;
	CCB_SCSIIO		*ccb;

	ext = (SIM_MMC_EXT *)hba->ext;
	if( ext->nexus && !( simq_ccb_state( ext->nexus, SIM_CCB_QUERY ) & SIM_CCB_ABORT ) ) {
		return;
	}

	if( ( ext->nexus = ccb = simq_ccb_dequeue( hba->simq ) ) == NULL ) {
		return;
	}

	if( ccb->cam_ch.cam_flags & CAM_DATA_PHYS ) {
		ccb->cam_ch.cam_status = CAM_PROVIDE_FAIL;
		ext->nexus = NULL;
		simq_post_ccb( hba->simq, ccb );
		return;
	}

	switch( ccb->cam_ch.cam_func_code ) {
		case XPT_SCSI_IO:
				// check for direct I/O
			if( ccb->cam_ch.cam_flags & CAM_DATA_PHYS ) {
				ccb->cam_ch.cam_status	=  CAM_PROVIDE_FAIL;
				break;
			}
			mmc_io_cmds( hba );
			break;

		case XPT_DEVCTL:
			mmc_devctl( hba, (CCB_DEVCTL *)ccb);
			break;

		default:
			slogf (_SLOGC_SIM_MMC, _SLOG_ERROR,
					"mmc_start_ccb: unsupported function code %d", ccb->cam_ch.cam_func_code );
			ccb->cam_ch.cam_status	= CAM_REQ_CMP_ERR;
			ccb->cam_scsi_status	= SCS_CHECK;
			break;
	}

	if( ccb->cam_ch.cam_status != CAM_REQ_INPROG ) {
		ext->nexus = NULL;
		simq_post_ccb( hba->simq, ccb );
	}
}

// the driver thread starts here
void mmc_pulse_handler( SIM_HBA *hba )
{
	struct _pulse	pulse;
	iov_t			iov;
	int				rcvid;
	SIM_MMC_EXT     *ext = (SIM_MMC_EXT *)hba->ext;
	
	SETIOV( &iov, &pulse, sizeof( pulse ) );

	while( 1 ) {
		if( ( rcvid = MsgReceivev( hba->chid, &iov, 1, NULL ) ) == -1 ) {
			continue;
		}
		switch( pulse.code ) {
			case SIM_ENQUEUE:
				mmc_start_ccb( hba );
				break;

			default:
				if( rcvid ) {
					MsgReplyv( rcvid, ENOTSUP, &iov, 1 );
				}
				break;
		}
		if( ext->nexus == NULL ) {
			mmc_start_ccb( hba );
		}
	}
}

void *mmc_driver_thread( void *data )
{
	mmc_pulse_handler( (SIM_HBA *)data );
	return( 0 );
}

__SRCVERSION("sim_mmc.c $Rev: 168632 $");
