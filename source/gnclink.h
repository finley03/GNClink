#ifndef GNCLINK_H
#define GNCLINK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gnclink_standard.h"

// Returns pointer to payload section of packet buffer.
// Maximum payload size is defined by GNCLINK_MAX_PACKET_PAYLOAD_LENGTH.
uint8_t* GNClink_Get_Packet_Payload_Pointer(uint8_t* packetPointer);

// Constructs packet header and footer around payload.
// Payload must not be edited after this is called.
// Returns false if an error is encountered.
bool GNClink_Construct_Packet(uint8_t* packetPointer, GNClink_PacketType packetType, GNClink_PacketFlags packetFlags, int payloadSize);

// Checks packet header and footer values are within bounds
// and agree with the CRC.
bool GNClink_Check_Packet(uint8_t* packetPointer);

// Get packet type
GNClink_PacketType GNClink_Get_Packet_Type(uint8_t* packetPointer);

// Get packet flags
GNClink_PacketFlags GNClink_Get_Packet_Flags(uint8_t* packetPointer);

// Get packet payload size
int GNClink_Get_Packet_Payload_Size(uint8_t* packetPointer);

// Returns pointer to payload section of packet buffer.
// Maximum payload size is defined by GNCLINK_MAX_PACKET_PAYLOAD_LENGTH.
uint8_t* GNClink_Get_Frame_Payload_Pointer(uint8_t* framePointer);

// Gets frame at index from given packet.
// Puts result at the framePointer.
// frameIndex gives the frame number.
// moreFrames is populated with true if there are more frames to be
// consructed for this packet.
bool GNClink_Get_Frame(uint8_t* packetPointer, uint8_t* framePointer, GNClink_FrameFlags flags, int frameIndex, bool* moreFrames);

// Checks frame header and footer values are within bounds
// and agree with the CRC.
bool GNClink_Check_Frame(uint8_t* framePointer);

// Checks if frame is requesting a resend
bool GNClink_Frame_RequestResend(uint8_t* framePointer);

// Get frame index
int GNClink_Get_Frame_Index(uint8_t* framePointer);

// Construct resend request frame
// Payload is assumed to already be populated
void GNClink_Construct_RequestResendFrame(uint8_t* framePointer);

// Reconstucts packet from frames.
// Also checks frames, returns false if invalid
bool GNClink_Reconstruct_Packet_From_Frames(uint8_t* framePointer, uint8_t* packetPointer, bool* moreFrames);

#ifdef __cplusplus
}
#endif

#endif