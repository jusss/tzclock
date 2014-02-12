/**********************************************************************************************************************
 *                                                                                                                    *
 *  T Z  C L O C K  C A I R O . C                                                                                     *
 *  =============================                                                                                     *
 *                                                                                                                    *
 *  This is free software; you can redistribute it and/or modify it under the terms of the GNU General Public         *
 *  License version 2 as published by the Free Software Foundation.  Note that I am not granting permission to        *
 *  redistribute or modify this under the terms of any later version of the General Public License.                   *
 *                                                                                                                    *
 *  This is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the                *
 *  impliedwarranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for   *
 *  more details.                                                                                                     *
 *                                                                                                                    *
 *  You should have received a copy of the GNU General Public License along with this program (in the file            *
 *  "COPYING"); if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111,   *
 *  USA.                                                                                                              *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @file
 *  @brief .
 *  @version $Id: TzClockCairo.c 1818 2013-12-29 10:38:58Z chris $
 */
#include "config.h"
#include "TzClockDisp.h"
#include "TimeZone.h"

extern GtkWidget *mainWindow;
extern GtkWidget *drawingArea;
#if GTK_MAJOR_VERSION == 2
extern GdkColor clockColours[];
#else
extern GdkRGBA clockColours[];
#endif
extern char fontName[];
extern int faceSize;
extern int faceWidth;
extern int faceHeight;
extern int faceGradient;
extern int weHaveFocus;
extern int currentFace;
extern int toolTipFace;
extern int timeSetting;
extern bool fastSetting;
extern bool showBounceSec;
extern int markerType;
extern int markerStep;
extern time_t forceTime;
extern HAND_STYLE handStyle[];
extern FACE_SETTINGS *faceSettings[];
extern TZ_INFO *timeZones;

#if GTK_MAJOR_VERSION == 2
static GdkDrawable *windowShapeBitmap = NULL;
#endif

/*------------------------------------------------------------------------------------------------*
 * Change the font size as the window gets bigger.                                                *
 *------------------------------------------------------------------------------------------------*/
static int fontSizes[16] = 
{
	 6, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64,
};
char *roman[24] = 
{
	"XXIV", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII",
	"XIII", "XIV", "XV", "XVI", "XVII", "XVIII", "XIX", "XX", "XXI", "XXII", "XXIII"
};
char *latin[24] =
{
	"24", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", 
	"13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23"
};

void drawText (cairo_t *cr, int posX, int posY, char *string1, int colour, int scale);
float getFontSize (char *fontName);

/**********************************************************************************************************************
 *                                                                                                                    *
 *  M A K E  W I N D O W  M A S K                                                                                     *
 *  =============================                                                                                     *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @result .
 */
void makeWindowMask ()
{
#if GTK_MAJOR_VERSION == 2

	int fullHeight, fullWidth, i, j;
	GdkGC *gc;
        
	fullHeight = faceHeight * faceSize;
	fullWidth = faceWidth * faceSize;
        
	if (windowShapeBitmap)
	{
		g_object_unref (windowShapeBitmap);
		windowShapeBitmap = NULL;
	}

	windowShapeBitmap = (GdkBitmap *) gdk_pixmap_new (NULL, fullWidth, fullHeight, 1);

	gc = gdk_gc_new (windowShapeBitmap);
	gdk_gc_set_foreground (gc, &clockColours[BLACK_COLOUR]);
	gdk_gc_set_background (gc, &clockColours[WHITE_COLOUR]);

	gdk_draw_rectangle (windowShapeBitmap, gc, TRUE, 0, 0, fullWidth, fullHeight);

	gdk_gc_set_foreground (gc, &clockColours[WHITE_COLOUR]);
	gdk_gc_set_background (gc, &clockColours[BLACK_COLOUR]);

	for (i = 0; i < faceWidth; i++)
	{
		for (j = 0; j < faceHeight; j++)
			gdk_draw_arc (windowShapeBitmap, gc, TRUE, (i * faceSize) - 1, (j * faceSize) - 1, 
					faceSize + 1, faceSize + 1, 0, 360 * 64);
	}
	gtk_widget_shape_combine_mask (mainWindow, windowShapeBitmap, 0, 0);
	g_object_unref (gc);

#else

	cairo_t *cr;
	cairo_surface_t *surface;
	cairo_region_t *region;
	int fullHeight, fullWidth, i, j;

	fullHeight = faceHeight * faceSize;
	fullWidth = faceWidth * faceSize;

	surface = cairo_image_surface_create (CAIRO_FORMAT_A8, fullWidth, fullHeight);
	cr = cairo_create (surface);
	if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
	{
		for (i = 0; i < faceWidth; i++)
		{
			for (j = 0; j < faceHeight; j++)
			{
				int centerX = (faceSize * i) + (faceSize >> 1);
				int centerY = (faceSize * j) + (faceSize >> 1);
				
				cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
		 		cairo_arc (cr, centerX, centerY, (faceSize * 64) >> 7, 0, 2 * M_PI);
				cairo_fill (cr);
				cairo_stroke (cr);
			}
		}
		region = gdk_cairo_region_create_from_surface (surface);
		gtk_widget_shape_combine_region (mainWindow, region);
		cairo_region_destroy (region);
	}
	cairo_destroy (cr);

#endif
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  S E T  C O L O U R                                                                                                *
 *  ==================                                                                                                *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param cr .
 *  @param colour .
 *  @result .
 */
void setColour (cairo_t *cr, int colour)
{
#if GTK_MAJOR_VERSION == 2
	gdk_cairo_set_source_color (cr, &clockColours[colour]);
#else
	gdk_cairo_set_source_rgba (cr, &clockColours[colour]);
#endif
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  D R A W  M I N U T E                                                                                              *
 *  ====================                                                                                              *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param cr .
 *  @param posX .
 *  @param posY .
 *  @param size .
 *  @param len .
 *  @param angle .
 *  @param colour .
 *  @result .
 */
void drawMinute (cairo_t *cr, int posX, int posY, int size, int len, int angle, int colour)
{
	setColour (cr, colour);
	if (len < size)
	{
		cairo_move_to (cr,
				posX + xSinCos ((faceSize * size) >> 6, angle, 0),
				posY - xSinCos ((faceSize * size) >> 6, angle, 1));

		cairo_line_to (cr,
				posX + xSinCos ((faceSize * (size + len)) >> 6, angle, 0),
				posY - xSinCos ((faceSize * (size + len)) >> 6, angle, 1));
	}
	else
	{
		cairo_move_to (cr,
				posX + xSinCos ((faceSize * size) >> 6, angle + SCALE_2, 0),
				posY - xSinCos ((faceSize * size) >> 6, angle + SCALE_2, 1));

		cairo_line_to (cr,
				posX + xSinCos ((faceSize * (size + len)) >> 6, angle, 0),
				posY - xSinCos ((faceSize * (size + len)) >> 6, angle, 1));
	}
	cairo_stroke (cr);
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  D R A W  C I R C L E                                                                                              *
 *  ====================                                                                                              *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param cr .
 *  @param posX .
 *  @param posY .
 *  @param size .
 *  @param colFill .
 *  @param colOut .
 *  @result .
 */
void drawCircle (cairo_t *cr, int posX, int posY, int size, int colFill, int colOut)
{
	if (colFill != -1)
	{
		setColour (cr, colFill);
 		cairo_arc (cr, posX, posY, (faceSize * size) >> 7, 0, 2 * M_PI);
		cairo_fill (cr);
		cairo_stroke (cr);
	}
	if (colOut != -1)
	{
		setColour (cr, colOut);
 		cairo_arc (cr, posX, posY, (faceSize * size) >> 7, 0, 2 * M_PI);
		cairo_stroke (cr);
	}
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  D R A W  C I R C L E  G R A D I E N T                                                                             *
 *  =====================================                                                                             *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param cr .
 *  @param posX .
 *  @param posY .
 *  @param size .
 *  @param colFill .
 *  @param style .
 *  @result .
 */
void drawCircleGradient (cairo_t * cr, int posX, int posY, int size, int colFill, int style)
{
	cairo_pattern_t *pat;
	int patSize = (faceSize >> 1) + (posX > posY ? posX : posY), i;
	int x = posX / faceSize, y = posY / faceSize, j = x + y, k = patSize / faceSize;
	float gradL = (float)(100 - faceGradient) / 100.0;
	float gradH = (float)(100 + faceGradient) / 100.0;
	float x1, x2, col[3][3];
	
#if GTK_MAJOR_VERSION == 2
	col[0][0] = (float)clockColours[colFill].red / 65535.0;
	col[1][0] = (float)clockColours[colFill].green / 65535.0;
	col[2][0] = (float)clockColours[colFill].blue / 65535.0;
#else
	col[0][0] = clockColours[colFill].red;
	col[1][0] = clockColours[colFill].green;
	col[2][0] = clockColours[colFill].blue;
#endif

	for (i = 0; i < 3; ++i)
	{
		col[i][1] = col[i][0] * (style ? gradH : gradL);
		if (col[i][1] > 1) col[i][1] = 1;
		col[i][2] = col[i][0] * (style ? gradL : gradH);
		if (col[i][2] > 1) col[i][2] = 1;
	}
	pat = cairo_pattern_create_linear (0.0, 0.0, patSize, patSize);
	
	x1 = j;
	x1 /= (2 * k);
	x2 = x1 + ((float)1 / k);
	
	cairo_pattern_add_color_stop_rgb (pat, x1, col[0][1], col[1][1], col[2][1]);
	cairo_pattern_add_color_stop_rgb (pat, x2, col[0][2], col[1][2], col[2][2]);
	cairo_arc (cr, posX, posY, (faceSize * size) >> 7, 0, 2 * M_PI);
	cairo_set_source (cr, pat);
	cairo_fill (cr);

	cairo_pattern_destroy (pat);
}


/**********************************************************************************************************************
 *                                                                                                                    *
 *  D R A W  H A N D                                                                                                  *
 *  ================                                                                                                  *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param cr .
 *  @param posX .
 *  @param posY .
 *  @param angle .
 *  @param handStyle .
 *  @result .
 */
void drawHand (cairo_t *cr, int posX, int posY, int angle, HAND_STYLE *handStyle)
{
	int points[20], i, j, numPoints, fill = 0;
	int size = handStyle -> length, style = handStyle -> style, tail = handStyle -> tail;
	int colFill = handStyle -> fill, colOut = handStyle -> line;

	while (angle < 0) angle += SCALE_4;
	while (angle >= SCALE_4) angle -= SCALE_4;

	switch (style)
	{
	case 0:
		// Original double triangle
		points[0] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0);
		points[1] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1);
		points[2] = posX + xSinCos (faceSize >> 6, angle + SCALE_1, 0);
		points[3] = posY - xSinCos (faceSize >> 6, angle + SCALE_1, 1);
		points[4] = posX + xSinCos ((faceSize * size) >> 6, angle, 0);
		points[5] = posY - xSinCos ((faceSize * size) >> 6, angle, 1);
		points[6] = posX + xSinCos (faceSize >> 6, angle + SCALE_3, 0);
		points[7] = posY - xSinCos (faceSize >> 6, angle + SCALE_3, 1); 
		numPoints = 4;
		fill = 1;
		break;

	case 1:
		// Single triangle
		points[0] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0) 
					+ xSinCos (faceSize >> 6, angle + SCALE_1, 0);
		points[1] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1)
					- xSinCos (faceSize >> 6, angle + SCALE_1, 1);
		points[2] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0) 
					+ xSinCos (faceSize >> 6, angle + SCALE_3, 0);
		points[3] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1)
					- xSinCos (faceSize >> 6, angle + SCALE_3, 1);
		points[4] = posX + xSinCos ((faceSize * size) >> 6, angle, 0);
		points[5] = posY - xSinCos ((faceSize * size) >> 6, angle, 1);
		numPoints = 3;
		fill = 1;
		break;

	case 2:
		// Rectangle
		points[0] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0) + xSinCos (faceSize >> 6, angle + SCALE_1, 0);
		points[1] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1) - xSinCos (faceSize >> 6, angle + SCALE_1, 1);
		points[2] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0) + xSinCos (faceSize >> 6, angle + SCALE_3, 0);
		points[3] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1) - xSinCos (faceSize >> 6, angle + SCALE_3, 1);
		points[4] = posX + xSinCos ((faceSize * size) >> 6, angle, 0) + xSinCos (faceSize >> 6, angle + SCALE_3, 0);
		points[5] = posY - xSinCos ((faceSize * size) >> 6, angle, 1) - xSinCos (faceSize >> 6, angle + SCALE_3, 1);
		points[6] = posX + xSinCos ((faceSize * size) >> 6, angle, 0) + xSinCos (faceSize >> 6, angle + SCALE_1, 0);
		points[7] = posY - xSinCos ((faceSize * size) >> 6, angle, 1) - xSinCos (faceSize >> 6, angle + SCALE_1, 1);
		numPoints = 4;
		fill = 1;
		break;

	case 3:
		//Rectangle with pointer
		points[0] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0) + xSinCos (faceSize >> 6, angle + SCALE_1, 0);
		points[1] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1) - xSinCos (faceSize >> 6, angle + SCALE_1, 1);
		points[2] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0) + xSinCos (faceSize >> 6, angle + SCALE_3, 0);
		points[3] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1) - xSinCos (faceSize >> 6, angle + SCALE_3, 1);
		points[4] = posX + xSinCos ((faceSize * (size * 15)) >> 10, angle, 0) + xSinCos (faceSize >> 6, angle + SCALE_3, 0);
		points[5] = posY - xSinCos ((faceSize * (size * 15)) >> 10, angle, 1) - xSinCos (faceSize >> 6, angle + SCALE_3, 1);
		points[6] = posX + xSinCos ((faceSize * size) >> 6, angle, 0);
		points[7] = posY - xSinCos ((faceSize * size) >> 6, angle, 1);
		points[8] = posX + xSinCos ((faceSize * (size * 15)) >> 10, angle, 0) + xSinCos (faceSize >> 6, angle + SCALE_1, 0);
		points[9] = posY - xSinCos ((faceSize * (size * 15)) >> 10, angle, 1) - xSinCos (faceSize >> 6, angle + SCALE_1, 1);
		numPoints = 5;
		fill = 1;
		break;

	case 4:
		//Rectangle with arrow
		points[0] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0) + xSinCos (faceSize >> 6, angle + SCALE_1, 0);
		points[1] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1)	- xSinCos (faceSize >> 6, angle + SCALE_1, 1);
		points[2] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0)	+ xSinCos (faceSize >> 6, angle + SCALE_3, 0);
		points[3] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1)	- xSinCos (faceSize >> 6, angle + SCALE_3, 1);
		points[4] = posX + xSinCos ((faceSize * (size * 12)) >> 10, angle, 0) + xSinCos (faceSize >> 6, angle + SCALE_3, 0);
		points[5] = posY - xSinCos ((faceSize * (size * 12)) >> 10, angle, 1) - xSinCos (faceSize >> 6, angle + SCALE_3, 1);
		points[6] = posX + xSinCos ((faceSize * (size * 12)) >> 10, angle, 0) + xSinCos (faceSize / 30, angle + SCALE_3, 0);
		points[7] = posY - xSinCos ((faceSize * (size * 12)) >> 10, angle, 1) - xSinCos (faceSize / 30, angle + SCALE_3, 1);
		points[8] = posX + xSinCos ((faceSize * size) >> 6, angle, 0);
		points[9] = posY - xSinCos ((faceSize * size) >> 6, angle, 1);
		points[10] = posX + xSinCos ((faceSize * (size * 12)) >> 10, angle, 0) + xSinCos (faceSize / 30, angle + SCALE_1, 0);
		points[11] = posY - xSinCos ((faceSize * (size * 12)) >> 10, angle, 1) - xSinCos (faceSize / 30, angle + SCALE_1, 1);
		points[12] = posX + xSinCos ((faceSize * (size * 12)) >> 10, angle, 0) + xSinCos (faceSize >> 6, angle + SCALE_1, 0);
		points[13] = posY - xSinCos ((faceSize * (size * 12)) >> 10, angle, 1) - xSinCos (faceSize >> 6, angle + SCALE_1, 1);
		numPoints = 7;
		fill = 1;
		break;

	case 5:
		// Single triangle
		points[0] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0) 
					+ xSinCos (faceSize >> 5, angle + SCALE_1, 0);
		points[1] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1)
					- xSinCos (faceSize >> 5, angle + SCALE_1, 1);
		points[2] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0) 
					+ xSinCos (faceSize >> 5, angle + SCALE_3, 0);
		points[3] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1)
					- xSinCos (faceSize >> 5, angle + SCALE_3, 1);
		points[4] = posX + xSinCos ((faceSize * size) >> 6, angle, 0);
		points[5] = posY - xSinCos ((faceSize * size) >> 6, angle, 1);

    	cairo_set_line_width (cr, 0.1f);
		for (i = 0; i < 4; i++)
		{
			setColour (cr, i < 2 ? colFill : colOut);
			cairo_move_to (cr, points[0], points[1]);
			for (j = 1; j < 3; j++)
				cairo_line_to (cr, points[j << 1], points[(j << 1) + 1]);
			cairo_close_path (cr);
			if (i & 1)
				cairo_fill (cr);
			cairo_stroke (cr);
			if (i == 1)
			{
				points[2] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0);
				points[3] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1);
			}
		}
    	cairo_set_line_width (cr, 1.0f + ((float)faceSize / 256.0f));
		return;

	case 9:
	default:
		// Simple line
		points[0] = posX + xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 0);
		points[1] = posY - xSinCos ((faceSize * tail) >> 6, angle + SCALE_2, 1);
		points[2] = posX + xSinCos ((faceSize * size) >> 6, angle, 0);
		points[3] = posY - xSinCos ((faceSize * size) >> 6, angle, 1);
		numPoints = 2;
		break;
	}
	if (!handStyle -> fillIn)
	{
		fill = 0;
	}
	for (i = 0; i < 2; i++)
	{
		if (i != 0 || fill)
		{
			setColour (cr, i == 0 ? colFill : colOut);
			cairo_move_to (cr, points[0], points[1]);
			for (j = 1; j < numPoints; j++)
				cairo_line_to (cr, points[j << 1], points[(j << 1) + 1]);
			if (numPoints > 2)
				cairo_close_path (cr);
			if (i == 0)
				cairo_fill (cr);
			cairo_stroke (cr);
		}
	}
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  D R A W  M A R K                                                                                                  *
 *  ================                                                                                                  *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param cr .
 *  @param posX .
 *  @param posY .
 *  @param angle .
 *  @param size .
 *  @param colFill .
 *  @param colOut .
 *  @param hour .
 *  @result .
 */
void drawMark (cairo_t *cr, int posX, int posY, int angle, int size, 
		int colFill, int colOut, int hour)
{
	if (markerStep == 0) markerStep = 1;

	if (!(hour % markerStep))
	{
		switch (markerType)
		{
		case 0:
			/* No markers */
			break;
	
		case 1:
			/* Triangle markers */
			{
				int points[6], i;
				points[0] = posX + xSinCos ((faceSize * size) >> 6, angle + 2, 0);
				points[1] = posY - xSinCos ((faceSize * size) >> 6, angle + 2, 1);
				points[2] = posX + xSinCos ((faceSize * (size - 3)) >> 6, angle, 0);
				points[3] = posY - xSinCos ((faceSize * (size - 3)) >> 6, angle, 1);
				points[4] = posX + xSinCos ((faceSize * size) >> 6, angle - 2, 0);
				points[5] = posY - xSinCos ((faceSize * size) >> 6, angle - 2, 1);
				for (i = 0; i < 2; i++)
				{
					setColour (cr, i == 0 ? colFill : colOut);
					cairo_move_to (cr, points[0], points[1]);
					cairo_line_to (cr, points[2], points[3]);
					cairo_line_to (cr, points[4], points[5]);
					cairo_close_path (cr);
					if (i == 0)	cairo_fill (cr);
					cairo_stroke (cr);
				}
			}
			break;

		case 2:
			/* Circle markers */
			posX += xSinCos ((faceSize * (size - 2)) >> 6, angle, 0);
			posY -= xSinCos ((faceSize * (size - 2)) >> 6, angle, 1);
			drawCircle (cr, posX, posY, 3, colFill, colOut);
			break;

		case 3:
			/* Latin number markers */
			posX += xSinCos ((faceSize * (size - 5)) >> 6, angle, 0);
			posY -= xSinCos ((faceSize * (size - 5)) >> 6, angle, 1);
			drawText (cr, posX, posY, latin[hour], colOut, 1);
			break;

		case 4:
			/* Roman number markers */
			posX += xSinCos ((faceSize * (size - 5)) >> 6, angle, 0);
			posY -= xSinCos ((faceSize * (size - 5)) >> 6, angle, 1);
			drawText (cr, posX, posY, roman[hour], colOut, 1);
			break;
		}
	}
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  G E T  F O N T  S I Z E                                                                                           *
 *  =======================                                                                                           *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief Get the font size from a font def.
 *  @param fontName String to read the size from.
 *  @result The font size.
 */
float getFontSize (char *fontName)
{
	int i = strlen (fontName);
	float size = 0;
	
	while (i)
	{
		i --;
		if ((fontName[i] >= '0' && fontName[i] <= '9') || fontName[i] == '.')
			continue;
		break;
	}
	if (i < strlen (fontName))
	{
		size = atof (&fontName[i + 1]);
	}
	return size;
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  D R A W  T E X T                                                                                                  *
 *  ================                                                                                                  *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param cr .
 *  @param posX .
 *  @param posY .
 *  @param string1 .
 *  @param colour .
 *  @param scale .
 *  @result .
 */
void drawText (cairo_t *cr, int posX, int posY, char *string1, int colour, int scale)
{
	int fontSize = 0;
	gint posW, posH;

	if ((fontSize = getFontSize (fontName)) == 0)
		fontSize = fontSizes[(faceSize >> 6) - 1];
	if (scale)
		fontSize = (fontSize * 10) / 12;

	PangoLayout *layout = pango_cairo_create_layout (cr);;
	PangoFontDescription *fontDesc = pango_font_description_from_string (fontName);

	pango_font_description_set_size (fontDesc, (int)(fontSize * PANGO_SCALE));
 	pango_layout_set_font_description (layout, fontDesc); 
	pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);

	setColour (cr, colour == -1 ? TEXT__COLOUR : colour);
	pango_layout_set_text (layout, string1, -1);
	pango_layout_get_pixel_size (layout, &posW, &posH);

	cairo_move_to (cr, posX - (posW >> 1), posY - (posH >> 1));
	pango_cairo_show_layout (cr, layout);

	pango_font_description_free (fontDesc);
	g_object_unref (layout);
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  D R A W  F A C E                                                                                                  *
 *  ================                                                                                                  *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param cr .
 *  @param face .
 *  @param posX .
 *  @param posY .
 *  @param circ .
 *  @result .
 */
gboolean 
drawFace (cairo_t *cr, int face, int posX, int posY, char circ)
{
	FACE_SETTINGS *faceSetting = faceSettings[face];
	int i, j, timeZone = faceSetting -> currentTZ;
	int centerX = posX + (faceSize >> 1), centerY = posY + (faceSize >> 1);
	int showSubSec, markerFlags = 0xFFFFFF;
	time_t t = faceSetting -> timeShown;
	char tempString[101];
	struct tm tm;

	/*------------------------------------------------------------------------------------------------*
	 * The alarm check has moved out of here                                                          *
	 *------------------------------------------------------------------------------------------------*/	
	getTheFaceTime (faceSetting, &t, &tm);
	
	/*------------------------------------------------------------------------------------------------*
	 * Draw the face, it is made up of 3 overlapping circles                                          *
	 *------------------------------------------------------------------------------------------------*/	
	cairo_save (cr);
	cairo_set_line_width (cr, 1.0f + ((float)faceSize / 256.0f));
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);

	if (weHaveFocus && face == currentFace)
		setColour (cr, FACE1_COLOUR);
	else
		setColour (cr, FACE2_COLOUR);
		
	if (circ)
		cairo_arc (cr, centerX, centerY, faceSize >> 1, 0, 2 * M_PI);
	else
		cairo_rectangle (cr, posX, posY, faceSize, faceSize);

	cairo_fill (cr);
	if (faceGradient == 0)
	{
		drawCircle (cr, centerX, centerY, 62, FACE3_COLOUR, -1);
		drawCircle (cr, centerX, centerY, 60, FACE4_COLOUR, -1);
	}
	else
	{
		drawCircleGradient (cr, centerX, centerY, 62, FACE3_COLOUR, 0);
		drawCircleGradient (cr, centerX, centerY, 58, FACE4_COLOUR, 1);
	}
	
	/*------------------------------------------------------------------------------------------------*
	 * Add the text, ether the date or the timezone, plus an AM/PM indicator                          *
	 *------------------------------------------------------------------------------------------------*/
	showSubSec = (faceSetting -> showSeconds && (faceSetting -> stopwatch || faceSetting -> subSecond)) ? 1 : 0;

	getStringValue (tempString, 100, faceSetting -> stopwatch ?
			(timeZone ? TXT_TOPSW_Z : TXT_TOPSW_L) : (timeZone ? TXT_TOP_Z : TXT_TOP_L), face, t);
	drawText (cr, centerX, posY + ((faceSize * 5) >> 4), tempString, -1, 0);
	 
	if (!showSubSec)
	{
		getStringValue (tempString, 100, timeZone ? TXT_BOTTOM_Z : TXT_BOTTOM_L, face, t);	
		drawText (cr, centerX, posY + ((faceSize * 11) >> 4), tempString, -1, 0);
	}

	/*------------------------------------------------------------------------------------------------*
	 * Calculate which markers to draw                                                                *
	 *------------------------------------------------------------------------------------------------*/
	if (markerType > 2)
	{
		if (showSubSec)
		{
			if (faceSetting -> show24Hour)
			{
				markerFlags &= ~(1 << 11);
				markerFlags &= ~(1 << 12);
				markerFlags &= ~(1 << 13);
			}
			else
				markerFlags &= ~(1 << 6);
		}
		if (faceSetting -> stopwatch)
		{
			if (faceSetting -> show24Hour)
			{
				markerFlags &= ~(1 << 5);
				markerFlags &= ~(1 << 6);
				markerFlags &= ~(1 << 7);
				markerFlags &= ~(1 << 17);
				markerFlags &= ~(1 << 18);
				markerFlags &= ~(1 << 19);
			}
			else
			{
				markerFlags &= ~(1 << 3);
				markerFlags &= ~(1 << 9);
			}
		}		
	}

	/*------------------------------------------------------------------------------------------------*
	 * Draw the hour markers                                                                          *
	 *------------------------------------------------------------------------------------------------*/
	cairo_set_line_width (cr, 1.0f + ((float)faceSize / 512.0f));
	j = faceSetting -> show24Hour ? 120 : 60;

	for (i = 0; i < j ; i++)
	{
		int m = faceSetting -> show24Hour ? i * 10 : i * 20;

		if (faceSize > 256)
		{
			if (!faceSetting -> show24Hour || !(i % 2))
				drawMinute (cr, centerX, centerY, 30, 1, m, MMARK_COLOUR);
		}
		if (!(i % 5))
		{
			drawMinute (cr, centerX, centerY, 29, 1, m, HMARK_COLOUR);
			if (markerFlags & (1 << (i / 5)))
				drawMark (cr, centerX , centerY, m, 31, QFILL_COLOUR, QMARK_COLOUR, 
						(i == 0 && !faceSetting -> show24Hour) ? 12 : i / 5);
		}
	}

	/*------------------------------------------------------------------------------------------------*
	 * Draw other clock faces                                                                         *
	 *------------------------------------------------------------------------------------------------*/
	if (showSubSec)
	{
		drawCircle (cr, centerX, posY + ((3 * faceSize) >> 2), 21, FACE5_COLOUR, -1);
	}
	if (faceSetting -> stopwatch)
	{
		drawCircle (cr, posX + (faceSize >> 2), centerY, 21, FACE5_COLOUR, -1);			
		drawCircle (cr, posX + (3 * faceSize >> 2), centerY, 21, FACE5_COLOUR, -1);
	}	
	
	for (i = 0; i < 60 ; i++)
	{
		int m = i * 20;

		if (showSubSec)
		{
			if (!(i % 5))
				drawMinute (cr, centerX, posY + ((3 * faceSize) >> 2),
						(i % 15) ? 9 : 8, (i % 15) ? 1 : 2, m, WMARK_COLOUR);
		}
		if (faceSetting -> stopwatch)
		{
			if (!(i % 6))
				drawMinute (cr, posX + (faceSize >> 2), centerY,
						(i % 12) ? 9 : 8, (i % 12) ? 1 : 2, m, WMARK_COLOUR);
			if (!(i % 4))
				drawMinute (cr, posX + ((3 * faceSize) >> 2), centerY,
						(i % 20) ? 9 : 8, (i % 20) ? 1 : 2, m, WMARK_COLOUR);
		}
	}

	cairo_set_line_width (cr, 1.0f + ((float)faceSize / 256.0f));

	/*------------------------------------------------------------------------------------------------*
	 * Draw the hands                                                                                 *
	 *------------------------------------------------------------------------------------------------*/
	if (faceSetting -> alarmInfo.showAlarm)
	{
		drawHand (cr, centerX, centerY, faceSetting -> handPosition[HAND_ALARM], &handStyle[HAND_ALARM]);
	}
	if (showSubSec)
	{
		drawHand (cr, centerX, posY + ((3 * faceSize) >> 2), faceSetting -> handPosition[HAND_SECS], &handStyle[HAND_SUBS]);
		drawCircle (cr, centerX, posY + ((3 * faceSize) >> 2), 2, SFILL_COLOUR, SEC___COLOUR);
	}
	if (faceSetting -> stopwatch)
	{
		drawHand (cr, posX + (faceSize >> 2), centerY, faceSetting -> handPosition[HAND_STOPWT], &handStyle[HAND_STOPWT]);
		drawHand (cr, posX + ((3 * faceSize) >> 2), centerY, faceSetting -> handPosition[HAND_STOPWM], &handStyle[HAND_STOPWM]);
		drawCircle (cr, posX + (faceSize >> 2), centerY, 2, WFILL_COLOUR, WATCH_COLOUR);			
		drawCircle (cr, posX + ((3 * faceSize) >> 2), centerY, 2, WFILL_COLOUR, WATCH_COLOUR);
	}

	drawHand (cr, centerX, centerY, faceSetting -> handPosition[HAND_HOUR], &handStyle[HAND_HOUR]);
	drawHand (cr, centerX, centerY, faceSetting -> handPosition[HAND_MINUTE], &handStyle[HAND_MINUTE]);

	if (faceSetting -> showSeconds || faceSetting -> stopwatch)
	{
		if (faceSetting -> stopwatch)
		{
			drawHand (cr, centerX, centerY, faceSetting -> handPosition[HAND_STOPWS], &handStyle[HAND_SECS]);
			drawCircle (cr, centerX, centerY, 4, WFILL_COLOUR, WATCH_COLOUR);
		}
		else if (faceSetting -> subSecond)
		{
			drawCircle (cr, centerX, centerY, 4, MFILL_COLOUR, MIN___COLOUR);
		}
		else
		{
			drawHand (cr, centerX, centerY, faceSetting -> handPosition[HAND_SECS], &handStyle[HAND_SECS]);
			drawCircle (cr, centerX, centerY, 4, SFILL_COLOUR, SEC___COLOUR);
		}
	}
	else
	{
		drawCircle (cr, centerX, centerY, 4, MFILL_COLOUR, MIN___COLOUR);
	}
	cairo_restore (cr);
	return TRUE;
}

#if GTK_MAJOR_VERSION == 2

/**********************************************************************************************************************
 *                                                                                                                    *
 *  C L O C K  E X P O S E                                                                                            *
 *  ======================                                                                                            *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief Call when the gauge needs to be drawn.
 *  @param widget .
 *  @result None.
 */
void clockExpose (GtkWidget *widget)
{
	int i, j, face = 0;
	cairo_t *cr = gdk_cairo_create (drawingArea -> window);

	for (j = 0; j < faceHeight; j++)
	{
		for (i = 0; i < faceWidth; i++)
		{
			drawFace (cr, face++, (i * faceSize), (j * faceSize), 0);
		}
	}

	/*------------------------------------------------------------------------------------------------*
	 * Reset the color and stuff back to the default.                                                 *
	 *------------------------------------------------------------------------------------------------*/
	cairo_destroy (cr);
	cr = NULL;	
}

#else

/**********************************************************************************************************************
 *                                                                                                                    *
 *  C L O C K  E X P O S E                                                                                            *
 *  ======================                                                                                            *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param cr .
 *  @result .
 */
void clockExpose (cairo_t *cr)
{
	int i, j, face = 0;
	for (j = 0; j < faceHeight; j++)
	{
		for (i = 0; i < faceWidth; i++)
		{
			drawFace (cr, face++, (i * faceSize), (j * faceSize), 0);
		}
	}
}

#endif

/**********************************************************************************************************************
 *                                                                                                                    *
 *  D I A L  S A V E                                                                                                  *
 *  ================                                                                                                  *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief Save the display to a file.
 *  @param fileName Name of the file to save the SVG in.
 *  @result None.
 */
int dialSave(char *fileName) 
{
	cairo_surface_t *surface;
	int i, j, face = 0;
	cairo_t *cr;

	surface = cairo_svg_surface_create (fileName, faceWidth * faceSize, faceHeight * faceSize);
	cr = cairo_create(surface);

	for (j = 0; j < faceHeight; j++)
	{
		for (i = 0; i < faceWidth; i++)
		{
			drawFace (cr, face++, (i * faceSize), (j * faceSize), 1);
		}
	}

	cairo_surface_destroy(surface);
	cairo_destroy(cr);

	return 0;
}

