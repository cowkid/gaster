#include "blib.h"

/* A few things NOT documented on rawr.ws:
 * Chunk names are CAPITALIZED.
 * FORM, GEN8, OPTN, etc.
 *
 * GM:S data is actually very IFF-like. Look it up. */

/* String
    StringAddress : Int32 //The value is an ASCIIString
*/

/* Chunk
    Name : ChunkName
    Length : Int32
*/

struct gms_chunk {
	char name[4];
	int32_t length;
} __attribute__((packed));

/*     Sprt : ListChunk<Sprite>
        Name : String
        Width : Int32
        Height : Int32
        Unknown : Byte[56]
        TextureCount : Int32
        TextureAddresses : Int32[TextureCount]
        Unknown : Byte[] //Until next object
*/

struct gms_sprite {
	uint32_t name;
	int32_t width;
	int32_t height;
	uint8_t unknown[56];
	int32_t texture_count;
} __attribute__((packed));

struct gms_background {
	uint32_t name;
	int32_t unknown[3];
	int32_t offset;
} __attribute__((packed));

struct gms_script_def {
	uint32_t name;
	int32_t id;
} __attribute__((packed));

struct gms_object_def {
	uint32_t name;
	int32_t id;
} __attribute__((packed));

struct gms_room {
	uint32_t name;
	int32_t ukn1;
	int32_t width;
	int32_t height;
	int32_t ukn2;
	int32_t ukn3;
	uint32_t argb;
	uint8_t ukn4[60];
} __attribute__((packed));

struct gms_room_bgnd {
	int32_t enabled; // bool 4
	int32_t ukn1; // 8
	int32_t bgnd_index; // 12
	int32_t x; // 16
	int32_t y; // 20
	int32_t tile_x; // bool 24
	int32_t tile_y; // bool 28
	uint8_t ukn2[12];
} __attribute__((packed)); // 40 bytes

struct gms_room_view {
	int32_t enabled; // 4
	int32_t view_x; // 8
	int32_t view_y; // 12
	int32_t view_width; // 16
	int32_t view_height; // 20
	int32_t port_x; // 24
	int32_t port_y; // 28
	int32_t port_width; // 32
	int32_t port_height; // 36
	uint8_t unk1[20];
} __attribute__((packed)); // 56 bytes

struct gms_room_gameobj {
	int32_t x;
	int32_t y;
	int32_t def_index;
	int32_t ukn1;
	int32_t ukn2;
	float scale_x;
	float scale_y;
	float tint;
	int32_t ukn3;
} __attribute__((packed));

struct gms_room_tiles {
	int32_t x;
	int32_t y;
	int32_t def_index;
	int32_t source_x;
	int32_t source_y;
	int32_t width;
	int32_t height;
	int32_t ukn1;
	int32_t ukn2;
	float scale_x;
	float scale_y;
	float tint;
} __attribute__((packed));

struct gms_texture {
	int16_t x;
	int16_t y;
	int16_t width;
	int16_t height;
	int16_t render_x;
	int16_t render_y;
	int16_t unk1;
	int16_t unk2;
	int16_t unk3;
	int16_t unk4;
	int16_t spritesheet_id;
} __attribute__((packed));
