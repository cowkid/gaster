#include "blib.h"
#include "gms_types.h"

struct gms_chunk *form, *gen8, *optn, *extn, *sond, *agrp, *sprt, *bgnd, *path, *scpt, *shdr, *font, *tmln, *objt, *room, *dafl, *tpag, *code, *vari, *func, *strg, *txtr, *audo;

#define NORM_INDEX(x) ((uint64_t)x - ((uint64_t)&CH_BUF(0)))

int32_t get_array_length(uint8_t* array) {
	return ((int32_t*)array)[0];
}

uint8_t* get_array_index_from(uint8_t* array, int32_t index) {
	int32_t length = *((int32_t*)array);
	uint32_t* offsets_list = (uint32_t*)(array+4);

	for(int32_t i = 0; i < length; i++) {
		if (i == index) {
			return &CH_BUF( offsets_list[i] );
		}
	}
	return NULL;
}

uint64_t get_chunk_loc(char* name) {
	uint64_t loc = 0;
	search_file_raw(name, 4, &loc);
	return loc;
}

struct gms_chunk* get_chunk(char* name) {
	uint64_t off = get_chunk_loc(name);
	return (struct gms_chunk*)&CH_BUF(off);
}

struct gms_chunk* get_chunk_safe(char* name, uint64_t expect) {
	uint64_t loc = get_chunk_loc(name);

	if (expect != loc) {
		// Abort. Wrongamundo.
		printf("Chunk '%c%c%c%c' not found at expected location %08llx\n", name[0], name[1], name[2], name[3], loc);
		unmap_file();
		exit(1);
	} else {
		printf("Chunk '%c%c%c%c' found @ 0x%08llx\n", name[0], name[1], name[2], name[3], loc);
	}

	return (struct gms_chunk*)&CH_BUF(loc);
}

void map_gms_data(char* name) {
	// Map data.win / data.ios.
	map_file(name, READ_FILE);

	// FORM chunk should be @ 0x00000000.
	struct gms_chunk *form = (struct gms_chunk*)&CH_BUF(0);

	if (!strncmp(form->name, "FORM", 4)) {
		printf("Chunk 'FORM' found; file seems okay. Length is %d\n", form->length);
	} else {
		printf("No Form chunk. Probably not a gm:s datafile.\n");
		unmap_file();
		exit(1);
	}

	// We found the form chunk. Next up, we need all the ancillary chunks.
	uint64_t at_addr = sizeof(struct gms_chunk);

	gen8 = get_chunk_safe("GEN8", at_addr);
	at_addr += sizeof(struct gms_chunk) + gen8->length;
	optn = get_chunk_safe("OPTN", at_addr);
	at_addr += sizeof(struct gms_chunk) + optn->length;
	extn = get_chunk_safe("EXTN", at_addr);
	at_addr += sizeof(struct gms_chunk) + extn->length;
	sond = get_chunk_safe("SOND", at_addr);
	at_addr += sizeof(struct gms_chunk) + sond->length;
	agrp = get_chunk_safe("AGRP", at_addr);
	at_addr += sizeof(struct gms_chunk) + agrp->length;
	sprt = get_chunk_safe("SPRT", at_addr);
	at_addr += sizeof(struct gms_chunk) + sprt->length;
	bgnd = get_chunk_safe("BGND", at_addr);
	at_addr += sizeof(struct gms_chunk) + bgnd->length;
	path = get_chunk_safe("PATH", at_addr);
	at_addr += sizeof(struct gms_chunk) + path->length;
	scpt = get_chunk_safe("SCPT", at_addr);
	at_addr += sizeof(struct gms_chunk) + scpt->length;
	shdr = get_chunk_safe("SHDR", at_addr);
	at_addr += sizeof(struct gms_chunk) + shdr->length;
	font = get_chunk_safe("FONT", at_addr);
	at_addr += sizeof(struct gms_chunk) + font->length;
	tmln = get_chunk_safe("TMLN", at_addr);
	at_addr += sizeof(struct gms_chunk) + tmln->length;
	objt = get_chunk_safe("OBJT", at_addr);
	at_addr += sizeof(struct gms_chunk) + objt->length;
	room = get_chunk_safe("ROOM", at_addr);
	at_addr += sizeof(struct gms_chunk) + room->length;
	dafl = get_chunk_safe("DAFL", at_addr);
	at_addr += sizeof(struct gms_chunk) + dafl->length;
	tpag = get_chunk_safe("TPAG", at_addr);
	at_addr += sizeof(struct gms_chunk) + tpag->length;
	code = get_chunk_safe("CODE", at_addr);
	at_addr += sizeof(struct gms_chunk) + code->length;
	vari = get_chunk_safe("VARI", at_addr);
	at_addr += sizeof(struct gms_chunk) + vari->length;
	func = get_chunk_safe("FUNC", at_addr);
	at_addr += sizeof(struct gms_chunk) + func->length;
	strg = get_chunk_safe("STRG", at_addr);
	at_addr += sizeof(struct gms_chunk) + strg->length;
	txtr = get_chunk_safe("TXTR", at_addr);
	at_addr += sizeof(struct gms_chunk) + txtr->length;
	audo = get_chunk_safe("AUDO", at_addr);
	at_addr += sizeof(struct gms_chunk) + audo->length;
}

void dump_chunks(char* file) {

	printf("Dumping chunks to folder...");
	fflush(stdout);

	FILE* out = fopen("dump/chunk/gen8", "w");
	fwrite(gen8, 1, sizeof(struct gms_chunk)+gen8->length, out);
	fclose(out);

	out = fopen("dump/chunk/optn", "w");
	fwrite(optn, 1, sizeof(struct gms_chunk)+optn->length, out);
	fclose(out);

	out = fopen("dump/chunk/extn", "w");
	fwrite(extn, 1, sizeof(struct gms_chunk)+extn->length, out);
	fclose(out);

	out = fopen("dump/chunk/sond", "w");
	fwrite(sond, 1, sizeof(struct gms_chunk)+sond->length, out);
	fclose(out);

	out = fopen("dump/chunk/agrp", "w");
	fwrite(agrp, 1, sizeof(struct gms_chunk)+agrp->length, out);
	fclose(out);

	out = fopen("dump/chunk/sprt", "w");
	fwrite(sprt, 1, sizeof(struct gms_chunk)+sprt->length, out);
	fclose(out);

	out = fopen("dump/chunk/bgnd", "w");
	fwrite(bgnd, 1, sizeof(struct gms_chunk)+bgnd->length, out);
	fclose(out);

	out = fopen("dump/chunk/path", "w");
	fwrite(path, 1, sizeof(struct gms_chunk)+path->length, out);
	fclose(out);

	out = fopen("dump/chunk/scpt", "w");
	fwrite(scpt, 1, sizeof(struct gms_chunk)+scpt->length, out);
	fclose(out);

	out = fopen("dump/chunk/shdr", "w");
	fwrite(shdr, 1, sizeof(struct gms_chunk)+shdr->length, out);
	fclose(out);

	out = fopen("dump/chunk/font", "w");
	fwrite(font, 1, sizeof(struct gms_chunk)+font->length, out);
	fclose(out);

	out = fopen("dump/chunk/tmln", "w");
	fwrite(tmln, 1, sizeof(struct gms_chunk)+tmln->length, out);
	fclose(out);

	out = fopen("dump/chunk/objt", "w");
	fwrite(objt, 1, sizeof(struct gms_chunk)+objt->length, out);
	fclose(out);

	out = fopen("dump/chunk/room", "w");
	fwrite(room, 1, sizeof(struct gms_chunk)+room->length, out);
	fclose(out);

	out = fopen("dump/chunk/dafl", "w");
	fwrite(dafl, 1, sizeof(struct gms_chunk)+dafl->length, out);
	fclose(out);

	out = fopen("dump/chunk/tpag", "w");
	fwrite(tpag, 1, sizeof(struct gms_chunk)+tpag->length, out);
	fclose(out);

	out = fopen("dump/chunk/code", "w");
	fwrite(code, 1, sizeof(struct gms_chunk)+code->length, out);
	fclose(out);

	out = fopen("dump/chunk/vari", "w");
	fwrite(vari, 1, sizeof(struct gms_chunk)+vari->length, out);
	fclose(out);

	out = fopen("dump/chunk/func", "w");
	fwrite(func, 1, sizeof(struct gms_chunk)+func->length, out);
	fclose(out);

	out = fopen("dump/chunk/strg", "w");
	fwrite(txtr, 1, sizeof(struct gms_chunk)+strg->length, out);
	fclose(out);

	out = fopen("dump/chunk/txtr", "w");
	fwrite(txtr, 1, sizeof(struct gms_chunk)+txtr->length, out);
	fclose(out);

	out = fopen("dump/chunk/audo", "w");
	fwrite(audo, 1, sizeof(struct gms_chunk)+audo->length, out);
	fclose(out);

	fprintf(stdout, "okay\n");
}

void decode_sprt() {
	int32_t count, vec_off;
	uint8_t* index;

	FILE* out = fopen("dump/txt/sprt.txt", "w");

	// Extract sprite metadata. (I'm not sure how correct this all is; texcount is irrational

	vec_off = NORM_INDEX(sprt) + sizeof(struct gms_chunk);
	index = &CH_BUF(vec_off);

	count = get_array_length(index);
	for (int i = 0; i < count; i++) {
		struct gms_sprite *bg_s = (struct gms_sprite*)get_array_index_from(index, i);
		// A lot of these don't make much sense.
		fprintf(out, "[SPRT %d]\n", i);
		fprintf(out, "  Name '%s'\n", &CH_BUF(bg_s->name));
		fprintf(out, "  Width '%d'\n", bg_s->width);
		fprintf(out, "  Height '%d'\n", bg_s->height);
		fprintf(out, "  Texture count '%x'\n", bg_s->texture_count);
	}

	fclose(out);
}

void decode_bgnd() {
	int32_t count, vec_off;
	uint8_t* index;

	FILE* out = fopen("dump/txt/bgnd.txt", "w");

	// Backgrounds.

	vec_off = NORM_INDEX(bgnd) + sizeof(struct gms_chunk);
	index = &CH_BUF(vec_off);

	count = get_array_length(index);
	for (int i = 0; i < count; i++) {
		struct gms_background *bg_s = (struct gms_background*)get_array_index_from(index, i);
		fprintf(out, "[BGND %d]\n", i);
		fprintf(out, "  Name '%s'\n", &CH_BUF(bg_s->name));
		fprintf(out, "  Offset '%08x'\n", bg_s->offset);
	}

	fclose(out);
}

void decode_scpt() {
	int32_t count, vec_off;
	uint8_t* index;

	// Next is SCPT.

	FILE* out = fopen("dump/txt/scpt.txt", "w");

	vec_off = NORM_INDEX(scpt) + sizeof(struct gms_chunk);
	index = &CH_BUF(vec_off);

	count = get_array_length(index);
	for (int i = 0; i < count; i++) {
		struct gms_script_def *bg_s = (struct gms_script_def*)get_array_index_from(index, i);
		fprintf(out, "[SCPT %d]\n", i);
		fprintf(out, "  Name '%s'\n", &CH_BUF(bg_s->name));
		fprintf(out, "  ID '%08x'\n", bg_s->id);
	}
	fclose(out);

}

void decode_objt() {
	int32_t count, vec_off;
	uint8_t* index;

	// Next is OBJT.
	FILE* out = fopen("dump/txt/objt.txt", "w");

	vec_off = NORM_INDEX(objt) + sizeof(struct gms_chunk);
	index = &CH_BUF(vec_off);

	count = get_array_length(index);
	for (int i = 0; i < count; i++) {
		struct gms_object_def *bg_s = (struct gms_object_def*)get_array_index_from(index, i);
		fprintf(out, "[OBJT %d]\n", i);
		fprintf(out, "  Name '%s'\n", &CH_BUF(bg_s->name));
		fprintf(out, "  ID '%08x'\n", bg_s->id);
	}
	fclose(out);
}

void decode_room() {
	int32_t count, vec_off;
	uint8_t* index;

	// Next is the complicated one, ROOM.

	vec_off = NORM_INDEX(room) + sizeof(struct gms_chunk);
	index = &CH_BUF(vec_off);

	count = get_array_length(index);

	for (int i = 0; i < count; i++) {
		struct gms_room *bg_s = (struct gms_room*)get_array_index_from(index, i);

		char* name = malloc(strlen("dump/txt/room/") + strlen(&CH_BUF(bg_s->name)) + 15);
		sprintf(name, "dump/txt/room/%s.txt", &CH_BUF(bg_s->name));
		FILE* out = fopen(name, "w");

		fprintf(out, "Room %d\n", i);
		fprintf(out, "  Name '%s'\n", &CH_BUF(bg_s->name));
		fprintf(out, "  Width '%d'\n", bg_s->width);
		fprintf(out, "  Height '%d'\n", bg_s->height);
		fprintf(out, "  ARGB '%08x'\n", bg_s->argb);

		int32_t room_b_at = NORM_INDEX(bg_s) + sizeof(struct gms_room);
		uint8_t* room_b_off = &CH_BUF(room_b_at);
		int32_t room_b_len = get_array_length(room_b_off);
		fprintf(out, "  Backgrounds: %d\n", room_b_len);

		for (int i = 0; i < room_b_len; i++) {
			struct gms_room_bgnd* room_b = (struct gms_room_bgnd*)get_array_index_from(room_b_off, i);
			fprintf(out, "    Background %d\n", i);
			fprintf(out, "      Enabled: %u\n", room_b->enabled);

			fprintf(out, "      Background Ref:  %08x\n", room_b->bgnd_index);

			fprintf(out, "      X:  %d\n", room_b->x);
			fprintf(out, "      Y:  %d\n", room_b->y);

			fprintf(out, "      Tile X: %u\n", room_b->tile_x);
			fprintf(out, "      Tile Y: %u\n", room_b->tile_y);
		}

		int32_t room_v_at = room_b_at + (sizeof(struct gms_room_bgnd) * room_b_len) + (4 * room_b_len) + 4;
		uint8_t* room_v_off = &CH_BUF(room_v_at);
		int32_t room_v_len = get_array_length(room_v_off);
		fprintf(out, "  Views: %d\n", room_v_len);

		for (int i = 0; i < room_v_len; i++) {
			struct gms_room_view* room_v = (struct gms_room_view*)get_array_index_from(room_v_off, i);
			fprintf(out, "    View %d\n", i);
			fprintf(out, "      Enabled: %u\n", room_v->enabled);

			fprintf(out, "      View X:  %d\n", room_v->view_x);
			fprintf(out, "      View Y:  %d\n", room_v->view_y);

			fprintf(out, "      View Width:  %d\n", room_v->view_width);
			fprintf(out, "      View Height:  %d\n", room_v->view_height);

			fprintf(out, "      Port X:  %d\n", room_v->port_x);
			fprintf(out, "      Port Y:  %d\n", room_v->port_y);

			fprintf(out, "      Port Width:  %d\n", room_v->port_width);
			fprintf(out, "      Port Height:  %d\n", room_v->port_height);
		}

		int32_t room_o_at = room_v_at + (sizeof(struct gms_room_view) * room_v_len) + (4 * room_v_len) + 4;
		uint8_t* room_o_off = &CH_BUF(room_o_at);
		int32_t room_o_len = get_array_length(room_o_off);
		fprintf(out, "  Objects: %d\n", room_o_len);

		for (int i = 0; i < room_o_len; i++) {
			struct gms_room_gameobj* room_o = (struct gms_room_gameobj*)get_array_index_from(room_o_off, i);
			fprintf(out, "    Object %d\n", i);

			fprintf(out, "      X:  %d\n", room_o->x);
			fprintf(out, "      Y:  %d\n", room_o->y);

			fprintf(out, "      Definition Index:  %d\n", room_o->def_index);

			fprintf(out, "      Scale X:  %f\n", room_o->scale_x);
			fprintf(out, "      Scale Y:  %f\n", room_o->scale_y);
			fprintf(out, "      Tint:  %f\n", room_o->tint);
		}

		int32_t room_t_at = room_o_at + (sizeof(struct gms_room_gameobj) * room_o_len) + (4 * room_o_len) + 4;
		uint8_t* room_t_off = &CH_BUF(room_t_at);
		int32_t room_t_len = get_array_length(room_t_off);
		fprintf(out, "  Tiles: %d\n", room_t_len);

		for (int i = 0; i < room_t_len; i++) {
			struct gms_room_tiles* room_t = (struct gms_room_tiles*)get_array_index_from(room_t_off, i);
			fprintf(out, "    Tile %d\n", i);

			fprintf(out, "      X:  %d\n", room_t->x);
			fprintf(out, "      Y:  %d\n", room_t->y);

			fprintf(out, "      Definition Index:  %d\n", room_t->def_index);

			fprintf(out, "      Source X:  %d\n", room_t->source_x);
			fprintf(out, "      Source Y:  %d\n", room_t->source_y);

			fprintf(out, "      Width:  %d\n", room_t->width);
			fprintf(out, "      Height:  %d\n", room_t->height);

			fprintf(out, "      Scale X:  %f\n", room_t->scale_x);
			fprintf(out, "      Scale Y:  %f\n", room_t->scale_y);
			fprintf(out, "      Tint:  %f\n", room_t->tint);
		}

		free(name);
		fclose(out);
	}

}

void decode_strg() {
	int32_t count, vec_off;
	uint8_t* index;

	// Next is the complicated one, ROOM.
	FILE* out = fopen("dump/txt/strg.txt", "w");

	vec_off = NORM_INDEX(strg) + sizeof(struct gms_chunk);
	index = &CH_BUF(vec_off);

	count = get_array_length(index);

	for (int i = 0; i < count; i++) {
		uint8_t* off = get_array_index_from(index, i);
		int32_t    len = ((int32_t*)off)[0];
		char*      str = off+4;
		fprintf(out, "%s\n", str);
	}
	fclose(out);
}

char* gm_string_get(int index) {
	int32_t count, vec_off;
	uint8_t* at;

	vec_off = NORM_INDEX(strg) + sizeof(struct gms_chunk);
	at = &CH_BUF(vec_off);

	count = get_array_length(at);

	if (index > count) return NULL;

	uint8_t* off = get_array_index_from(at, index);
	char*      str = off+4;

	return str;
}

void decode_tpag() {
	int32_t count, vec_off;
	uint8_t* index;

	// Next is OBJT.
	FILE* out = fopen("dump/txt/tpag.txt", "w");

	vec_off = NORM_INDEX(tpag) + sizeof(struct gms_chunk);
	index = &CH_BUF(vec_off);

	count = get_array_length(index);
	for (int i = 0; i < count; i++) {
		struct gms_texture *bg_s = (struct gms_texture*)get_array_index_from(index, i);
		fprintf(out, "[TPAG %d]\n", i);
		fprintf(out, "  X: '%hd'\n", bg_s->x);
		fprintf(out, "  Y: '%hd'\n", bg_s->y);

		fprintf(out, "  Width: '%hd'\n", bg_s->width);
		fprintf(out, "  Height: '%hd'\n", bg_s->height);

		fprintf(out, "  Render X: '%hd'\n", bg_s->render_x);
		fprintf(out, "  Render Y: '%hd'\n", bg_s->render_y);

		fprintf(out, "  Sprite Sheet ID: '%hd'\n", bg_s->spritesheet_id);
	}
	fclose(out);

}



void decode_code() {
	int32_t count, vec_off;
	uint8_t* index;

	// Next is OBJT.

	vec_off = NORM_INDEX(code) + sizeof(struct gms_chunk);
	index = &CH_BUF(vec_off);

	count = get_array_length(index);
	for (int i = 0; i < count; i++) {
		uint8_t *at = get_array_index_from(index, i);
		char* name_c = &CH_BUF(((uint32_t*)at)[0]);
		int32_t len = ((uint32_t*)at)[1];
		at = at + 8;

		char* name = malloc(strlen("dump/txt/code/") + strlen(name_c) + 15);
		sprintf(name, "dump/txt/code/%s.txt", name_c);
		for(int i=0; i < strlen(name); i++) {
			if (name[i] == '_') {
				name[i] = 0;
				mkdir(name, 0755);
				name[i] = '/';
			}
		}

		printf("Disassembly -> Pass 1\n");

		FILE* out = fopen(name, "w");

		fprintf(out, "; Script Index: %d\n", i);
		fprintf(out, "; Script Name: '%s'\n", name_c);
		fprintf(out, "; Byte Length: %d\n", len);
		fprintf(out, "; Disassembly follows\n");

		disassemble_new(at, len, out, 0, NULL, 0);

		free(name);
		fclose(out);
	}
}

int main(int c, char** v) {
	printf("DARKER, DARKER YET DARKER, THIS EXPERIMENT SEEMS VERY INTERESTING\n\n");

	mkdir("dump", 0755);
	mkdir("dump/chunk", 0755);
	mkdir("dump/txt", 0755);
	mkdir("dump/txt/code", 0755);
	mkdir("dump/txt/room", 0755);

	map_gms_data(v[1]);
//	dump_chunks(v[1]);

//	printf("Dumping sprt (sprite) metadata...\n");
//	decode_sprt();

//	printf("Dumping bgnd (background) metadata...\n");
//	decode_bgnd();

//	printf("Dumping scpt (sound) metadata...\n");
//	decode_scpt();

//	printf("Dumping objt (object) metadata...\n");
//	decode_objt();

//	printf("Dumping room metadata...\n");
//	decode_room();

//	printf("Dumping tpag (texture) metadata...\n");
//	decode_tpag();

	printf("Dumping code...\n");
	decode_code();

//	printf("Dumping strg (string) metadata...\n");
//	decode_strg();

end:

	unmap_file();
	return 0;
}
