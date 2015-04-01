/*

Copyright (c) 2015 Harm Hanemaaijer <fgenfb@yahoo.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

*/

/* Test/validation program. */

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <cairo/cairo.h>

#include "detex.h"

#define TEXTURE_WIDTH 64
#define TEXTURE_HEIGHT 64

static const uint32_t texture_format[] = {
	DETEX_TEXTURE_FORMAT_BC1,
	DETEX_TEXTURE_FORMAT_BC1A,
	DETEX_TEXTURE_FORMAT_BC2,
	DETEX_TEXTURE_FORMAT_BC3,
	DETEX_TEXTURE_FORMAT_RGTC1,
	DETEX_TEXTURE_FORMAT_RGTC2,
	DETEX_TEXTURE_FORMAT_SIGNED_RGTC1,
	DETEX_TEXTURE_FORMAT_SIGNED_RGTC2,
	DETEX_TEXTURE_FORMAT_BPTC,
	DETEX_TEXTURE_FORMAT_ETC1,
	DETEX_TEXTURE_FORMAT_ETC2,
	DETEX_TEXTURE_FORMAT_ETC2_PUNCHTHROUGH,
	DETEX_TEXTURE_FORMAT_ETC2_EAC,
	DETEX_TEXTURE_FORMAT_EAC_R11,
	DETEX_TEXTURE_FORMAT_EAC_RG11,
	DETEX_TEXTURE_FORMAT_EAC_SIGNED_R11,
	DETEX_TEXTURE_FORMAT_EAC_SIGNED_RG11
};

static const char *texture_file[] = {
	"test-texture-BC1.ktx",
	"test-texture-BC1A.ktx",
	"test-texture-BC2.ktx",
	"test-texture-BC3.ktx",
	"test-texture-RGTC1.ktx",
	"test-texture-RGTC2.ktx",
	"test-texture-SIGNED_RGTC1.ktx",
	"test-texture-SIGNED_RGTC2.ktx",
	"test-texture-BPTC.ktx",
	"test-texture-ETC1.ktx",
	"test-texture-ETC2.ktx",
	"test-texture-ETC2_PUNCHTHROUGH.ktx",
	"test-texture-ETC2_EAC.ktx",
	"test-texture-EAC_R11.ktx",
	"test-texture-EAC_RG11.ktx",
	"test-texture-EAC_SIGNED_R11.ktx",
	"test-texture-EAC_SIGNED_RG11.ktx",
};

static const uint32_t pixel_format[] = {
	DETEX_PIXEL_FORMAT_BGRX8,
	DETEX_PIXEL_FORMAT_BGRA8,
	DETEX_PIXEL_FORMAT_BGRA8,
	DETEX_PIXEL_FORMAT_BGRA8,
	DETEX_PIXEL_FORMAT_BGRX8,
	DETEX_PIXEL_FORMAT_BGRX8,
	// Convert from signed R16 to BGRX8.
	DETEX_PIXEL_FORMAT_BGRX8,
	// Convert from signed RG16 to BGRX8.
	DETEX_PIXEL_FORMAT_BGRX8,
	DETEX_PIXEL_FORMAT_BGRA8,
	DETEX_PIXEL_FORMAT_BGRX8,
	DETEX_PIXEL_FORMAT_BGRX8,
	DETEX_PIXEL_FORMAT_BGRA8,
	DETEX_PIXEL_FORMAT_BGRA8,
	// Convert from R16 to BGRX8.
	DETEX_PIXEL_FORMAT_BGRX8,
	// Convert from RG16 to BGRX8.
	DETEX_PIXEL_FORMAT_BGRX8,
	// Convert from signed R16 to BGRX8.
	DETEX_PIXEL_FORMAT_BGRX8,
	// Convert from signed RG16 to BGRX8.
	DETEX_PIXEL_FORMAT_BGRX8
};

#define NU_TEXTURE_FORMATS (sizeof(texture_format) / sizeof(texture_format[0]))

static GtkWidget *gtk_window;
cairo_surface_t *surface[NU_TEXTURE_FORMATS];
uint8_t *pixel_buffer[NU_TEXTURE_FORMATS];
uint8_t *compressed_data[NU_TEXTURE_FORMATS];

static gboolean delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data) {
    return FALSE;
}

static void destroy_cb(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
    exit(0);
}

static gboolean area_draw_cb(GtkWidget *widget, cairo_t *cr, cairo_surface_t *surface) {
    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);
    return TRUE;
}

static void CreateWindowLayout() {
	gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(gtk_window),
		"detex library validation program");
	g_signal_connect(G_OBJECT(gtk_window), "delete_event",
		G_CALLBACK(delete_event_cb), NULL);
	g_signal_connect(G_OBJECT(gtk_window), "destroy",
		G_CALLBACK(destroy_cb), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(gtk_window), 0);

	GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(GTK_CONTAINER(gtk_window), hbox);
	for (int i = 0; i < NU_TEXTURE_FORMATS; i++) {
		GtkWidget *image_drawing_area = gtk_drawing_area_new();
		gtk_widget_set_size_request(image_drawing_area,
			TEXTURE_WIDTH, TEXTURE_HEIGHT);
		gtk_box_pack_start(GTK_BOX(hbox), image_drawing_area, TRUE,
			TRUE, 0);
		surface[i] = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
			TEXTURE_WIDTH, TEXTURE_HEIGHT);
		g_signal_connect(image_drawing_area, "draw",
			G_CALLBACK(area_draw_cb), surface[i]);
	}

	gtk_widget_show_all(gtk_window);
}

static void DrawTexture(int i) {
	cairo_surface_t *image_surface = cairo_image_surface_create_for_data(
		(unsigned char *)pixel_buffer[i], CAIRO_FORMAT_ARGB32,
		TEXTURE_WIDTH, TEXTURE_HEIGHT, TEXTURE_WIDTH * 4);
	cairo_t *cr = cairo_create(surface[i]);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_rgba(cr, 0.0d, 0.0d, 0.5d, 1.0d);
	cairo_paint(cr);
	cairo_set_source_surface(cr, image_surface, 0.0d, 0.0d);
	cairo_mask_surface(cr, image_surface, 0.0d, 0.0d);
	cairo_destroy(cr);
	cairo_surface_destroy(image_surface);
}

static bool LoadCompressedTexture(int i) {
	compressed_data[i] = (uint8_t *)malloc(16 *
		(TEXTURE_WIDTH / 4) * (TEXTURE_HEIGHT / 4));
	FILE *f = fopen(texture_file[i], "rb");
	if (f == NULL) {
		printf("Error opening texture file %s.\n", texture_file[i]);
		return false;
	}
	// Read the KTX header (skip it).
	int n = fread(compressed_data[i], 1, 68, f);
	if (n != 68) {
		printf("Error reading texture file %s.\n", texture_file[i]);
		return false;
	}
	// Read the compressed texture. */
	uint32_t compressed_block_size = detexGetCompressedBlockSize(
		texture_format[i]);
	n = fread(compressed_data[i], 1, compressed_block_size *
		(TEXTURE_WIDTH / 4) * (TEXTURE_HEIGHT / 4), f);
	if (n != compressed_block_size * (TEXTURE_WIDTH / 4) *
	(TEXTURE_HEIGHT / 4)) {
		printf("Error reading texture file %s.\n", texture_file[i]);
		return false;
	}
	fclose(f);
	return true;
}

int main(int argc, char **argv) {
	gtk_init(&argc, &argv);
	CreateWindowLayout();
	for (int i = 0; i < NU_TEXTURE_FORMATS; i++) {
		pixel_buffer[i] = (uint8_t *)malloc(16 * 8 *
			(TEXTURE_WIDTH / 4) * (TEXTURE_HEIGHT / 4));
		bool r = LoadCompressedTexture(i);
		if (!r) {
			memset(pixel_buffer[i], 0, 16 * 8 *
				(TEXTURE_WIDTH / 4) * (TEXTURE_HEIGHT / 4));
			continue;
		}
		r = detexDecompressTextureLinear(compressed_data[i],
			texture_format[i], TEXTURE_WIDTH / 4,
			TEXTURE_HEIGHT / 4, pixel_buffer[i],
			pixel_format[i]);
		if (!r) {
			printf("Decompression of %s returned error.\n",
				texture_file[i]);
		}
	}
	for (int i = 0; i < NU_TEXTURE_FORMATS; i++)
		DrawTexture(i);
	gtk_main();
}

