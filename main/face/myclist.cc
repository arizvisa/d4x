/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2002 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "myclist.h"
#include "gdk/gdk.h"
#include <stdio.h>
#include "../var.h"
#include "../dlist.h"
#include "misc.h"

#ifdef FLT_ROUNDS
#undef FLT_ROUNDS
#endif
#define FLT_ROUNDS 3
static GtkWidgetClass *parent_class = (GtkWidgetClass *)NULL;

static void my_draw_row(GtkCList      *clist,
			GdkRectangle  *area,
			gint           row,
			GtkCListRow   *clist_row);

static void my_cell_size_request(GtkCList       *clist,
			         GtkCListRow    *clist_row,
			         gint            column,
				 GtkRequisition *requisition);

static void my_set_cell_contents(GtkCList      *clist,
			         GtkCListRow   *clist_row,
			         gint           column,
			         GtkCellType    type,
			         const gchar   *text,
			         guint8         spacing,
			         GdkPixmap     *pixmap,
			         GdkBitmap     *mask);

#define	ROW_ELEMENT(clist, row)	(((row) == (clist)->rows - 1) ? \
				 (clist)->row_list_end : \
				 g_list_nth ((clist)->row_list, (row)))
#define ROW_TOP_YPIXEL(clist, row) (((clist)->row_height * (row)) + \
				    (((row) + 1) * CELL_SPACING) + \
				    (clist)->voffset)
#define GTK_CLIST_CLASS_FW(_widget_) GTK_CLIST_CLASS (((GtkObject*) (_widget_))->klass)
#define CELL_SPACING 1
#define COLUMN_INSET 3
#define CLIST_UNFROZEN(clist)     (((GtkCList*) (clist))->freeze_count == 0)


static void my_gtk_clist_class_init(MyGtkCListClass *klass){
//	GtkObjectClass *object_class=(GtkObjectClass *)klass;	
	GtkCListClass *clist_klass=(GtkCListClass *)klass;
//	GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;
	clist_klass->draw_row = my_draw_row;
	clist_klass->cell_size_request = my_cell_size_request;
	clist_klass->set_cell_contents = my_set_cell_contents;
//	widget_class->drag_begin=NULL;//FIXME?
	parent_class=(GtkWidgetClass *)gtk_type_class(GTK_TYPE_CONTAINER);
};

static void my_gtk_clist_init(MyGtkCList *filesel){
};

GtkType my_gtk_clist_get_type(){
	static guint my_clist_type=0;
	if (!my_clist_type){
		GtkTypeInfo my_clist_info={
			"MyGtkCList",
			sizeof(MyGtkCList),
			sizeof(MyGtkCListClass),
			(GtkClassInitFunc)my_gtk_clist_class_init,
			(GtkObjectInitFunc)my_gtk_clist_init,
			NULL,NULL
//			(GtkArgSetFunc) NULL,
//			(GtkArgGetFunc) NULL
		};
		my_clist_type = gtk_type_unique (gtk_clist_get_type (), &my_clist_info);
	};
	return my_clist_type;
};

GtkWidget *my_gtk_clist_new_with_titles (gint   columns,
					 gchar *titles[]){
  GtkWidget *widget;

  widget = (GtkWidget *)gtk_type_new (MY_GTK_TYPE_CLIST);
  gtk_clist_construct (GTK_CLIST (widget), columns, titles);

  return widget;
}

GtkWidget *my_gtk_clist_new(gint columns){
	return my_gtk_clist_new_with_titles (columns, (char **)NULL);
};

void my_gtk_clist_set_progress(GtkCList    *clist,
			       gint         row,
			       gint         column,
			       float value){
	GtkCListRow *clist_row;
	
	g_return_if_fail (clist != NULL);
	g_return_if_fail (GTK_IS_CLIST (clist));
	
	if (row < 0 || row >= clist->rows)
		return;
	if (column < 0 || column >= clist->columns)
		return;
	
	clist_row =(GtkCListRow *) ROW_ELEMENT (clist, row)->data;
	
	/* if text is null, then the cell is empty */
	GTK_CLIST_CLASS_FW (clist)->set_cell_contents
		(clist, clist_row, column, (GtkCellType )GTK_CELL_PROGRESS, (char *)(&value), 0, (GdkPixmap*)NULL, (GdkBitmap*)NULL);
	
	/* redraw the list if it's not frozen */
	if (CLIST_UNFROZEN (clist)){
		if (gtk_clist_row_is_visible (clist, row) != GTK_VISIBILITY_NONE)
			GTK_CLIST_CLASS_FW (clist)->draw_row (clist, (GdkRectangle*)NULL, row, clist_row);
	}
}

/* static functions
 */


static void column_auto_resize (GtkCList    *clist,
				GtkCListRow *clist_row,
				gint         column,
				gint         old_width){
	/* resize column if needed for auto_resize */
	GtkRequisition requisition;

	if (!clist->column[column].auto_resize ||
	    GTK_CLIST_AUTO_RESIZE_BLOCKED(clist))
		return;

	if (clist_row)
		GTK_CLIST_CLASS_FW (clist)->cell_size_request (clist, clist_row,
							       column, &requisition);
	else
		requisition.width = 0;

	if (requisition.width > clist->column[column].width)
		gtk_clist_set_column_width (clist, column, requisition.width);
	else if (requisition.width < old_width &&
		 old_width == clist->column[column].width)    {
		GList *list;
		gint new_width = 0;

		/* run a "gtk_clist_optimal_column_width" but break, if
       * the column doesn't shrink */
		if (GTK_CLIST_SHOW_TITLES(clist) && clist->column[column].button)
			new_width = (clist->column[column].button->requisition.width -
				     (CELL_SPACING + (2 * COLUMN_INSET)));
		else
			new_width = 0;

		for (list = clist->row_list; list; list = list->next)	{
			GTK_CLIST_CLASS_FW (clist)->cell_size_request
				(clist, GTK_CLIST_ROW (list), column, &requisition);
			new_width = MAX (new_width, requisition.width);
			if (new_width == clist->column[column].width)
				break;
		}
		if (new_width < clist->column[column].width)
			gtk_clist_set_column_width
				(clist, column, MAX (new_width, clist->column[column].min_width));
	}
}

static void my_set_cell_contents (GtkCList    *clist,
				  GtkCListRow *clist_row,
		                  gint         column,
		                  GtkCellType  type,
				  const gchar *text,
				  guint8       spacing,
				  GdkPixmap   *pixmap,
				  GdkBitmap   *mask){
	GtkRequisition requisition;
	g_return_if_fail (clist != NULL);
	g_return_if_fail (GTK_IS_CLIST (clist));
	g_return_if_fail (clist_row != NULL);
  
	if (clist->column[column].auto_resize &&
	    !GTK_CLIST_AUTO_RESIZE_BLOCKED(clist))
		GTK_CLIST_CLASS_FW (clist)->cell_size_request (clist, clist_row,
							       column, &requisition);

	switch (clist_row->cell[column].type){
	case GTK_CELL_EMPTY:
		break;
	case GTK_CELL_TEXT:
		g_free (GTK_CELL_TEXT (clist_row->cell[column])->text);
		break;
	case GTK_CELL_PIXMAP:
		gdk_pixmap_unref (GTK_CELL_PIXMAP (clist_row->cell[column])->pixmap);
		if (GTK_CELL_PIXMAP (clist_row->cell[column])->mask)
			gdk_bitmap_unref (GTK_CELL_PIXMAP (clist_row->cell[column])->mask);
		break;
	case GTK_CELL_PIXTEXT:
		g_free (GTK_CELL_PIXTEXT (clist_row->cell[column])->text);
		gdk_pixmap_unref (GTK_CELL_PIXTEXT (clist_row->cell[column])->pixmap);
		if (GTK_CELL_PIXTEXT (clist_row->cell[column])->mask)
			gdk_bitmap_unref (GTK_CELL_PIXTEXT (clist_row->cell[column])->mask);
		break;
	case GTK_CELL_WIDGET:
		/* unimplimented */
		break;
	default:
		break;
	}
  
	clist_row->cell[column].type = GTK_CELL_EMPTY;
  
	switch (type){
	case GTK_CELL_TEXT:
		if (text){
			clist_row->cell[column].type = GTK_CELL_TEXT;
			GTK_CELL_TEXT (clist_row->cell[column])->text = g_strdup (text);
		}
		break;
	case GTK_CELL_PIXMAP:
		if (pixmap){
			clist_row->cell[column].type = GTK_CELL_PIXMAP;
			GTK_CELL_PIXMAP (clist_row->cell[column])->pixmap = pixmap;
			/* We set the mask even if it is NULL */
			GTK_CELL_PIXMAP (clist_row->cell[column])->mask = mask;
		}
		break;
	case GTK_CELL_PIXTEXT:
		if (text && pixmap){
			clist_row->cell[column].type = GTK_CELL_PIXTEXT;
			GTK_CELL_PIXTEXT (clist_row->cell[column])->text = g_strdup (text);
			GTK_CELL_PIXTEXT (clist_row->cell[column])->spacing = spacing;
			GTK_CELL_PIXTEXT (clist_row->cell[column])->pixmap = pixmap;
			GTK_CELL_PIXTEXT (clist_row->cell[column])->mask = mask;
		}
		break;
	case GTK_CELL_PROGRESS:
		if (text){
			clist_row->cell[column].type = (GtkCellType)GTK_CELL_PROGRESS;
			float *f = (float *)text;
			GTK_CELL_PROGRESS(clist_row->cell[column])->value = *f;
		};
		break;
	default:
		break;
	}
  
	if (clist->column[column].auto_resize &&
	    !GTK_CLIST_AUTO_RESIZE_BLOCKED(clist))
		column_auto_resize (clist, clist_row, column, requisition.width);
}

static void get_cell_style (GtkCList     *clist,
			    GtkCListRow  *clist_row,
			    gint          state,
			    gint          column,
			    GtkStyle    **style,
			    GdkGC       **fg_gc,
			    GdkGC       **bg_gc,
			    GdkGC	**pr_gc){
	gint fg_state;
	gint pr_state;

	if ((state == GTK_STATE_NORMAL) &&
	    (GTK_WIDGET (clist)->state == GTK_STATE_INSENSITIVE)){
		fg_state = pr_state = GTK_STATE_INSENSITIVE;
	}else{
		fg_state = state;
		pr_state = GTK_STATE_NORMAL;
	};

	if (clist_row->cell[column].style)    {
		if (style)
			*style = clist_row->cell[column].style;
		if (fg_gc)
			*fg_gc = clist_row->cell[column].style->fg_gc[fg_state];
		if (bg_gc) {
			if (state == GTK_STATE_SELECTED)
				*bg_gc = clist_row->cell[column].style->bg_gc[state];
			else
				*bg_gc = clist_row->cell[column].style->base_gc[state];
		}
		if (pr_gc)
			*pr_gc=clist_row->cell[column].style->fg_gc[pr_state];
	}  else if (clist_row->style)    {
		if (style)
			*style = clist_row->style;
		if (fg_gc)
			*fg_gc = clist_row->style->fg_gc[fg_state];
		if (bg_gc) {
			if (state == GTK_STATE_SELECTED)
				*bg_gc = clist_row->style->bg_gc[state];
			else
				*bg_gc = clist_row->style->base_gc[state];
		}
		if (pr_gc)
			*pr_gc=clist_row->style->fg_gc[pr_state];
	}  else    {
		if (style)
			*style = GTK_WIDGET (clist)->style;
		if (fg_gc)
			*fg_gc = GTK_WIDGET (clist)->style->fg_gc[fg_state];
		if (bg_gc) {
			if (state == GTK_STATE_SELECTED)
				*bg_gc = GTK_WIDGET (clist)->style->bg_gc[state];
			else
				*bg_gc = GTK_WIDGET (clist)->style->base_gc[state];
		}
		if (pr_gc)
			*pr_gc=GTK_WIDGET (clist)->style->fg_gc[pr_state];

		if (state != GTK_STATE_SELECTED)	{
			if (pr_gc && clist_row->fg_set)
				*pr_gc = clist->fg_gc;
			if (fg_gc && clist_row->fg_set)
				*fg_gc = clist->fg_gc;
			if (bg_gc && clist_row->bg_set)
				*bg_gc = clist->bg_gc;
		}
	}
}

static void my_cell_size_request (GtkCList       *clist,
			       GtkCListRow    *clist_row,
			       gint            column,
			       GtkRequisition *requisition){
	GtkStyle *style;
	gint width;
	gint height;

	g_return_if_fail (clist != NULL);
	g_return_if_fail (GTK_IS_CLIST (clist));
	g_return_if_fail (requisition != NULL);

	get_cell_style (clist, clist_row, GTK_STATE_NORMAL, column, &style,
			(GdkGC **)NULL, (GdkGC **)NULL, (GdkGC **)NULL);

	switch (clist_row->cell[column].type)   {
	case GTK_CELL_TEXT:
		requisition->width =
			gdk_string_width (style->font,
					  GTK_CELL_TEXT (clist_row->cell[column])->text);
		requisition->height = style->font->ascent + style->font->descent;
		break;
	case GTK_CELL_PIXTEXT:
		gdk_window_get_size (GTK_CELL_PIXTEXT (clist_row->cell[column])->pixmap,
				     &width, &height);
		requisition->width = width +
			GTK_CELL_PIXTEXT (clist_row->cell[column])->spacing +
			gdk_string_width (style->font,
					  GTK_CELL_TEXT (clist_row->cell[column])->text);

		requisition->height = MAX (style->font->ascent + style->font->descent,
					   height);
		break;
	case GTK_CELL_PIXMAP:
		gdk_window_get_size (GTK_CELL_PIXMAP (clist_row->cell[column])->pixmap,
				     &width, &height);
		requisition->width = width;
		requisition->height = height;
		break;
	case GTK_CELL_PROGRESS:
		char tmptext[100];
		d4x_percent_str(GTK_CELL_PROGRESS(clist_row->cell[column])->value,
			       tmptext,sizeof(tmptext));
		requisition->width = gdk_string_width (style->font,
						       tmptext);
		requisition->height = style->font->ascent + style->font->descent;
		break;
	default:
		requisition->width  = 0;
		requisition->height = 0;
		break;
	}

	requisition->width  += clist_row->cell[column].horizontal;
	requisition->height += clist_row->cell[column].vertical;
}

static gint draw_cell_pixmap (GdkWindow    *window,
			      GdkRectangle *clip_rectangle,
			      GdkGC        *fg_gc,
			      GdkPixmap    *pixmap,
			      GdkBitmap    *mask,
			      gint          x,
			      gint          y,
			      gint          width,
			      gint          height){
	gint xsrc = 0;
	gint ysrc = 0;
	
	if (mask) {
		gdk_gc_set_clip_mask (fg_gc, mask);
		gdk_gc_set_clip_origin (fg_gc, x, y);
	}
	
	if (x < clip_rectangle->x){
		xsrc = clip_rectangle->x - x;
		width -= xsrc;
		x = clip_rectangle->x;
	}
	if (x + width > clip_rectangle->x + clip_rectangle->width)
		width = clip_rectangle->x + clip_rectangle->width - x;
	
	if (y < clip_rectangle->y) {
		ysrc = clip_rectangle->y - y;
		height -= ysrc;
		y = clip_rectangle->y;
	}
	if (y + height > clip_rectangle->y + clip_rectangle->height)
		height = clip_rectangle->y + clip_rectangle->height - y;
	
	gdk_draw_pixmap (window, fg_gc, pixmap, xsrc, ysrc, x, y, width, height);
	gdk_gc_set_clip_origin (fg_gc, 0, 0);
	if (mask)
		gdk_gc_set_clip_mask (fg_gc, (GdkBitmap*) NULL);
	
	return x + MAX (width, 0);
}

static void my_draw_row (GtkCList     *clist,
			  GdkRectangle *area,
			  gint          row,
			  GtkCListRow  *clist_row){
	GtkWidget *widget;
	GdkRectangle *rect;
	GdkRectangle row_rectangle;
	GdkRectangle cell_rectangle;
	GdkRectangle clip_rectangle;
	GdkRectangle intersect_rectangle;
	gint last_column;
	gint state;
	gint i;
	
	g_return_if_fail (clist != NULL);
	
	/* bail now if we arn't drawable yet */
	if (!GTK_WIDGET_DRAWABLE (clist) || row < 0 || row >= clist->rows)
		return;
	
	widget = GTK_WIDGET (clist);
	
	/* if the function is passed the pointer to the row instead of null,
	 * it avoids this expensive lookup */
	if (!clist_row)
		clist_row = (GtkCListRow *)ROW_ELEMENT (clist, row)->data;
	
	/* rectangle of the entire row */
	row_rectangle.x = 0;
	row_rectangle.y = ROW_TOP_YPIXEL (clist, row);
	row_rectangle.width = clist->clist_window_width;
	row_rectangle.height = clist->row_height;
	
	/* rectangle of the cell spacing above the row */
	cell_rectangle.x = 0;
	cell_rectangle.y = row_rectangle.y - CELL_SPACING;
	cell_rectangle.width = row_rectangle.width;
	cell_rectangle.height = CELL_SPACING;
	
	/* rectangle used to clip drawing operations, its y and height
	 * positions only need to be set once, so we set them once here. 
	 * the x and width are set withing the drawing loop below once per
	 * column */
	clip_rectangle.y = row_rectangle.y;
	clip_rectangle.height = row_rectangle.height;

	if (clist_row->state == GTK_STATE_NORMAL){
		if (clist_row->fg_set)
			gdk_gc_set_foreground (clist->fg_gc, &clist_row->foreground);
		if (clist_row->bg_set)
			gdk_gc_set_foreground (clist->bg_gc, &clist_row->background);
	}

	state = clist_row->state;

  /* draw the cell borders and background */
	if (area){
		rect = &intersect_rectangle;
		if (gdk_rectangle_intersect (area, &cell_rectangle,
					     &intersect_rectangle))
			gdk_draw_rectangle (clist->clist_window,
					    widget->style->base_gc[GTK_STATE_ACTIVE],
					    TRUE,
					    intersect_rectangle.x,
					    intersect_rectangle.y,
					    intersect_rectangle.width,
					    intersect_rectangle.height);

		/* the last row has to clear its bottom cell spacing too */
		if (clist_row == clist->row_list_end->data){
			cell_rectangle.y += clist->row_height + CELL_SPACING;

			if (gdk_rectangle_intersect (area, &cell_rectangle,
						     &intersect_rectangle))
				gdk_draw_rectangle (clist->clist_window,
						    widget->style->base_gc[GTK_STATE_ACTIVE],
						    TRUE,
						    intersect_rectangle.x,
						    intersect_rectangle.y,
						    intersect_rectangle.width,
						    intersect_rectangle.height);
		}

		if (!gdk_rectangle_intersect (area, &row_rectangle,&intersect_rectangle))
			return;

	}else{
		rect = &clip_rectangle;
		gdk_draw_rectangle (clist->clist_window,
				    widget->style->base_gc[GTK_STATE_ACTIVE],
				    TRUE,
				    cell_rectangle.x,
				    cell_rectangle.y,
				    cell_rectangle.width,
				    cell_rectangle.height);

		/* the last row has to clear its bottom cell spacing too */
		if (clist_row == clist->row_list_end->data){
			cell_rectangle.y += clist->row_height + CELL_SPACING;

			gdk_draw_rectangle (clist->clist_window,
					    widget->style->base_gc[GTK_STATE_ACTIVE],
					    TRUE,
					    cell_rectangle.x,
					    cell_rectangle.y,
					    cell_rectangle.width,
					    cell_rectangle.height);     
		}	  
	}
  
	for (last_column = clist->columns - 1;
	     last_column >= 0 && !clist->column[last_column].visible; last_column--);

  /* iterate and draw all the columns (row cells) and draw their contents */
	for (i = 0; i < clist->columns; i++){
		GtkStyle *style;
		GdkGC *fg_gc;
		GdkGC *bg_gc;
		GdkGC *pr_gc;

		gint width;
		gint height;
		gint pixmap_width;
		gint offset = 0;
		gint row_center_offset;
		char tmptext[100];
		if (clist_row->cell[i].type==(GtkCellType)GTK_CELL_PROGRESS)
			d4x_percent_str(GTK_CELL_PROGRESS(clist_row->cell[i])->value, tmptext, sizeof(tmptext));

		if (!clist->column[i].visible)
			continue;

		get_cell_style (clist, clist_row, state, i, &style, &fg_gc, &bg_gc, &pr_gc);

		clip_rectangle.x = clist->column[i].area.x + clist->hoffset;
		clip_rectangle.width = clist->column[i].area.width;

		/* calculate clipping region clipping region */
		clip_rectangle.x -= COLUMN_INSET + CELL_SPACING;
		clip_rectangle.width += (2 * COLUMN_INSET + CELL_SPACING +
					 (i == last_column) * CELL_SPACING);
      
		if (area && !gdk_rectangle_intersect (area, &clip_rectangle,
						      &intersect_rectangle))
			continue;

		gdk_draw_rectangle (clist->clist_window, bg_gc, TRUE,
				    rect->x, rect->y, rect->width, rect->height);

		clip_rectangle.x += COLUMN_INSET + CELL_SPACING;
		clip_rectangle.width -= (2 * COLUMN_INSET + CELL_SPACING +
					 (i == last_column) * CELL_SPACING);

		/* calculate real width for column justification */
		pixmap_width = 0;
		offset = 0;
		switch (clist_row->cell[i].type){
		case GTK_CELL_TEXT:
			width = gdk_string_width (style->font,
						  GTK_CELL_TEXT (clist_row->cell[i])->text);
			break;
		case GTK_CELL_PIXMAP:
			gdk_window_get_size (GTK_CELL_PIXMAP (clist_row->cell[i])->pixmap,
					     &pixmap_width, &height);
			width = pixmap_width;
			break;
		case GTK_CELL_PIXTEXT:
			gdk_window_get_size (GTK_CELL_PIXTEXT (clist_row->cell[i])->pixmap,
					     &pixmap_width, &height);
			width = (pixmap_width +
				 GTK_CELL_PIXTEXT (clist_row->cell[i])->spacing +
				 gdk_string_width (style->font,
						   GTK_CELL_PIXTEXT
						   (clist_row->cell[i])->text));
			break;
		case GTK_CELL_PROGRESS:
			width = gdk_string_width (style->font,
						  tmptext);
//			width = clist->column[i].area.width;
			break;
		default:
			continue;
			break;
		}

		switch (clist->column[i].justification)
		{
		case GTK_JUSTIFY_LEFT:
			offset = clip_rectangle.x + clist_row->cell[i].horizontal;
			break;
		case GTK_JUSTIFY_RIGHT:
			offset = (clip_rectangle.x + clist_row->cell[i].horizontal +
				  clip_rectangle.width - width);
			break;
		case GTK_JUSTIFY_CENTER:
		case GTK_JUSTIFY_FILL:
			offset = (clip_rectangle.x + clist_row->cell[i].horizontal +
				  (clip_rectangle.width / 2) - (width / 2));
			break;
		};

		/* Draw Text and/or Pixmap */
		switch (clist_row->cell[i].type){
		case GTK_CELL_PIXMAP:
			draw_cell_pixmap (clist->clist_window, &clip_rectangle, fg_gc,
					  GTK_CELL_PIXMAP (clist_row->cell[i])->pixmap,
					  GTK_CELL_PIXMAP (clist_row->cell[i])->mask,
					  offset,
					  clip_rectangle.y + clist_row->cell[i].vertical +
					  (clip_rectangle.height - height) / 2,
					  pixmap_width, height);
			break;
		case GTK_CELL_PIXTEXT:
			offset = draw_cell_pixmap (clist->clist_window, &clip_rectangle, fg_gc,
						  GTK_CELL_PIXTEXT (clist_row->cell[i])->pixmap,
						  GTK_CELL_PIXTEXT (clist_row->cell[i])->mask,
						  offset,
						  clip_rectangle.y + clist_row->cell[i].vertical+
						  (clip_rectangle.height - height) / 2,
						  pixmap_width, height);
			offset += GTK_CELL_PIXTEXT (clist_row->cell[i])->spacing;
		case GTK_CELL_TEXT:
			if (style != GTK_WIDGET (clist)->style)
				row_center_offset = gint((((clist->row_height - style->font->ascent -
							    style->font->descent - 1) / 2) + 1.5 +
							  style->font->ascent));
			else
				row_center_offset = clist->row_center_offset;

			gdk_gc_set_clip_rectangle (fg_gc, &clip_rectangle);
			gdk_draw_string (clist->clist_window, style->font, fg_gc,
					 offset,
					 row_rectangle.y + row_center_offset + 
					 clist_row->cell[i].vertical,
					 (clist_row->cell[i].type == GTK_CELL_PIXTEXT) ?
					 GTK_CELL_PIXTEXT (clist_row->cell[i])->text :
					 GTK_CELL_TEXT (clist_row->cell[i])->text);
			gdk_gc_set_clip_rectangle (fg_gc, (GdkRectangle *)NULL);
			break;
		case GTK_CELL_PROGRESS:{
			if (style != GTK_WIDGET (clist)->style)
				row_center_offset = gint((((clist->row_height - style->font->ascent -
							    style->font->descent - 1) / 2) + 1.5 +
							  style->font->ascent));
			else
				row_center_offset = clist->row_center_offset;
			
			switch(CFG.PROGRESS_MODE){
			case 2:{
				tDownload *temp=(tDownload *)gtk_clist_get_row_data(clist,row);
				if (temp && temp->segments && temp->finfo.size>0){
					gdk_gc_set_clip_rectangle (pr_gc, &clip_rectangle);
					gtk_paint_box(style,clist->clist_window,
						      GTK_STATE_NORMAL,GTK_SHADOW_NONE,
						      &clip_rectangle,
						      (GtkWidget *)clist,
						      "trough",
						      clip_rectangle.x,clip_rectangle.y,
						      clip_rectangle.width,clip_rectangle.height);
					temp->segments->lock_public();
					tSegment *tmp=temp->segments->get_first();
					while(tmp){
						gint start=int(float(tmp->begin)*float(clip_rectangle.width)/float(temp->finfo.size));
						gint end=int(float(tmp->end)*float(clip_rectangle.width)/float(temp->finfo.size));
						if (end-start>=2)
							gtk_paint_box(style,clist->clist_window,
								      GTK_STATE_PRELIGHT,GTK_SHADOW_OUT,
								      &clip_rectangle,
								      (GtkWidget *)clist,
								      "bar",
								      clip_rectangle.x+start,clip_rectangle.y,
								      end-start,clip_rectangle.height);
						tmp=tmp->next;
					};
					temp->segments->unlock_public();
					gdk_draw_string (clist->clist_window, style->font, pr_gc,
							 offset,
							 row_rectangle.y + row_center_offset + 
							 clist_row->cell[i].vertical,
							 tmptext);
					gdk_gc_set_clip_rectangle (pr_gc, (GdkRectangle *)NULL);
					break;
				};
			};
			case 1:{
				gdk_gc_set_clip_rectangle (pr_gc, &clip_rectangle);
				gtk_paint_box(style,clist->clist_window,
					      GTK_STATE_NORMAL,GTK_SHADOW_NONE,
					      &clip_rectangle,
					      (GtkWidget *)clist,
					      "trough",
					      clip_rectangle.x,clip_rectangle.y,
					      clip_rectangle.width,clip_rectangle.height);
				float progress_width= GTK_CELL_PROGRESS(clist_row->cell[i])->value > 100 ? 100:GTK_CELL_PROGRESS(clist_row->cell[i])->value;
				if (progress_width<0)
					progress_width=0;
				progress_width=(progress_width*float(clip_rectangle.width))/float(100);
				if (progress_width>=2)
					gtk_paint_box(style,clist->clist_window,
						      GTK_STATE_PRELIGHT,GTK_SHADOW_OUT,
						      &clip_rectangle,
						      (GtkWidget *)clist,
						      "bar",
						      clip_rectangle.x,clip_rectangle.y,
						      gint(progress_width),clip_rectangle.height);
				gdk_draw_string (clist->clist_window, style->font, pr_gc,
						 offset,
						 row_rectangle.y + row_center_offset + 
						 clist_row->cell[i].vertical,
						 tmptext);
				gdk_gc_set_clip_rectangle (pr_gc, (GdkRectangle *)NULL);
				break;
			};
			default:
				gdk_gc_set_clip_rectangle (fg_gc, &clip_rectangle);
				gdk_draw_string (clist->clist_window, style->font, fg_gc,
						 offset,
						 row_rectangle.y + row_center_offset + 
						 clist_row->cell[i].vertical,
						 tmptext);
				
				gdk_gc_set_clip_rectangle (fg_gc, (GdkRectangle *)NULL);
				break;
			};
		};
		default:
			break;
		}
	}

	/* draw focus rectangle */
	if (clist->focus_row == row &&
	    GTK_WIDGET_CAN_FOCUS (widget) && GTK_WIDGET_HAS_FOCUS(widget)){
		if (!area)
			gdk_draw_rectangle (clist->clist_window, clist->xor_gc, FALSE,
					    row_rectangle.x, row_rectangle.y,
					    row_rectangle.width - 1, row_rectangle.height - 1);
		else if (gdk_rectangle_intersect (area, &row_rectangle,
						  &intersect_rectangle)){
			gdk_gc_set_clip_rectangle (clist->xor_gc, &intersect_rectangle);
			gdk_draw_rectangle (clist->clist_window, clist->xor_gc, FALSE,
					    row_rectangle.x, row_rectangle.y,
					    row_rectangle.width - 1,
					    row_rectangle.height - 1);
			gdk_gc_set_clip_rectangle (clist->xor_gc, (GdkRectangle *)NULL);
		}
	}
}
