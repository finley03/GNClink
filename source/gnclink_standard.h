#ifndef GNCLINK_STANDARD_H
#define GNCLINK_STANDARD_H

#ifdef __cplusplus
extern "C" {
#endif

// GNClink protocol specification

// DESCRIPTION

// The GNClink protocol is a serial protocol for communication between a
// computer and flight control boards running GNC firmware
// It can be used over either a wired or wireless connection

// Each packet executes one command or indicates a response,
// such as retreiving a value or acknowledging an operation.
// Packets have a maximum length of 64 bytes including the
// header and footer.

// For transmission, each packet is broken down into a number
// of frames. Each frame has a fixed length and can take a
// payload size of 16 bytes, so each packet is broken down and
// sent in between 1 and 4 frames. When the final frame is
// detected to have arrived, if any frames did not pass checks
// a frame should be with the request resend flag and the payload
// should specify the frames to be resent. When the frames are
// resent, the final frame should have the transaction end flag.

#include <stdint.h>
#include <stdbool.h>

#ifdef __GNUC__
#define __GNCLINK_PACKED __attribute__((packed))
#else
#define __GNCLINK_PACKED
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

// PACKET DEFINITIONS

typedef enum {
	// Request previous packet to be resent.
	// No payload.
	GNClink_PacketType_RequestResend,
	// Request global hash.
	// No payload.
	GNClink_PacketType_GetGlobalHash,
	// Contains response to GetGlobalHash.
	// Payload contains the global has as a 32 bit unsigned integer.
	GNClink_PacketType_GetGlobalHashResponse,
	// Request a list of values to be returned.
	// Payload should contain an 8 bit unsigned integer representing the
	// number of IDs in the list, and a contiguous list of value IDs, encoded
	// as 16 bit unsigned integers.
	GNClink_PacketType_GetValueList,
	// Contains response to GetValueList.
	// Payload contains the list of values arranged contiguously
	// no matter their size.
	GNClink_PacketType_GetValueListResponse,
	GNClink_PacketType_SetValueList,
	GNClink_PacketType_SetValueListResponse,
	GNClink_PacketType_SaveValueList,
	GNClink_PacketType_SaveValueListResponse,
	GNClink_PacketType_LoadValueList,
	GNClink_PacketType_LoadValueListResponse
} GNClink_PacketType;

typedef enum {
	GNClink_PacketFlags_None = 0, // no flags
	GNClink_PacketFlags_NoResponse = 1 << 0 // no response is expected for this packet no matter the error state
} GNClink_PacketFlags;

typedef struct __GNCLINK_PACKED {
	uint8_t magic;
	uint8_t packetType;
	uint8_t packetFlags;
	uint8_t packetLength;
} GNClink_PacketHeader;

typedef struct __GNCLINK_PACKED {
	uint8_t crc;
} GNClink_PacketFooter;

#define GNCLINK_PACKET_MAGIC 0xAC
#define GNCLINK_PACKET_HEADER_LENGTH (sizeof(GNClink_PacketHeader))
#define GNCLINK_PACKET_FOOTER_LENGTH (sizeof(GNClink_PacketFooter))
#define GNCLINK_PACKET_MAX_TOTAL_LENGTH 64
#define GNCLINK_PACKET_MAX_PAYLOAD_LENGTH (GNCLINK_PACKET_MAX_TOTAL_LENGTH - GNCLINK_PACKET_HEADER_LENGTH - GNCLINK_PACKET_FOOTER_LENGTH)
#define GNCLINK_PACKET_MIN_PAYLOAD_LENGTH 0
#define GNCLINK_PACKET_MIN_TOTAL_LENGTH (GNCLINK_PACKET_HEADER_LENGTH + GNCLINK_PACKET_FOOTER_LENGTH + GNCLINK_PACKET_MIN_PAYLOAD_LENGTH)

// FRAME DEFINITIONS

typedef enum {
	GNClink_FrameFlags_None = 0, // no flags
	GNClink_FrameFlags_NoResponse = 1 << 0, // no response is expected for this frame no matter the error state
	GNClink_FrameFlags_TransactionEnd = 1 << 1, // final frame in a transaction, responses should follow
	GNClink_FrameFlags_RequestResend = 1 << 2 // rerequest a specific number of frame indexes
} GNClink_FrameFlags;

typedef struct __GNCLINK_PACKED {
	uint8_t magic;
	uint8_t index;
	uint8_t flags;
} GNClink_FrameHeader;

typedef struct __GNCLINK_PACKED {
	uint8_t crc;
} GNClink_FrameFooter;

#define GNCLINK_FRAME_MAGIC 0xAE
#define GNCLINK_FRAME_HEADER_LENGTH (sizeof(GNClink_FrameHeader))
#define GNCLINK_FRAME_FOOTER_LENGTH (sizeof(GNClink_FrameFooter))
#define GNCLINK_FRAME_PAYLOAD_LENGTH 16
#define GNCLINK_FRAME_TOTAL_LENGTH (GNCLINK_FRAME_PAYLOAD_LENGTH + GNCLINK_FRAME_HEADER_LENGTH + GNCLINK_FRAME_FOOTER_LENGTH)

typedef struct __GNCLINK_PACKED {
	uint8_t resendCount;
	uint8_t resendIndexes[GNCLINK_FRAME_PAYLOAD_LENGTH - 1];
} GNClink_FramePayload_RequestResend;

#define GNCLINK_MAX_FRAMES_PER_PACKET ((GNCLINK_PACKET_MAX_TOTAL_LENGTH+GNCLINK_FRAME_PAYLOAD_LENGTH-1)/GNCLINK_FRAME_PAYLOAD_LENGTH)

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif