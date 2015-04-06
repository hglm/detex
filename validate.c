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
	DETEX_TEXTURE_FORMAT_BPTC_FLOAT,
	DETEX_TEXTURE_FORMAT_BPTC_SIGNED_FLOAT,
	DETEX_TEXTURE_FORMAT_ETC1,
	DETEX_TEXTURE_FORMAT_ETC2,
	DETEX_TEXTURE_FORMAT_ETC2_PUNCHTHROUGH,
	DETEX_TEXTURE_FORMAT_ETC2_EAC,
	DETEX_TEXTURE_FORMAT_EAC_R11,
	DETEX_TEXTURE_FORMAT_EAC_RG11,
	DETEX_TEXTURE_FORMAT_EAC_SIGNED_R11,
	DETEX_TEXTURE_FORMAT_EAC_SIGNED_RG11,
	// Uncompressed formats.
	DETEX_PIXEL_FORMAT_RGB8,
	DETEX_PIXEL_FORMAT_RGBA8,
	DETEX_PIXEL_FORMAT_FLOAT_RGB16,
	DETEX_PIXEL_FORMAT_FLOAT_RGBA16,
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
	"test-texture-BPTC_FLOAT.ktx",
	"test-texture-BPTC_SIGNED_FLOAT.ktx",
	"test-texture-ETC1.ktx",
	"test-texture-ETC2.ktx",
	"test-texture-ETC2_PUNCHTHROUGH.ktx",
	"test-texture-ETC2_EAC.ktx",
	"test-texture-EAC_R11.ktx",
	"test-texture-EAC_RG11.ktx",
	"test-texture-EAC_SIGNED_R11.ktx",
	"test-texture-EAC_SIGNED_RG11.ktx",
	"test-texture-RGB8.ktx",
	"test-texture-RGBA8.ktx",
	"test-texture-FLOAT_RGB16.ktx",
	"test-texture-FLOAT_RGBA16.ktx"
};

#define NU_TEXTURE_FORMATS (sizeof(texture_format) / sizeof(texture_format[0]))

static GtkWidget *gtk_window;
cairo_surface_t *surface[NU_TEXTURE_FORMATS];
GtkWidget *texture_label[NU_TEXTURE_FORMATS];
uint8_t *pixel_buffer[NU_TEXTURE_FORMATS];
detexTexture *texture[NU_TEXTURE_FORMATS];

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
		"Detex Library Validation Program");
	g_signal_connect(G_OBJECT(gtk_window), "delete_event",
		G_CALLBACK(delete_event_cb), NULL);
	g_signal_connect(G_OBJECT(gtk_window), "destroy",
		G_CALLBACK(destroy_cb), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(gtk_window), 0);

	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(gtk_window), vbox);
	GtkWidget *hbox;
	for (int i = 0; i < NU_TEXTURE_FORMATS; i++) {
		if (i % 8 == 0) {
			hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
			gtk_container_add(GTK_CONTAINER(vbox), hbox);
		}
		GtkWidget *image_drawing_area = gtk_drawing_area_new();
		gtk_widget_set_size_request(image_drawing_area,
			TEXTURE_WIDTH, TEXTURE_HEIGHT);
		// Create a vbox for texture image and label.
		GtkWidget *texture_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_box_pack_start(GTK_BOX(texture_vbox), image_drawing_area, TRUE, TRUE, 0);
		texture_label[i] = gtk_label_new("Unknown");
		gtk_box_pack_start(GTK_BOX(texture_vbox), texture_label[i], TRUE, TRUE, 0);
		// Add the texture vbox to the lay-out.
		gtk_box_pack_start(GTK_BOX(hbox), texture_vbox, TRUE, FALSE, 8);
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
	return detexLoadKTXFile(texture_file[i], &texture[i]);
}

int main(int argc, char **argv) {
	gtk_init(&argc, &argv);
	CreateWindowLayout();
	for (int i = 0; i < NU_TEXTURE_FORMATS; i++) {
		pixel_buffer[i] = (uint8_t *)malloc(16 * 8 *
			(TEXTURE_WIDTH / 4) * (TEXTURE_HEIGHT / 4));
		bool r = LoadCompressedTexture(i);
		if (!r) {
			printf("%s\n", detexGetErrorMessage());
			memset(pixel_buffer[i], 0, 16 * 8 *
				(TEXTURE_WIDTH / 4) * (TEXTURE_HEIGHT / 4));
			continue;
		}
		gtk_label_set_text(GTK_LABEL(texture_label[i]), detexGetTextureFormatText(texture[i]->format));
		uint32_t pixel_format;
		// Convert to a format suitable for Cairo.
		if (detexFormatHasAlpha(texture[i]->format))
			pixel_format = DETEX_PIXEL_FORMAT_BGRA8;
		else
			pixel_format = DETEX_PIXEL_FORMAT_BGRX8;
		r = detexDecompressTextureLinear(texture[i], pixel_buffer[i],
			pixel_format);
		if (!r) {
			printf("Decompression of %s returned error:\n%s\n",
				texture_file[i], detexGetErrorMessage());
			continue;
		}
	}
	for (int i = 0; i < NU_TEXTURE_FORMATS; i++)
		DrawTexture(i);
	gtk_main();
}

