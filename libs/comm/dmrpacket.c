#include <config/defaults.h>

#include "dmrpacket.h"

#include <libs/daemon/console.h>

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

typedef struct __attribute__((packed)) {
	uint16_t port;
	uint8_t reserved1[2];
	uint8_t seq;
	uint8_t reserved2[3];
	uint8_t packet_type;
	uint8_t reserved3[7];
	uint16_t timeslot_raw; // 0x1111 if TS1, 0x2222 if TS2
	uint16_t slot_type;
	uint16_t delimiter; // Always 0x1111.
	uint16_t frame_type;
	uint8_t reserved4[3];
	uint8_t payload[33];
	uint8_t reserved5[2];
	uint8_t calltype; // 0x00 - private call, 0x01 - group call
	uint8_t reserved6;
	uint8_t dst_id_raw1;
	uint8_t dst_id_raw2;
	uint8_t dst_id_raw3;
	uint8_t reserved7;
	uint8_t src_id_raw1;
	uint8_t src_id_raw2;
	uint8_t src_id_raw3;
} dmr_packet_raw_t;

#define DMR_PACKET_SIZE 72

char *dmrpacket_get_readable_packet_type(dmr_packet_type_t packet_type) {
	switch (packet_type) {
		case DMRPACKET_PACKET_TYPE_VOICE: return "voice";
		case DMRPACKET_PACKET_TYPE_START_OF_TRANSMISSION: return "sync/start of transmission";
		case DMRPACKET_PACKET_TYPE_END_OF_TRANSMISSION: return "end of transmission";
		case DMRPACKET_PACKET_TYPE_HYTERA_DATA: return "hytera data";
		default: return "unknown";
	}
}

char *dmrpacket_get_readable_slot_type(dmr_slot_type_t slot_type) {
	switch (slot_type) {
		case DMRPACKET_SLOT_TYPE_CALL_START: return "call start";
		case DMRPACKET_SLOT_TYPE_START: return "start";
		case DMRPACKET_SLOT_TYPE_CALL_END: return "call end";
		case DMRPACKET_SLOT_TYPE_CSBK: return "csbk"; // Control Signaling Block
		case DMRPACKET_SLOT_TYPE_DATA_HEADER: return "data header";
		case DMRPACKET_SLOT_TYPE_1_2_RATE_DATA: return "1/2 rate data";
		case DMRPACKET_SLOT_TYPE_3_4_RATE_DATA: return "3/4 rate data";
		case DMRPACKET_SLOT_TYPE_VOICE_DATA_1: return "voice data 1";
		case DMRPACKET_SLOT_TYPE_VOICE_DATA_2: return "voice data 2";
		case DMRPACKET_SLOT_TYPE_VOICE_DATA_3: return "voice data 3";
		case DMRPACKET_SLOT_TYPE_VOICE_DATA_4: return "voice data 4";
		case DMRPACKET_SLOT_TYPE_VOICE_DATA_5: return "voice data 5";
		case DMRPACKET_SLOT_TYPE_VOICE_DATA_6: return "voice data 6";
		default: return "unknown";
	}
}

char *dmrpacket_get_readable_frame_type(dmr_frame_type_t frame_type) {
	switch (frame_type) {
		case DMRPACKET_FRAME_TYPE_GENERAL: return "general";
		case DMRPACKET_FRAME_TYPE_VOICE_SYNC: return "voice sync";
		case DMRPACKET_FRAME_TYPE_DATA_START: return "data start";
		case DMRPACKET_FRAME_TYPE_VOICE: return "voice";
		default: return "unknown";
	}
}

char *dmrpacket_get_readable_call_type(dmr_call_type_t call_type) {
	switch (call_type) {
		case DMRPACKET_CALL_TYPE_PRIVATE: return "private";
		case DMRPACKET_CALL_TYPE_GROUP: return "group";
		default: return "unknown";
	}
}

// Decodes the UDP packet given in udp_packet to dmr_packet,
// returns 1 if decoding was successful, otherwise returns 0.
flag_t dmrpacket_decode(struct udphdr *udp_packet, dmr_packet_t *dmr_packet) {
	dmr_packet_raw_t *dmr_packet_raw = (dmr_packet_raw_t *)((uint8_t *)udp_packet + sizeof(struct udphdr));
	int dmr_packet_raw_length = 0;
	int i;
	loglevel_t loglevel;

	// Length in UDP header contains length of the UDP header too, so we are substracting it.
	dmr_packet_raw_length = ntohs(udp_packet->len)-sizeof(struct udphdr);
	if (dmr_packet_raw_length != DMR_PACKET_SIZE) {
		//console_log(LOGLEVEL_DEBUG "dmrpacket: decode failed, packet size not %u bytes.\n", DMR_PACKET_SIZE);
		return 0;
	}

	loglevel = console_get_loglevel();
	if (loglevel.flags.debug && loglevel.flags.comm_dmr) {
		console_log(LOGLEVEL_DEBUG "dmrpacket: decoding: ");
		for (i = 0; i < dmr_packet_raw_length; i++)
			console_log(LOGLEVEL_DEBUG "%.2x ", *((uint8_t *)dmr_packet_raw+i));
		console_log(LOGLEVEL_DEBUG "\n");
	}

	/*if (dmr_packet_raw->delimiter != 0x1111) {
		console_log(LOGLEVEL_DEBUG "dmrpacket: decode failed, delimiter mismatch (it's %.4x, should be 0x1111)\n",
			dmr_packet_raw->delimiter);
		return 0;
	}*/

	dmr_packet->packet_type = dmr_packet_raw->packet_type;
	if (dmr_packet_raw->timeslot_raw == 0x1111)
		dmr_packet->timeslot = 1;
	else if (dmr_packet_raw->timeslot_raw == 0x2222)
		dmr_packet->timeslot = 2;
	else {
		console_log(LOGLEVEL_DEBUG "dmrpacket: decode failed, invalid timeslot (%.4x)\n", dmr_packet_raw->timeslot_raw);
		return 0;
	}

	dmr_packet->slot_type = dmr_packet_raw->slot_type;
	dmr_packet->frame_type = dmr_packet_raw->frame_type;
	dmr_packet->call_type = dmr_packet_raw->calltype;
	dmr_packet->dst_id = dmr_packet_raw->dst_id_raw3 << 16 | dmr_packet_raw->dst_id_raw2 << 8 | dmr_packet_raw->dst_id_raw1;
	dmr_packet->src_id = dmr_packet_raw->src_id_raw3 << 16 | dmr_packet_raw->src_id_raw2 << 8 | dmr_packet_raw->src_id_raw1;

	return 1;
}

flag_t dmrpacket_heartbeat_decode(struct udphdr *udp_packet) {
	uint8_t heartbeat[] = { 0x00, 0x00, 0x00, 0x14 };

	if (memcmp((uint8_t *)udp_packet + sizeof(struct udphdr) + 5, heartbeat, sizeof(heartbeat)) == 0)
		return 1;
	return 0;
}
