/*
 * $QNXLicenseC: 
 * Copyright 2007, 2008, QNX Software Systems.  
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


/*
 *  cdrom.h
 *
 */
#ifndef _CDROM_H_INCLUDED
#define _CDROM_H_INCLUDED

#include <_pack64.h>

typedef union _cdrom_absaddr {
    struct {
        unsigned char   reserved1;
        unsigned char   minute;
        unsigned char   second;
        unsigned char   frame;
    }					msf;
    unsigned long       lba;
} cdrom_absaddr_t;

typedef struct _cdrom_playmsf {
    unsigned char       start_minute;       /* starting minute */
    unsigned char       start_second;       /* starting second */
    unsigned char       start_frame;        /* starting frame  */
    unsigned char       end_minute;         /* ending minute */
    unsigned char       end_second;         /* ending second */
    unsigned char       end_frame;          /* ending frame  */
} cdrom_playmsf_t;

typedef struct _cdrom_playti {
    unsigned char       start_track;        /* starting track */
    unsigned char       start_index;        /* starting index */
    unsigned char       end_track;          /* ending track */
    unsigned char       end_index;          /* ending index */
} cdrom_playti_t;

#define CDROM_ADR_NOT_SUPPLIED      0x00
#define CDROM_ADR_CURRENT_POSITION  0x01
#define CDROM_ADR_MEDIA_CATALOGUE   0x02
#define CDROM_ADR_ISRC              0x03

#define CDROM_CTRL_AUDIO_PREEMPHASIS    0x01
#define CDROM_CTRL_COPY_PERMITTED       0x02
#define CDROM_CTRL_DATA_TRACK           0x04
#define CDROM_CTRL_FOUR_CHANNEL         0x08

#define CDROM_MAX_TRACKS    100

typedef struct _cdrom_tocentry {
    unsigned char       reserved1;
    unsigned char       control_adr;
    unsigned char       track_number;
	unsigned char       reserved2;
    cdrom_absaddr_t     addr;
} cdrom_tocentry_t;

typedef struct _cdrom_read_toc {
    unsigned short      length;
    unsigned char       first_track;
    unsigned char       last_track;
    cdrom_tocentry_t    toc_entry[CDROM_MAX_TRACKS];
} cdrom_read_toc_t;

typedef struct _cdrom_read_multisession {
    unsigned short      length;
    unsigned char       first_session;
    unsigned char       last_session;
    cdrom_tocentry_t    ms_entry;
} cdrom_read_multisession_t;

#define CDROM_LBA           0x01
#define CDROM_MSF           0x02

#define CDROM_DATA_TRACK    0x04

#define CDROM_LEADOUT       0xAA

#define CDROM_SUBCH_DATA                0x00
#define CDROM_SUBCH_CURRENT_POSITION    0x01
#define CDROM_SUBCH_MEDIA_CATALOG       0x02
#define CDROM_SUBCH_ISRC                0x03

typedef struct _cdrom_read_sub_channel {
    unsigned char    data_format;
    unsigned char    track_number;
} cdrom_read_subch_t;

/*
 * Definition for audio_status returned from Read Sub-channel
 */
#define CDROM_AUDIO_INVALID     0x00    /* audio status not supported */
#define CDROM_AUDIO_PLAY        0x11    /* audio play operation in progress */
#define CDROM_AUDIO_PAUSED      0x12    /* audio play operation paused */
#define CDROM_AUDIO_COMPLETED   0x13    /* audio play successfully completed */
#define CDROM_AUDIO_ERROR       0x14    /* audio play stopped due to error */
#define CDROM_AUDIO_NO_STATUS   0x15    /* no current audio status to return */

typedef struct _sub_channel_header {
    unsigned char   reserved;
    unsigned char   audio_status;
    unsigned short  data_length;
} subch_header_t;

typedef struct _sub_channel_current_position {
    subch_header_t          header;
    unsigned char           data_format;
    unsigned char           control_adr;
    unsigned char           track_number;
    unsigned char           index_number;
    cdrom_absaddr_t         addr;              /* absolute address */
    cdrom_absaddr_t         raddr;             /* relative address */
} subch_current_position_t;

typedef struct _sub_channel_media_catalog {
    subch_header_t          header;
    unsigned char           data_format;
    unsigned char           reserved1;
    unsigned char           reserved2;
    unsigned char           reserved3;
    unsigned char           reserved4       :7,
                            mcval           :1;
    unsigned char           media_catalog[15];
} subch_media_catalog_t;

typedef struct _sub_channel_track_isrc {
    subch_header_t          header;
    unsigned char           data_format;
    unsigned char           track_number;
    unsigned char           reserved2;
    unsigned char           reserved3;
    unsigned char           reserved4       :7,
                            tcval           :1;
    unsigned char           isrc[15];
} subch_track_isrc_t;

typedef union _cdrom_subch_data {
	cdrom_read_subch_t			subch_command;
    subch_current_position_t    current_position;
    subch_media_catalog_t       media_catalog;
    subch_track_isrc_t          track_isrc;
} cdrom_subch_data_t;
	
typedef struct _cdrom_volume {
	unsigned char     volume[4];
} cdrom_volume_t;

typedef struct _cdrom_speed {
	unsigned short       speed;
} cdrom_speed_t;

/*
 CDROM Raw Read Track modes                     Frame Layout
Red Book (CD-DA)              |                      Data                     |
                              |                      2352                     |

Yellow Book (Mode 1)          | Sync | Header |      Data     | EDC | 0 | ECC |
                              |  12  |   4    |      2048     |  4  | 8 | 276 |

Yellow Book (Mode 2)          | Sync | Header |            Data               |
                              |  12  |   4    |            2336               |

Green Book (XA Mode 2 Form 1) | Sync | Header | SubHeader | Data | EDC | ECC  |
                              |  12  |   4    |     8     | 2048 |  4  | 276  |

Green Book (XA Mode 2 Form 2) | Sync | Header | SubHeader |    Data   | Spare |
                              |  12  |   4    |     8     |    2324   |   4   |
*/

#define CDROM_SYNC_SIZE         12
#define CDROM_HEADER_SIZE        4
#define CDROM_SUBHEADER_SIZE     8
#define CDROM_EDC_SIZE           4
#define CDROM_ECC_SIZE         276
#define CDROM_ZERO_SIZE          8
#define CDROM_SPARE_SIZE         4

#define CDROM_CDDA_FRAME_SIZE			2352 /* Data */
#define CDROM_YELLOW_MODE2_FRAME_SIZE	2352 /* Sync + Header + Data */
#define CDROM_XA_FORM2_FRAME_SIZE		2352 /* Header + SubHeader + Data */
#define CDROM_RAW_FRAME_SIZE			2352
#define CDROM_COOKED_FRAME_SIZE			2048

#define CDROM_EST_CDDA			0x00
#define CDROM_EST_YELLOW_MODE2	0x02
#define CDROM_EST_XA_FORM2		0x04
typedef struct _cdrom_raw_read {
    unsigned long        lba;
    unsigned long        nsectors;
    unsigned long        est;           /* expected sector type */
} cdrom_raw_read_t;

#define MSF_S_DFLT		0x3C
#define MSF_F_DFLT		0x4B
typedef struct _cdrom_param {
    unsigned char       multiplier;		/* Inactivity Time Multiplier ( 0 - 0xf */
    unsigned short      msf_s;			/* Number of MSF - S Units per MSF - M Unit */
    unsigned short      msf_f;			/* Number of MSF - F Units per MSF - S Unit */
} cdrom_param_t;

#define CDROM_ADR(_x)                   (((_x) >> 4) & 0xf)
#define CDROM_CONTROL(_x)               ((_x) & 0xf)

#define MSF2LBA(_min, _sec, _frame)     (((_min) * 60 + (_sec)) * 75 + (_frame) - 150)
#define LBA2MIN(_lba)                   ((((_lba) + 150) / 75) / 60)
#define LBA2SEC(_lba)                   ((((_lba) + 150) / 75) % 60)
#define LBA2FRAME(_lba)                 (((_lba) + 150) % 75)

#define CDROM_EXCHANGE_EJECT        0x0
#define CDROM_EXCHANGE_RELOAD       0x1
#define CDROM_EXCHANGE_UNLOAD       0x2
#define CDROM_EXCHANGE_LOAD         0x3
#define CDROM_EXCHANGE_INITIALIZE   0xf
typedef struct _cdrom_exchange {
	unsigned long	operation;
	unsigned long	slot;
	unsigned long	rsvd;
} cdrom_exchange_t;

#define CDROM_MAX_SLOTS		255
#define CDROM_MSH_CHANGER_FAULT			(0x1 << 7)
#define CDROM_MSH_CHANGER_READY			(0x0 << 5)
#define CDROM_MSH_CHANGER_LOADING		(0x1 << 5)
#define CDROM_MSH_CHANGER_UNLOADING		(0x2 << 5)
#define CDROM_MSH_CHANGER_INITIALIZING	(0x3 << 5)
#define CDROM_MSH_MECHANISM_DOOR_OPEN	(0x1 << 4)
#define CDROM_MSH_MECHANISM_IDLE		(0x0 << 5)
#define CDROM_MSH_MECHANISM_AUDIO		(0x1 << 5)
#define CDROM_MSH_MECHANISM_AUDIO_SCAN	(0x2 << 5)
#define CDROM_MSH_MECHANISM_HOST		(0x3 << 5)
#define CDROM_MSH_MECHANISM_UNKNOWN		(0x7 << 5)
typedef struct _cdrom_mechanism_status_header {
    unsigned char       changer_state_slot;
    unsigned char       mech_state;
    unsigned char       current_lba[3];
    unsigned char       num_slots_avail;
    unsigned short      slot_table_len;
} cdrom_mechanism_status_header_t;

#define CDROM_STR_DISC_PRESENT  0x80
#define CDROM_STR_DISC_CHANGED  0x01
typedef struct _cdrom_slot_table_response {
    unsigned char       flags;
    unsigned char       rsvd[3];
} cdrom_slot_table_response_t;

typedef struct _cdrom_mechanism_status {
	cdrom_mechanism_status_header_t   hdr;
	cdrom_slot_table_response_t       str[CDROM_MAX_SLOTS];
} cdrom_mechanism_status_t;

#define CDROM_MAX_TEXT		255

/* Data pack types */
#define CDROM_DPT_TITLE       0x80    /* Title of album name or track titles */
#define CDROM_DPT_PERFORMER   0x81    /* Name(s) of performer(s) */
#define CDROM_DPT_SONGWRITER  0x82    /* Name(s) of songwriter(s) */
#define CDROM_DPT_COMPOSER    0x83    /* Name(s) of composer(s) */
#define CDROM_DPT_ARRANGER    0x84    /* Name(s) of arranger(s) */
#define CDROM_DPT_MESSAGE     0x85    /* Message(s) from content provider and or artist */
#define CDROM_DPT_IDENT       0x86    /* Disc identification information */
#define CDROM_DPT_GENRE       0x87    /* Genre identification and genre information */
#define CDROM_DPT_TOC         0x88    /* Table of content information */
#define CDROM_DPT_TOC2        0x89    /* Second Table of content information */
#define CDROM_DPT_UPCEAN      0x8e    /* UPC/EAN code of the album and ISRC code of each track */
#define CDROM_DPT_SIZEINFO    0x8f    /* Size information of the block */
typedef struct _cdrom_datapack {
	unsigned char	pack_type;
	unsigned char	trk;
	unsigned char	seq;
	unsigned char	blk_char;
	char			data[12];
	unsigned char	crc0;
	unsigned char	crc1;
} cdrom_datapack_t;

typedef struct _cdrom_cd_text {
    unsigned short      npacks;               /* number of descriptors */
	char				rsvd[2];
 	cdrom_datapack_t	packs[CDROM_MAX_TEXT];
} cdrom_cd_text_t;

typedef struct _cdrom_scan {
    cdrom_absaddr_t     addr;
#define CDROM_SCAN_DIR_FORWARD		0x00
#define CDROM_SCAN_DIR_REVERSE		0x10
	unsigned char		direction;
#define CDROM_SCAN_TYPE_LBA		0x00
#define CDROM_SCAN_TYPE_MSF		0x40
#define CDROM_SCAN_TYPE_TRK		0x80
#define CDROM_SCAN_TYPE_MSK		0xc0
	unsigned char		type;
	unsigned char		speed;
	unsigned char		rsvd[9];
} cdrom_scan_t;

#include <_packpop.h>

#endif
