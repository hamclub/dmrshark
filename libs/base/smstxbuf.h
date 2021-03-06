/*
 * This file is part of dmrshark.
 *
 * dmrshark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dmrshark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dmrshark.  If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef SMSTXBUF_H_
#define SMSTXBUF_H_

#include "dmr.h"

#include <libs/dmrpacket/dmrpacket-data.h>
#include <libs/comm/repeaters.h>
#include <libs/aprs/aprs.h>

#include <time.h>

typedef struct smstxbuf_st {
	char msg[DMRPACKET_MAX_FRAGMENTSIZE];
	time_t added_at;
	uint8_t send_tries;
	uint8_t selective_ack_tries;
	time_t waiting_for_tms_ack_started_at;
	uint8_t delay_before_send_sec;

	dmr_data_type_t data_type;
	repeater_t *repeater;
	dmr_timeslot_t ts;
	dmr_call_type_t call_type;
	dmr_id_t dst_id;
	dmr_id_t src_id;
	unsigned int db_id; // Used to associate remotedb msg queue entries.
	aprs_msg_t *aprs_msg; // Holds the APRS message which initiated the DMR SMS sending.

	struct smstxbuf_st *next;
} smstxbuf_t;

void smstxbuf_print_entry(smstxbuf_t *entry);
void smstxbuf_print(void);
void smstxbuf_add(uint8_t delay_before_send_sec, repeater_t *repeater, dmr_timeslot_t ts, dmr_call_type_t calltype, dmr_id_t dstid, dmr_data_type_t data_type, char *msg, unsigned int db_id, aprs_msg_t *aprs_msg);

void smstxbuf_first_entry_sent_successfully(repeater_t *repeater);
void smstxbuf_first_entry_waiting_for_tms_ack_started(void);
smstxbuf_t *smstxbuf_get_first_entry(void);
void smstxbuf_free_entry(smstxbuf_t *entry);

void smstxbuf_process(void);
void smstxbuf_deinit(void);

#endif
