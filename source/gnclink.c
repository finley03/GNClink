#include "gnclink.h"

const uint8_t CRC8_LUT_L[] = { 0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D };
const uint8_t CRC8_LUT_H[] = { 0x00, 0x70, 0xE0, 0x90, 0xC7, 0xB7, 0x27, 0x57, 0x89, 0xF9, 0x69, 0x19, 0x4E, 0x3E, 0xAE, 0xDE };

uint8_t CRC8(uint8_t* data, int length) {
	uint8_t crc = 0;
	for (int index = 0; index < length; ++index) {
		crc ^= data[index];
		crc = CRC8_LUT_L[crc & 0xF] ^ CRC8_LUT_H[crc >> 4];
	}
	return crc;
}

uint8_t* GNClink_Get_Packet_Payload_Pointer(uint8_t* packetPointer) {
	return packetPointer + GNCLINK_PACKET_HEADER_LENGTH;
}

bool GNClink_Construct_Packet(uint8_t* packetPointer, GNClink_PacketType packetType, GNClink_PacketFlags packetFlags, int payloadSize) {
	// validate inputs
	if (payloadSize < GNCLINK_PACKET_MIN_PAYLOAD_LENGTH || payloadSize > GNCLINK_PACKET_MAX_PAYLOAD_LENGTH) return false;

	// get pointer to header
	GNClink_PacketHeader* header = (GNClink_PacketHeader*)packetPointer;
	// get pointer to footer
	GNClink_PacketFooter* footer = (GNClink_PacketFooter*)(packetPointer + GNCLINK_PACKET_HEADER_LENGTH + payloadSize);

	// populate header
	header->magic = GNCLINK_PACKET_MAGIC;
	header->packetType = packetType;
	header->packetFlags = packetFlags;
	header->packetLength = payloadSize + GNCLINK_PACKET_HEADER_LENGTH + GNCLINK_PACKET_FOOTER_LENGTH;

	// populate footer
	footer->crc = CRC8(packetPointer, GNCLINK_PACKET_HEADER_LENGTH + payloadSize);

	return true;
}

bool GNClink_Check_Packet(uint8_t* packetPointer) {
	// get pointer to header
	GNClink_PacketHeader* header = (GNClink_PacketHeader*)packetPointer;

	// check magic
	if (header->magic != GNCLINK_PACKET_MAGIC) return false;

	// check packet length
	if (header->packetLength < GNCLINK_PACKET_MIN_TOTAL_LENGTH || header->packetLength > GNCLINK_PACKET_MAX_TOTAL_LENGTH) return false;

	// check CRC
	if (CRC8(packetPointer, header->packetLength) != 0) return false;

	// all checks passed, return true
	return true;
}

GNClink_PacketType GNClink_Get_Packet_Type(uint8_t* packetPointer) {
	// get pointer to header
	GNClink_PacketHeader* header = (GNClink_PacketHeader*)packetPointer;

	return (GNClink_PacketType)header->packetType;
}

GNClink_PacketFlags GNClink_Get_Packet_Flags(uint8_t* packetPointer) {
	// get pointer to header
	GNClink_PacketHeader* header = (GNClink_PacketHeader*)packetPointer;

	return (GNClink_PacketType)header->packetFlags;
}


uint8_t* GNClink_Get_Frame_Payload_Pointer(uint8_t* framePointer) {
	return framePointer + GNCLINK_FRAME_HEADER_LENGTH;
}

bool GNClink_Get_Frame(uint8_t* packetPointer, uint8_t* framePointer, GNClink_FrameFlags flags, int frameIndex, bool* moreFrames) {
	// get pointer to packet header
	GNClink_PacketHeader* packetHeader = (GNClink_PacketHeader*)packetPointer;

	// check packet length
	if (packetHeader->packetLength < GNCLINK_PACKET_MIN_TOTAL_LENGTH || packetHeader->packetLength > GNCLINK_PACKET_MAX_TOTAL_LENGTH) return false;

	// calculate number of frames for packet
	int frameCount = (packetHeader->packetLength + GNCLINK_FRAME_PAYLOAD_LENGTH - 1) / GNCLINK_FRAME_PAYLOAD_LENGTH;
	*moreFrames = frameIndex < frameCount - 1;

	// check index is valid
	if (frameIndex < 0 || frameIndex >= frameCount) return false;

	// get pointer to frame header
	GNClink_FrameHeader* header = (GNClink_FrameHeader*)framePointer;
	// get pointer to frame footer
	GNClink_FrameFooter* footer = (GNClink_FrameFooter*)(framePointer + GNCLINK_FRAME_HEADER_LENGTH + GNCLINK_FRAME_PAYLOAD_LENGTH);
	// get pointer to payload section
	uint8_t* payload = framePointer + GNCLINK_FRAME_HEADER_LENGTH;

	// populate header
	header->magic = GNCLINK_FRAME_MAGIC;
	header->index = frameIndex;
	header->flags = flags;
	if (frameIndex + 1 == frameCount) header->flags |= GNClink_FrameFlags_TransactionEnd;

	// populate payload
	int i = 0;
	for (; i < GNCLINK_FRAME_PAYLOAD_LENGTH && i < packetHeader->packetLength - frameIndex * GNCLINK_FRAME_PAYLOAD_LENGTH; ++i) {
		payload[i] = packetPointer[i + GNCLINK_FRAME_PAYLOAD_LENGTH * frameIndex];
	}
	// populate the rest with zeros
	for (; i < GNCLINK_FRAME_PAYLOAD_LENGTH; ++i) {
		payload[i] = 0;
	}

	// populate footer
	footer->crc = CRC8(framePointer, GNCLINK_FRAME_HEADER_LENGTH + GNCLINK_FRAME_PAYLOAD_LENGTH);

	return true;
}

bool GNClink_Check_Frame(uint8_t* framePointer) {
	// get pointer to header
	GNClink_FrameHeader* header = (GNClink_FrameHeader*)framePointer;

	// check magic
	if (header->magic != GNCLINK_FRAME_MAGIC) return false;

	// check index
	if (header->index >= GNCLINK_MAX_FRAMES_PER_PACKET) return false;

	// check CRC
	if (CRC8(framePointer, GNCLINK_FRAME_TOTAL_LENGTH) != 0) return false;

	// all checks passed, return true
	return true;
}

bool GNClink_Frame_RequestResend(uint8_t* framePointer) {
	// get pointer to header
	GNClink_FrameHeader* header = (GNClink_FrameHeader*)framePointer;

	return (header->flags & GNClink_FrameFlags_RequestResend) != 0;
}

int GNClink_Get_Frame_Index(uint8_t* framePointer) {
	// get pointer to header
	GNClink_FrameHeader* header = (GNClink_FrameHeader*)framePointer;

	return (int)header->index;
}

void GNClink_Construct_RequestResendFrame(uint8_t* framePointer) {
	// get pointer to frame header
	GNClink_FrameHeader* header = (GNClink_FrameHeader*)framePointer;
	// get pointer to frame footer
	GNClink_FrameFooter* footer = (GNClink_FrameFooter*)(framePointer + GNCLINK_FRAME_HEADER_LENGTH + GNCLINK_FRAME_PAYLOAD_LENGTH);
	// get pointer to payload section
	uint8_t* payload = framePointer + GNCLINK_FRAME_HEADER_LENGTH;

	// populate header
	header->magic = GNCLINK_FRAME_MAGIC;
	header->index = 0;
	header->flags = GNClink_FrameFlags_RequestResend;

	// populate footer
	footer->crc = CRC8(framePointer, GNCLINK_FRAME_HEADER_LENGTH + GNCLINK_FRAME_PAYLOAD_LENGTH);
}


bool GNClink_Reconstruct_Packet_From_Frames(uint8_t* framePointer, uint8_t* packetPointer, bool* moreFrames) {
	// get pointer to frame header
	GNClink_FrameHeader* header = (GNClink_FrameHeader*)framePointer;
	// get pointer to frame footer
	//GNClink_FrameFooter* footer = (GNClink_FrameFooter*)(framePointer + GNCLINK_FRAME_HEADER_LENGTH + GNCLINK_FRAME_PAYLOAD_LENGTH);
	// get pointer to payload section
	uint8_t* payload = framePointer + GNCLINK_FRAME_HEADER_LENGTH;

	for (int i = 0; i < GNCLINK_FRAME_PAYLOAD_LENGTH && i + header->index * GNCLINK_FRAME_PAYLOAD_LENGTH < GNCLINK_PACKET_MAX_TOTAL_LENGTH; ++i) {
		packetPointer[i + header->index * GNCLINK_FRAME_PAYLOAD_LENGTH] = payload[i];
	}

	*moreFrames = (header->flags & GNClink_FrameFlags_TransactionEnd) == 0;

	return true;
}