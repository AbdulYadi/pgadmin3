//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Portions Copyright (C) 1998 - 2011, Julian Smart
// Portions Copyright (C) 2011 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// basic2.cpp - Basic OGL classes (2)
//
//////////////////////////////////////////////////////////////////////////

#include "pgAdmin3.h"

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "ogl/ogl.h"


// Control point types
// Rectangle and most other shapes
#define CONTROL_POINT_VERTICAL   1
#define CONTROL_POINT_HORIZONTAL 2
#define CONTROL_POINT_DIAGONAL   3

// Line
#define CONTROL_POINT_ENDPOINT_TO 4
#define CONTROL_POINT_ENDPOINT_FROM 5
#define CONTROL_POINT_LINE       6

// Two stage construction: need to call Create
IMPLEMENT_DYNAMIC_CLASS(wxPolygonShape, wxShape)

wxPolygonShape::wxPolygonShape()
{
	m_points = NULL;
	m_originalPoints = NULL;
}

void wxPolygonShape::Create(wxList *the_points)
{
	ClearPoints();

	if (!the_points)
	{
		m_originalPoints = new wxList;
		m_points = new wxList;
	}
	else
	{
		m_originalPoints = the_points;

		// Duplicate the list of points
		m_points = new wxList;

		wxNode *node = the_points->GetFirst();
		while (node)
		{
			wxRealPoint *point = (wxRealPoint *)node->GetData();
			wxRealPoint *new_point = new wxRealPoint(point->x, point->y);
			m_points->Append((wxObject *) new_point);
			node = node->GetNext();
		}
		CalculateBoundingBox();
		m_originalWidth = m_boundWidth;
		m_originalHeight = m_boundHeight;
		SetDefaultRegionSize();
	}
}

wxPolygonShape::~wxPolygonShape()
{
	ClearPoints();
}

void wxPolygonShape::ClearPoints()
{
	if (m_points)
	{
		wxNode *node = m_points->GetFirst();
		while (node)
		{
			wxRealPoint *point = (wxRealPoint *)node->GetData();
			delete point;
			delete node;
			node = m_points->GetFirst();
		}
		delete m_points;
		m_points = NULL;
	}
	if (m_originalPoints)
	{
		wxNode *node = m_originalPoints->GetFirst();
		while (node)
		{
			wxRealPoint *point = (wxRealPoint *)node->GetData();
			delete point;
			delete node;
			node = m_originalPoints->GetFirst();
		}
		delete m_originalPoints;
		m_originalPoints = NULL;
	}
}


// Width and height. Centre of object is centre of box.
void wxPolygonShape::GetBoundingBoxMin(double *width, double *height)
{
	*width = m_boundWidth;
	*height = m_boundHeight;
}

void wxPolygonShape::CalculateBoundingBox()
{
	// Calculate bounding box at construction (and presumably resize) time
	double left = 10000;
	double right = -10000;
	double top = 10000;
	double bottom = -10000;

	wxNode *node = m_points->GetFirst();
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		if (point->x < left) left = point->x;
		if (point->x > right) right = point->x;

		if (point->y < top) top = point->y;
		if (point->y > bottom) bottom = point->y;

		node = node->GetNext();
	}
	m_boundWidth = right - left;
	m_boundHeight = bottom - top;
}

// Recalculates the centre of the polygon, and
// readjusts the point offsets accordingly.
// Necessary since the centre of the polygon
// is expected to be the real centre of the bounding
// box.
void wxPolygonShape::CalculatePolygonCentre()
{
	double left = 10000;
	double right = -10000;
	double top = 10000;
	double bottom = -10000;

	wxNode *node = m_points->GetFirst();
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		if (point->x < left) left = point->x;
		if (point->x > right) right = point->x;

		if (point->y < top) top = point->y;
		if (point->y > bottom) bottom = point->y;

		node = node->GetNext();
	}
	double bwidth = right - left;
	double bheight = bottom - top;

	double newCentreX = (double)(left + (bwidth / 2.0));
	double newCentreY = (double)(top + (bheight / 2.0));

	node = m_points->GetFirst();
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		point->x -= newCentreX;
		point->y -= newCentreY;
		node = node->GetNext();
	}
	m_xpos += newCentreX;
	m_ypos += newCentreY;
}

bool PolylineHitTest(double n, double xvec[], double yvec[],
                     double x1, double y1, double x2, double y2)
{
	bool isAHit = FALSE;
	int i;
	double lastx = xvec[0];
	double lasty = yvec[0];

	double min_ratio = 1.0;
	double line_ratio;
	double other_ratio;

	for (i = 1; i < n; i++)
	{
		oglCheckLineIntersection(x1, y1, x2, y2, lastx, lasty, xvec[i], yvec[i],
		                         &line_ratio, &other_ratio);
		if (line_ratio != 1.0)
			isAHit = TRUE;
		lastx = xvec[i];
		lasty = yvec[i];

		if (line_ratio < min_ratio)
			min_ratio = line_ratio;
	}

	// Do last (implicit) line if last and first doubles are not identical
	if (!(xvec[0] == lastx && yvec[0] == lasty))
	{
		oglCheckLineIntersection(x1, y1, x2, y2, lastx, lasty, xvec[0], yvec[0],
		                         &line_ratio, &other_ratio);
		if (line_ratio != 1.0)
			isAHit = TRUE;

	}
	return isAHit;
}

bool wxPolygonShape::HitTest(double x, double y, int *attachment, double *distance)
{
	// Imagine four lines radiating from this point. If all of these lines hit the polygon,
	// we're inside it, otherwise we're not. Obviously we'd need more radiating lines
	// to be sure of correct results for very strange (concave) shapes.
	double endPointsX[4];
	double endPointsY[4];
	// North
	endPointsX[0] = x;
	endPointsY[0] = (double)(y - 1000.0);
	// East
	endPointsX[1] = (double)(x + 1000.0);
	endPointsY[1] = y;
	// South
	endPointsX[2] = x;
	endPointsY[2] = (double)(y + 1000.0);
	// West
	endPointsX[3] = (double)(x - 1000.0);
	endPointsY[3] = y;

	// Store polygon points in an array
	int np = m_points->GetCount();
	double *xpoints = new double[np];
	double *ypoints = new double[np];
	wxNode *node = m_points->GetFirst();
	int i = 0;
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		xpoints[i] = point->x + m_xpos;
		ypoints[i] = point->y + m_ypos;
		node = node->GetNext();
		i ++;
	}

	// We assume it's inside the polygon UNLESS one or more
	// lines don't hit the outline.
	bool isContained = TRUE;

	int noPoints = 4;
	for (i = 0; i < noPoints; i++)
	{
		if (!PolylineHitTest(np, xpoints, ypoints, x, y, endPointsX[i], endPointsY[i]))
			isContained = FALSE;
	}
	/*
	  if (isContained)
	    ClipsErrorFunction("It's a hit!\n");
	  else
	    ClipsErrorFunction("No hit.\n");
	*/
	delete[] xpoints;
	delete[] ypoints;

	if (!isContained)
		return FALSE;

	int nearest_attachment = 0;

	// If a hit, check the attachment points within the object.
	int n = GetNumberOfAttachments();
	double nearest = 999999.0;

	for (i = 0; i < n; i++)
	{
		double xp, yp;
		if (GetAttachmentPositionEdge(i, &xp, &yp))
		{
			double l = (double)sqrt(((xp - x) * (xp - x)) +
			                        ((yp - y) * (yp - y)));
			if (l < nearest)
			{
				nearest = l;
				nearest_attachment = i;
			}
		}
	}
	*attachment = nearest_attachment;
	*distance = nearest;
	return TRUE;
}

// Really need to be able to reset the shape! Otherwise, if the
// points ever go to zero, we've lost it, and can't resize.
void wxPolygonShape::SetSize(double new_width, double new_height, bool WXUNUSED(recursive))
{
	SetAttachmentSize(new_width, new_height);

	// Multiply all points by proportion of new size to old size
	double x_proportion = (double)(fabs(new_width / m_originalWidth));
	double y_proportion = (double)(fabs(new_height / m_originalHeight));

	wxNode *node = m_points->GetFirst();
	wxNode *original_node = m_originalPoints->GetFirst();
	while (node && original_node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		wxRealPoint *original_point = (wxRealPoint *)original_node->GetData();

		point->x = (original_point->x * x_proportion);
		point->y = (original_point->y * y_proportion);

		node = node->GetNext();
		original_node = original_node->GetNext();
	}

//  CalculateBoundingBox();
	m_boundWidth = (double)fabs(new_width);
	m_boundHeight = (double)fabs(new_height);
	SetDefaultRegionSize();
}

// Make the original points the same as the working points
void wxPolygonShape::UpdateOriginalPoints()
{
	if (!m_originalPoints) m_originalPoints = new wxList;
	wxNode *original_node = m_originalPoints->GetFirst();
	while (original_node)
	{
		wxNode *next_node = original_node->GetNext();
		wxRealPoint *original_point = (wxRealPoint *)original_node->GetData();
		delete original_point;
		delete original_node;

		original_node = next_node;
	}

	wxNode *node = m_points->GetFirst();
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		wxRealPoint *original_point = new wxRealPoint(point->x, point->y);
		m_originalPoints->Append((wxObject *) original_point);

		node = node->GetNext();
	}
	CalculateBoundingBox();
	m_originalWidth = m_boundWidth;
	m_originalHeight = m_boundHeight;
}

void wxPolygonShape::AddPolygonPoint(int pos)
{
	wxNode *node = m_points->Item(pos);
	if (!node) node = m_points->GetFirst();
	wxRealPoint *firstPoint = (wxRealPoint *)node->GetData();

	wxNode *node2 = m_points->Item(pos + 1);
	if (!node2) node2 = m_points->GetFirst();
	wxRealPoint *secondPoint = (wxRealPoint *)node2->GetData();

	double x = (double)((secondPoint->x - firstPoint->x) / 2.0 + firstPoint->x);
	double y = (double)((secondPoint->y - firstPoint->y) / 2.0 + firstPoint->y);
	wxRealPoint *point = new wxRealPoint(x, y);

	if (pos >= (int) (m_points->GetCount() - 1))
		m_points->Append((wxObject *) point);
	else
		m_points->Insert(node2, (wxObject *) point);

	UpdateOriginalPoints();

	if (m_selected)
	{
		DeleteControlPoints();
		MakeControlPoints();
	}
}

void wxPolygonShape::DeletePolygonPoint(int pos)
{
	wxNode *node = m_points->Item(pos);
	if (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		delete point;
		delete node;
		UpdateOriginalPoints();
		if (m_selected)
		{
			DeleteControlPoints();
			MakeControlPoints();
		}
	}
}

// Assume (x1, y1) is centre of box (most generally, line end at box)
bool wxPolygonShape::GetPerimeterPoint(double x1, double y1,
                                       double x2, double y2,
                                       double *x3, double *y3)
{
	int n = m_points->GetCount();

	// First check for situation where the line is vertical,
	// and we would want to connect to a point on that vertical --
	// oglFindEndForPolyline can't cope with this (the arrow
	// gets drawn to the wrong place).
	if ((m_attachmentMode == ATTACHMENT_MODE_NONE) && (x1 == x2))
	{
		// Look for the point we'd be connecting to. This is
		// a heuristic...
		wxNode *node = m_points->GetFirst();
		while (node)
		{
			wxRealPoint *point = (wxRealPoint *)node->GetData();
			if (point->x == 0.0)
			{
				if ((y2 > y1) && (point->y > 0.0))
				{
					*x3 = point->x + m_xpos;
					*y3 = point->y + m_ypos;
					return TRUE;
				}
				else if ((y2 < y1) && (point->y < 0.0))
				{
					*x3 = point->x + m_xpos;
					*y3 = point->y + m_ypos;
					return TRUE;
				}
			}
			node = node->GetNext();
		}
	}

	double *xpoints = new double[n];
	double *ypoints = new double[n];

	wxNode *node = m_points->GetFirst();
	int i = 0;
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		xpoints[i] = point->x + m_xpos;
		ypoints[i] = point->y + m_ypos;
		node = node->GetNext();
		i ++;
	}

	oglFindEndForPolyline(n, xpoints, ypoints,
	                      x1, y1, x2, y2, x3, y3);

	delete[] xpoints;
	delete[] ypoints;

	return TRUE;
}

void wxPolygonShape::OnDraw(wxDC &dc)
{
	int n = m_points->GetCount();
	wxPoint *intPoints = new wxPoint[n];
	int i;
	for (i = 0; i < n; i++)
	{
		wxRealPoint *point = (wxRealPoint *) m_points->Item(i)->GetData();
		intPoints[i].x = WXROUND(point->x);
		intPoints[i].y = WXROUND(point->y);
	}

	if (m_shadowMode != SHADOW_NONE)
	{
		if (m_shadowBrush)
			dc.SetBrush(* m_shadowBrush);
		dc.SetPen(* g_oglTransparentPen);

		dc.DrawPolygon(n, intPoints, WXROUND(m_xpos + m_shadowOffsetX), WXROUND(m_ypos + m_shadowOffsetY));
	}

	if (m_pen)
	{
		if (m_pen->GetWidth() == 0)
			dc.SetPen(* g_oglTransparentPen);
		else
			dc.SetPen(* m_pen);
	}
	if (m_brush)
		dc.SetBrush(* m_brush);
	dc.DrawPolygon(n, intPoints, WXROUND(m_xpos), WXROUND(m_ypos));

	delete[] intPoints;
}

void wxPolygonShape::OnDrawOutline(wxDC &dc, double x, double y, double w, double h)
{
	dc.SetBrush(* wxTRANSPARENT_BRUSH);
	// Multiply all points by proportion of new size to old size
	double x_proportion = (double)(fabs(w / m_originalWidth));
	double y_proportion = (double)(fabs(h / m_originalHeight));

	int n = m_originalPoints->GetCount();
	wxPoint *intPoints = new wxPoint[n];
	int i;
	for (i = 0; i < n; i++)
	{
		wxRealPoint *point = (wxRealPoint *) m_originalPoints->Item(i)->GetData();
		intPoints[i].x = WXROUND(x_proportion * point->x);
		intPoints[i].y = WXROUND(y_proportion * point->y);
	}
	dc.DrawPolygon(n, intPoints, WXROUND(x), WXROUND(y));
	delete[] intPoints;
}

// Make as many control points as there are vertices.
void wxPolygonShape::MakeControlPoints()
{
	wxNode *node = m_points->GetFirst();
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		wxPolygonControlPoint *control = new wxPolygonControlPoint(m_canvas, this, CONTROL_POINT_SIZE,
		        point, point->x, point->y);
		m_canvas->AddShape(control);
		m_controlPoints.Append(control);
		node = node->GetNext();
	}
}

void wxPolygonShape::ResetControlPoints()
{
	wxNode *node = m_points->GetFirst();
	wxNode *controlPointNode = m_controlPoints.GetFirst();
	while (node && controlPointNode)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		wxPolygonControlPoint *controlPoint = (wxPolygonControlPoint *)controlPointNode->GetData();

		controlPoint->m_xoffset = point->x;
		controlPoint->m_yoffset = point->y;
		controlPoint->m_polygonVertex = point;

		node = node->GetNext();
		controlPointNode = controlPointNode->GetNext();
	}
}


#if wxUSE_PROLOGIO
void wxPolygonShape::WriteAttributes(wxExpr *clause)
{
	wxShape::WriteAttributes(clause);

	clause->AddAttributeValue(wxT("x"), m_xpos);
	clause->AddAttributeValue(wxT("y"), m_ypos);

	// Make a list of lists for the coordinates
	wxExpr *list = new wxExpr(wxExprList);
	wxNode *node = m_points->GetFirst();
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		wxExpr *point_list = new wxExpr(wxExprList);
		wxExpr *x_expr = new wxExpr((double)point->x);
		wxExpr *y_expr = new wxExpr((double)point->y);

		point_list->Append(x_expr);
		point_list->Append(y_expr);
		list->Append(point_list);

		node = node->GetNext();
	}
	clause->AddAttributeValue(wxT("points"), list);

	// Save the original (unscaled) points
	list = new wxExpr(wxExprList);
	node = m_originalPoints->GetFirst();
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		wxExpr *point_list = new wxExpr(wxExprList);
		wxExpr *x_expr = new wxExpr((double) point->x);
		wxExpr *y_expr = new wxExpr((double) point->y);
		point_list->Append(x_expr);
		point_list->Append(y_expr);
		list->Append(point_list);

		node = node->GetNext();
	}
	clause->AddAttributeValue(wxT("m_originalPoints"), list);
}

void wxPolygonShape::ReadAttributes(wxExpr *clause)
{
	wxShape::ReadAttributes(clause);

	// Read a list of lists
	m_points = new wxList;
	m_originalPoints = new wxList;

	wxExpr *points_list = NULL;
	clause->AssignAttributeValue(wxT("points"), &points_list);

	// If no points_list, don't crash!! Assume a diamond instead.
	double the_height = 100.0;
	double the_width = 100.0;
	if (!points_list)
	{
		wxRealPoint *point = new wxRealPoint(0.0, (-the_height / 2));
		m_points->Append((wxObject *) point);

		point = new wxRealPoint((the_width / 2), 0.0);
		m_points->Append((wxObject *) point);

		point = new wxRealPoint(0.0, (the_height / 2));
		m_points->Append((wxObject *) point);

		point = new wxRealPoint((-the_width / 2), 0.0);
		m_points->Append((wxObject *) point);

		point = new wxRealPoint(0.0, (-the_height / 2));
		m_points->Append((wxObject *) point);
	}
	else
	{
		wxExpr *node = points_list->value.first;

		while (node)
		{
			wxExpr *xexpr = node->value.first;
			long x = xexpr->IntegerValue();

			wxExpr *yexpr = xexpr->next;
			long y = yexpr->IntegerValue();

			wxRealPoint *point = new wxRealPoint((double)x, (double)y);
			m_points->Append((wxObject *) point);

			node = node->next;
		}
	}

	points_list = NULL;
	clause->AssignAttributeValue(wxT("m_originalPoints"), &points_list);

	// If no points_list, don't crash!! Assume a diamond instead.
	if (!points_list)
	{
		wxRealPoint *point = new wxRealPoint(0.0, (-the_height / 2));
		m_originalPoints->Append((wxObject *) point);

		point = new wxRealPoint((the_width / 2), 0.0);
		m_originalPoints->Append((wxObject *) point);

		point = new wxRealPoint(0.0, (the_height / 2));
		m_originalPoints->Append((wxObject *) point);

		point = new wxRealPoint((-the_width / 2), 0.0);
		m_originalPoints->Append((wxObject *) point);

		point = new wxRealPoint(0.0, (-the_height / 2));
		m_originalPoints->Append((wxObject *) point);

		m_originalWidth = the_width;
		m_originalHeight = the_height;
	}
	else
	{
		wxExpr *node = points_list->value.first;
		double min_x = 1000;
		double min_y = 1000;
		double max_x = -1000;
		double max_y = -1000;
		while (node)
		{
			wxExpr *xexpr = node->value.first;
			long x = xexpr->IntegerValue();

			wxExpr *yexpr = xexpr->next;
			long y = yexpr->IntegerValue();

			wxRealPoint *point = new wxRealPoint((double)x, (double)y);
			m_originalPoints->Append((wxObject *) point);

			if (x < min_x)
				min_x = (double)x;
			if (y < min_y)
				min_y = (double)y;
			if (x > max_x)
				max_x = (double)x;
			if (y > max_y)
				max_y = (double)y;

			node = node->next;
		}
		m_originalWidth = max_x - min_x;
		m_originalHeight = max_y - min_y;
	}

	CalculateBoundingBox();
}
#endif

void wxPolygonShape::Copy(wxShape &copy)
{
	wxShape::Copy(copy);

	wxASSERT( copy.IsKindOf(CLASSINFO(wxPolygonShape)) );

	wxPolygonShape &polyCopy = (wxPolygonShape &) copy;

	polyCopy.ClearPoints();

	polyCopy.m_points = new wxList;
	polyCopy.m_originalPoints = new wxList;

	wxNode *node = m_points->GetFirst();
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		wxRealPoint *new_point = new wxRealPoint(point->x, point->y);
		polyCopy.m_points->Append((wxObject *) new_point);
		node = node->GetNext();
	}
	node = m_originalPoints->GetFirst();
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		wxRealPoint *new_point = new wxRealPoint(point->x, point->y);
		polyCopy.m_originalPoints->Append((wxObject *) new_point);
		node = node->GetNext();
	}
	polyCopy.m_boundWidth = m_boundWidth;
	polyCopy.m_boundHeight = m_boundHeight;
	polyCopy.m_originalWidth = m_originalWidth;
	polyCopy.m_originalHeight = m_originalHeight;
}

int wxPolygonShape::GetNumberOfAttachments() const
{
	int maxN = (m_points ? (m_points->GetCount() - 1) : 0);
	wxNode *node = m_attachmentPoints.GetFirst();
	while (node)
	{
		wxAttachmentPoint *point = (wxAttachmentPoint *)node->GetData();
		if (point->m_id > maxN)
			maxN = point->m_id;
		node = node->GetNext();
	}
	return maxN + 1;;
}

bool wxPolygonShape::GetAttachmentPosition(int attachment, double *x, double *y,
        int nth, int no_arcs, wxLineShape *line)
{
	if ((m_attachmentMode == ATTACHMENT_MODE_EDGE) && m_points && attachment < (int) m_points->GetCount())
	{
		wxRealPoint *point = (wxRealPoint *)m_points->Item(attachment)->GetData();
		*x = point->x + m_xpos;
		*y = point->y + m_ypos;
		return TRUE;
	}
	else
	{
		return wxShape::GetAttachmentPosition(attachment, x, y, nth, no_arcs, line);
	}
}

bool wxPolygonShape::AttachmentIsValid(int attachment) const
{
	if (!m_points)
		return FALSE;

	if ((attachment >= 0) && (attachment < (int) m_points->GetCount()))
		return TRUE;

	wxNode *node = m_attachmentPoints.GetFirst();
	while (node)
	{
		wxAttachmentPoint *point = (wxAttachmentPoint *)node->GetData();
		if (point->m_id == attachment)
			return TRUE;
		node = node->GetNext();
	}
	return FALSE;
}

// Rotate about the given axis by the given amount in radians
void wxPolygonShape::Rotate(double x, double y, double theta)
{
	double actualTheta = theta - m_rotation;

	// Rotate attachment points
	double sinTheta = (double)sin(actualTheta);
	double cosTheta = (double)cos(actualTheta);
	wxNode *node = m_attachmentPoints.GetFirst();
	while (node)
	{
		wxAttachmentPoint *point = (wxAttachmentPoint *)node->GetData();
		double x1 = point->m_x;
		double y1 = point->m_y;
		point->m_x = x1 * cosTheta - y1 * sinTheta + x * (1.0 - cosTheta) + y * sinTheta;
		point->m_y = x1 * sinTheta + y1 * cosTheta + y * (1.0 - cosTheta) + x * sinTheta;
		node = node->GetNext();
	}

	node = m_points->GetFirst();
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		double x1 = point->x;
		double y1 = point->y;
		point->x = x1 * cosTheta - y1 * sinTheta + x * (1.0 - cosTheta) + y * sinTheta;
		point->y = x1 * sinTheta + y1 * cosTheta + y * (1.0 - cosTheta) + x * sinTheta;
		node = node->GetNext();
	}
	node = m_originalPoints->GetFirst();
	while (node)
	{
		wxRealPoint *point = (wxRealPoint *)node->GetData();
		double x1 = point->x;
		double y1 = point->y;
		point->x = x1 * cosTheta - y1 * sinTheta + x * (1.0 - cosTheta) + y * sinTheta;
		point->y = x1 * sinTheta + y1 * cosTheta + y * (1.0 - cosTheta) + x * sinTheta;
		node = node->GetNext();
	}

	m_rotation = theta;

	CalculatePolygonCentre();
	CalculateBoundingBox();
	ResetControlPoints();
}

// Rectangle object

IMPLEMENT_DYNAMIC_CLASS(wxRectangleShape, wxShape)

wxRectangleShape::wxRectangleShape(double w, double h)
{
	m_width = w;
	m_height = h;
	m_cornerRadius = 0.0;
	SetDefaultRegionSize();
}

void wxRectangleShape::OnDraw(wxDC &dc)
{
	double x1 = (double)(m_xpos - m_width / 2.0);
	double y1 = (double)(m_ypos - m_height / 2.0);

	if (m_shadowMode != SHADOW_NONE)
	{
		if (m_shadowBrush)
			dc.SetBrush(* m_shadowBrush);
		dc.SetPen(* g_oglTransparentPen);

		if (m_cornerRadius != 0.0)
			dc.DrawRoundedRectangle(WXROUND(x1 + m_shadowOffsetX), WXROUND(y1 + m_shadowOffsetY),
			                        WXROUND(m_width), WXROUND(m_height), m_cornerRadius);
		else
			dc.DrawRectangle(WXROUND(x1 + m_shadowOffsetX), WXROUND(y1 + m_shadowOffsetY), WXROUND(m_width), WXROUND(m_height));
	}

	if (m_pen)
	{
		if (m_pen->GetWidth() == 0)
			dc.SetPen(* g_oglTransparentPen);
		else
			dc.SetPen(* m_pen);
	}
	if (m_brush)
		dc.SetBrush(* m_brush);

	if (m_cornerRadius != 0.0)
		dc.DrawRoundedRectangle(WXROUND(x1), WXROUND(y1), WXROUND(m_width), WXROUND(m_height), m_cornerRadius);
	else
		dc.DrawRectangle(WXROUND(x1), WXROUND(y1), WXROUND(m_width), WXROUND(m_height));
}

void wxRectangleShape::GetBoundingBoxMin(double *the_width, double *the_height)
{
	*the_width = m_width;
	*the_height = m_height;
}

void wxRectangleShape::SetSize(double x, double y, bool WXUNUSED(recursive))
{
	SetAttachmentSize(x, y);
	m_width = (double)wxMax(x, 1.0);
	m_height = (double)wxMax(y, 1.0);
	SetDefaultRegionSize();
}

void wxRectangleShape::SetCornerRadius(double rad)
{
	m_cornerRadius = rad;
}

// Assume (x1, y1) is centre of box (most generally, line end at box)
bool wxRectangleShape::GetPerimeterPoint(double WXUNUSED(x1), double WXUNUSED(y1),
        double x2, double y2,
        double *x3, double *y3)
{
	double bound_x, bound_y;
	GetBoundingBoxMax(&bound_x, &bound_y);
	oglFindEndForBox(bound_x, bound_y, m_xpos, m_ypos, x2, y2, x3, y3);

	return TRUE;
}

#if wxUSE_PROLOGIO
void wxRectangleShape::WriteAttributes(wxExpr *clause)
{
	wxShape::WriteAttributes(clause);
	clause->AddAttributeValue(wxT("x"), m_xpos);
	clause->AddAttributeValue(wxT("y"), m_ypos);

	clause->AddAttributeValue(wxT("width"), m_width);
	clause->AddAttributeValue(wxT("height"), m_height);
	if (m_cornerRadius != 0.0)
		clause->AddAttributeValue(wxT("corner"), m_cornerRadius);
}

void wxRectangleShape::ReadAttributes(wxExpr *clause)
{
	wxShape::ReadAttributes(clause);
	clause->AssignAttributeValue(wxT("width"), &m_width);
	clause->AssignAttributeValue(wxT("height"), &m_height);
	clause->AssignAttributeValue(wxT("corner"), &m_cornerRadius);

	// In case we're reading an old file, set the region's size
	if (m_regions.GetCount() == 1)
	{
		wxShapeRegion *region = (wxShapeRegion *)m_regions.GetFirst()->GetData();
		region->SetSize(m_width, m_height);
	}
}
#endif

void wxRectangleShape::Copy(wxShape &copy)
{
	wxShape::Copy(copy);

	wxASSERT( copy.IsKindOf(CLASSINFO(wxRectangleShape)) );

	wxRectangleShape &rectCopy = (wxRectangleShape &) copy;
	rectCopy.m_width = m_width;
	rectCopy.m_height = m_height;
	rectCopy.m_cornerRadius = m_cornerRadius;
}

int wxRectangleShape::GetNumberOfAttachments() const
{
	return wxShape::GetNumberOfAttachments();
}


// There are 4 attachment points on a rectangle - 0 = top, 1 = right, 2 = bottom,
// 3 = left.
bool wxRectangleShape::GetAttachmentPosition(int attachment, double *x, double *y,
        int nth, int no_arcs, wxLineShape *line)
{
	return wxShape::GetAttachmentPosition(attachment, x, y, nth, no_arcs, line);
}

// Text object (no box)

IMPLEMENT_DYNAMIC_CLASS(wxTextShape, wxRectangleShape)

wxTextShape::wxTextShape(double width, double height):
	wxRectangleShape(width, height)
{
}

void wxTextShape::OnDraw(wxDC &WXUNUSED(dc))
{
}

void wxTextShape::Copy(wxShape &copy)
{
	wxRectangleShape::Copy(copy);
}

#if wxUSE_PROLOGIO
void wxTextShape::WriteAttributes(wxExpr *clause)
{
	wxRectangleShape::WriteAttributes(clause);
}
#endif

// Ellipse object

IMPLEMENT_DYNAMIC_CLASS(wxEllipseShape, wxShape)

wxEllipseShape::wxEllipseShape(double w, double h)
{
	m_width = w;
	m_height = h;
	SetDefaultRegionSize();
}

void wxEllipseShape::GetBoundingBoxMin(double *w, double *h)
{
	*w = m_width;
	*h = m_height;
}

bool wxEllipseShape::GetPerimeterPoint(double x1, double y1,
                                       double x2, double y2,
                                       double *x3, double *y3)
{
	double bound_x, bound_y;
	GetBoundingBoxMax(&bound_x, &bound_y);

//  oglFindEndForBox(bound_x, bound_y, m_xpos, m_ypos, x2, y2, x3, y3);
	oglDrawArcToEllipse(m_xpos, m_ypos, bound_x, bound_y, x2, y2, x1, y1, x3, y3);

	return TRUE;
}

void wxEllipseShape::OnDraw(wxDC &dc)
{
	if (m_shadowMode != SHADOW_NONE)
	{
		if (m_shadowBrush)
			dc.SetBrush(* m_shadowBrush);
		dc.SetPen(* g_oglTransparentPen);
		dc.DrawEllipse((long) ((m_xpos - GetWidth() / 2) + m_shadowOffsetX),
		               (long) ((m_ypos - GetHeight() / 2) + m_shadowOffsetY),
		               (long) GetWidth(), (long) GetHeight());
	}

	if (m_pen)
	{
		if (m_pen->GetWidth() == 0)
			dc.SetPen(* g_oglTransparentPen);
		else
			dc.SetPen(* m_pen);
	}
	if (m_brush)
		dc.SetBrush(* m_brush);
	dc.DrawEllipse((long) (m_xpos - GetWidth() / 2), (long) (m_ypos - GetHeight() / 2), (long) GetWidth(), (long) GetHeight());
}

void wxEllipseShape::SetSize(double x, double y, bool WXUNUSED(recursive))
{
	SetAttachmentSize(x, y);
	m_width = x;
	m_height = y;
	SetDefaultRegionSize();
}

#if wxUSE_PROLOGIO
void wxEllipseShape::WriteAttributes(wxExpr *clause)
{
	wxShape::WriteAttributes(clause);
	clause->AddAttributeValue(wxT("x"), m_xpos);
	clause->AddAttributeValue(wxT("y"), m_ypos);

	clause->AddAttributeValue(wxT("width"), m_width);
	clause->AddAttributeValue(wxT("height"), m_height);
}

void wxEllipseShape::ReadAttributes(wxExpr *clause)
{
	wxShape::ReadAttributes(clause);
	clause->AssignAttributeValue(wxT("width"), &m_width);
	clause->AssignAttributeValue(wxT("height"), &m_height);

	// In case we're reading an old file, set the region's size
	if (m_regions.GetCount() == 1)
	{
		wxShapeRegion *region = (wxShapeRegion *)m_regions.GetFirst()->GetData();
		region->SetSize(m_width, m_height);
	}
}
#endif

void wxEllipseShape::Copy(wxShape &copy)
{
	wxShape::Copy(copy);

	wxASSERT( copy.IsKindOf(CLASSINFO(wxEllipseShape)) );

	wxEllipseShape &ellipseCopy = (wxEllipseShape &) copy;

	ellipseCopy.m_width = m_width;
	ellipseCopy.m_height = m_height;
}

int wxEllipseShape::GetNumberOfAttachments() const
{
	return wxShape::GetNumberOfAttachments();
}

// There are 4 attachment points on an ellipse - 0 = top, 1 = right, 2 = bottom,
// 3 = left.
bool wxEllipseShape::GetAttachmentPosition(int attachment, double *x, double *y,
        int nth, int no_arcs, wxLineShape *line)
{
	if (m_attachmentMode == ATTACHMENT_MODE_BRANCHING)
		return wxShape::GetAttachmentPosition(attachment, x, y, nth, no_arcs, line);

	if (m_attachmentMode != ATTACHMENT_MODE_NONE)
	{
		double top = (double)(m_ypos + m_height / 2.0);
		double bottom = (double)(m_ypos - m_height / 2.0);
		double left = (double)(m_xpos - m_width / 2.0);
		double right = (double)(m_xpos + m_width / 2.0);

		int physicalAttachment = LogicalToPhysicalAttachment(attachment);

		switch (physicalAttachment)
		{
			case 0:
			{
				if (m_spaceAttachments)
					*x = left + (nth + 1) * m_width / (no_arcs + 1);
				else *x = m_xpos;
				*y = top;
				// We now have the point on the bounding box: but get the point on the ellipse
				// by imagining a vertical line from (*x, m_ypos - m_height- 500) to (*x, m_ypos) intersecting
				// the ellipse.
				oglDrawArcToEllipse(m_xpos, m_ypos, m_width, m_height, *x, (double)(m_ypos - m_height - 500), *x, m_ypos, x, y);
				break;
			}
			case 1:
			{
				*x = right;
				if (m_spaceAttachments)
					*y = bottom + (nth + 1) * m_height / (no_arcs + 1);
				else *y = m_ypos;
				oglDrawArcToEllipse(m_xpos, m_ypos, m_width, m_height, (double)(m_xpos + m_width + 500), *y, m_xpos, *y, x, y);
				break;
			}
			case 2:
			{
				if (m_spaceAttachments)
					*x = left + (nth + 1) * m_width / (no_arcs + 1);
				else *x = m_xpos;
				*y = bottom;
				oglDrawArcToEllipse(m_xpos, m_ypos, m_width, m_height, *x, (double)(m_ypos + m_height + 500), *x, m_ypos, x, y);
				break;
			}
			case 3:
			{
				*x = left;
				if (m_spaceAttachments)
					*y = bottom + (nth + 1) * m_height / (no_arcs + 1);
				else *y = m_ypos;
				oglDrawArcToEllipse(m_xpos, m_ypos, m_width, m_height, (double)(m_xpos - m_width - 500), *y, m_xpos, *y, x, y);
				break;
			}
			default:
			{
				return wxShape::GetAttachmentPosition(attachment, x, y, nth, no_arcs, line);
			}
		}
		return TRUE;
	}
	else
	{
		*x = m_xpos;
		*y = m_ypos;
		return TRUE;
	}
}


// Circle object
IMPLEMENT_DYNAMIC_CLASS(wxCircleShape, wxEllipseShape)

wxCircleShape::wxCircleShape(double diameter): wxEllipseShape(diameter, diameter)
{
	SetMaintainAspectRatio(TRUE);
}

void wxCircleShape::Copy(wxShape &copy)
{
	wxEllipseShape::Copy(copy);
}

bool wxCircleShape::GetPerimeterPoint(double WXUNUSED(x1), double WXUNUSED(y1),
                                      double x2, double y2,
                                      double *x3, double *y3)
{
	oglFindEndForCircle(m_width / 2,
	                    m_xpos, m_ypos,  // Centre of circle
	                    x2, y2,  // Other end of line
	                    x3, y3);

	return TRUE;
}

// Control points

double wxControlPoint::sm_controlPointDragStartX = 0.0;
double wxControlPoint::sm_controlPointDragStartY = 0.0;
double wxControlPoint::sm_controlPointDragStartWidth = 0.0;
double wxControlPoint::sm_controlPointDragStartHeight = 0.0;
double wxControlPoint::sm_controlPointDragEndWidth = 0.0;
double wxControlPoint::sm_controlPointDragEndHeight = 0.0;
double wxControlPoint::sm_controlPointDragPosX = 0.0;
double wxControlPoint::sm_controlPointDragPosY = 0.0;

IMPLEMENT_DYNAMIC_CLASS(wxControlPoint, wxRectangleShape)

wxControlPoint::wxControlPoint(wxShapeCanvas *theCanvas, wxShape *object, double size, double the_xoffset, double the_yoffset, int the_type): wxRectangleShape(size, size)
{
	m_canvas = theCanvas;
	m_shape = object;
	m_xoffset = the_xoffset;
	m_yoffset = the_yoffset;
	m_type = the_type;
	SetPen(g_oglBlackForegroundPen);
	SetBrush((wxBrush *) wxBLACK_BRUSH);
	m_oldCursor = NULL;
	m_visible = TRUE;
	m_eraseObject = TRUE;
}

wxControlPoint::~wxControlPoint()
{
}

// Don't even attempt to draw any text - waste of time!
void wxControlPoint::OnDrawContents(wxDC &WXUNUSED(dc))
{
}

void wxControlPoint::OnDraw(wxDC &dc)
{
	m_xpos = m_shape->GetX() + m_xoffset;
	m_ypos = m_shape->GetY() + m_yoffset;
	wxRectangleShape::OnDraw(dc);
}

// Implement resizing of canvas object
void wxControlPoint::OnDragLeft(bool draw, double x, double y, int keys, int attachment)
{
	m_shape->GetEventHandler()->OnSizingDragLeft(this, draw, x, y, keys, attachment);
}

void wxControlPoint::OnBeginDragLeft(double x, double y, int keys, int attachment)
{
	m_shape->GetEventHandler()->OnSizingBeginDragLeft(this, x, y, keys, attachment);
}

void wxControlPoint::OnEndDragLeft(double x, double y, int keys, int attachment)
{
	m_shape->GetEventHandler()->OnSizingEndDragLeft(this, x, y, keys, attachment);
}

int wxControlPoint::GetNumberOfAttachments() const
{
	return 1;
}

bool wxControlPoint::GetAttachmentPosition(int WXUNUSED(attachment), double *x, double *y,
        int WXUNUSED(nth), int WXUNUSED(no_arcs), wxLineShape *WXUNUSED(line))
{
	*x = m_xpos;
	*y = m_ypos;
	return TRUE;
}

// Control points ('handles') redirect control to the actual shape, to make it easier
// to override sizing behaviour.
void wxShape::OnSizingDragLeft(wxControlPoint *pt, bool WXUNUSED(draw), double x, double y, int keys, int WXUNUSED(attachment))
{
	double bound_x;
	double bound_y;
	this->GetBoundingBoxMin(&bound_x, &bound_y);

	wxClientDC dc(GetCanvas());
	GetCanvas()->PrepareDC(dc);

	dc.SetLogicalFunction(OGLRBLF);

	wxPen dottedPen(wxColour(0, 0, 0), 1, wxDOT);
	dc.SetPen(dottedPen);
	dc.SetBrush((* wxTRANSPARENT_BRUSH));

	if (this->GetCentreResize())
	{
		// Maintain the same centre point.
		double new_width = (double)(2.0 * fabs(x - this->GetX()));
		double new_height = (double)(2.0 * fabs(y - this->GetY()));

		// Constrain sizing according to what control point you're dragging
		if (pt->m_type == CONTROL_POINT_HORIZONTAL)
		{
			if (GetMaintainAspectRatio())
			{
				new_height = bound_y * (new_width / bound_x);
			}
			else
				new_height = bound_y;
		}
		else if (pt->m_type == CONTROL_POINT_VERTICAL)
		{
			if (GetMaintainAspectRatio())
			{
				new_width = bound_x * (new_height / bound_y);
			}
			else
				new_width = bound_x;
		}
		else if (pt->m_type == CONTROL_POINT_DIAGONAL && (keys & KEY_SHIFT))
			new_height = bound_y * (new_width / bound_x);

		if (this->GetFixedWidth())
			new_width = bound_x;

		if (this->GetFixedHeight())
			new_height = bound_y;

		pt->sm_controlPointDragEndWidth = new_width;
		pt->sm_controlPointDragEndHeight = new_height;

		this->GetEventHandler()->OnDrawOutline(dc, this->GetX(), this->GetY(),
		                                       new_width, new_height);
	}
	else
	{
		// Don't maintain the same centre point!
		double newX1 = wxMin(pt->sm_controlPointDragStartX, x);
		double newY1 = wxMin(pt->sm_controlPointDragStartY, y);
		double newX2 = wxMax(pt->sm_controlPointDragStartX, x);
		double newY2 = wxMax(pt->sm_controlPointDragStartY, y);
		if (pt->m_type == CONTROL_POINT_HORIZONTAL)
		{
			newY1 = pt->sm_controlPointDragStartY;
			newY2 = newY1 + pt->sm_controlPointDragStartHeight;
		}
		else if (pt->m_type == CONTROL_POINT_VERTICAL)
		{
			newX1 = pt->sm_controlPointDragStartX;
			newX2 = newX1 + pt->sm_controlPointDragStartWidth;
		}
		else if (pt->m_type == CONTROL_POINT_DIAGONAL && ((keys & KEY_SHIFT) || GetMaintainAspectRatio()))
		{
			double newH = (double)((newX2 - newX1) * (pt->sm_controlPointDragStartHeight / pt->sm_controlPointDragStartWidth));
			if (GetY() > pt->sm_controlPointDragStartY)
				newY2 = (double)(newY1 + newH);
			else
				newY1 = (double)(newY2 - newH);
		}
		double newWidth = (double)(newX2 - newX1);
		double newHeight = (double)(newY2 - newY1);

		if (pt->m_type == CONTROL_POINT_VERTICAL && GetMaintainAspectRatio())
		{
			newWidth = bound_x * (newHeight / bound_y) ;
		}

		if (pt->m_type == CONTROL_POINT_HORIZONTAL && GetMaintainAspectRatio())
		{
			newHeight = bound_y * (newWidth / bound_x) ;
		}

		pt->sm_controlPointDragPosX = (double)(newX1 + (newWidth / 2.0));
		pt->sm_controlPointDragPosY = (double)(newY1 + (newHeight / 2.0));
		if (this->GetFixedWidth())
			newWidth = bound_x;

		if (this->GetFixedHeight())
			newHeight = bound_y;

		pt->sm_controlPointDragEndWidth = newWidth;
		pt->sm_controlPointDragEndHeight = newHeight;
		this->GetEventHandler()->OnDrawOutline(dc, pt->sm_controlPointDragPosX, pt->sm_controlPointDragPosY, newWidth, newHeight);
	}
}

void wxShape::OnSizingBeginDragLeft(wxControlPoint *pt, double x, double y, int keys, int WXUNUSED(attachment))
{
	m_canvas->CaptureMouse();

	wxClientDC dc(GetCanvas());
	GetCanvas()->PrepareDC(dc);
	/*
	  if (pt->m_eraseObject)
	    this->Erase(dc);
	*/

	dc.SetLogicalFunction(OGLRBLF);

	double bound_x;
	double bound_y;
	this->GetBoundingBoxMin(&bound_x, &bound_y);

	// Choose the 'opposite corner' of the object as the stationary
	// point in case this is non-centring resizing.
	if (pt->GetX() < this->GetX())
		pt->sm_controlPointDragStartX = (double)(this->GetX() + (bound_x / 2.0));
	else
		pt->sm_controlPointDragStartX = (double)(this->GetX() - (bound_x / 2.0));

	if (pt->GetY() < this->GetY())
		pt->sm_controlPointDragStartY = (double)(this->GetY() + (bound_y / 2.0));
	else
		pt->sm_controlPointDragStartY = (double)(this->GetY() - (bound_y / 2.0));

	if (pt->m_type == CONTROL_POINT_HORIZONTAL)
		pt->sm_controlPointDragStartY = (double)(this->GetY() - (bound_y / 2.0));
	else if (pt->m_type == CONTROL_POINT_VERTICAL)
		pt->sm_controlPointDragStartX = (double)(this->GetX() - (bound_x / 2.0));

	// We may require the old width and height.
	pt->sm_controlPointDragStartWidth = bound_x;
	pt->sm_controlPointDragStartHeight = bound_y;

	wxPen dottedPen(wxColour(0, 0, 0), 1, wxDOT);
	dc.SetPen(dottedPen);
	dc.SetBrush((* wxTRANSPARENT_BRUSH));

	if (this->GetCentreResize())
	{
		double new_width = (double)(2.0 * fabs(x - this->GetX()));
		double new_height = (double)(2.0 * fabs(y - this->GetY()));

		// Constrain sizing according to what control point you're dragging
		if (pt->m_type == CONTROL_POINT_HORIZONTAL)
		{
			if (GetMaintainAspectRatio())
			{
				new_height = bound_y * (new_width / bound_x);
			}
			else
				new_height = bound_y;
		}
		else if (pt->m_type == CONTROL_POINT_VERTICAL)
		{
			if (GetMaintainAspectRatio())
			{
				new_width = bound_x * (new_height / bound_y);
			}
			else
				new_width = bound_x;
		}
		else if (pt->m_type == CONTROL_POINT_DIAGONAL && (keys & KEY_SHIFT))
			new_height = bound_y * (new_width / bound_x);

		if (this->GetFixedWidth())
			new_width = bound_x;

		if (this->GetFixedHeight())
			new_height = bound_y;

		pt->sm_controlPointDragEndWidth = new_width;
		pt->sm_controlPointDragEndHeight = new_height;
		this->GetEventHandler()->OnDrawOutline(dc, this->GetX(), this->GetY(),
		                                       new_width, new_height);
	}
	else
	{
		// Don't maintain the same centre point!
		double newX1 = wxMin(pt->sm_controlPointDragStartX, x);
		double newY1 = wxMin(pt->sm_controlPointDragStartY, y);
		double newX2 = wxMax(pt->sm_controlPointDragStartX, x);
		double newY2 = wxMax(pt->sm_controlPointDragStartY, y);
		if (pt->m_type == CONTROL_POINT_HORIZONTAL)
		{
			newY1 = pt->sm_controlPointDragStartY;
			newY2 = newY1 + pt->sm_controlPointDragStartHeight;
		}
		else if (pt->m_type == CONTROL_POINT_VERTICAL)
		{
			newX1 = pt->sm_controlPointDragStartX;
			newX2 = newX1 + pt->sm_controlPointDragStartWidth;
		}
		else if (pt->m_type == CONTROL_POINT_DIAGONAL && ((keys & KEY_SHIFT) || GetMaintainAspectRatio()))
		{
			double newH = (double)((newX2 - newX1) * (pt->sm_controlPointDragStartHeight / pt->sm_controlPointDragStartWidth));
			if (pt->GetY() > pt->sm_controlPointDragStartY)
				newY2 = (double)(newY1 + newH);
			else
				newY1 = (double)(newY2 - newH);
		}
		double newWidth = (double)(newX2 - newX1);
		double newHeight = (double)(newY2 - newY1);

		if (pt->m_type == CONTROL_POINT_VERTICAL && GetMaintainAspectRatio())
		{
			newWidth = bound_x * (newHeight / bound_y) ;
		}

		if (pt->m_type == CONTROL_POINT_HORIZONTAL && GetMaintainAspectRatio())
		{
			newHeight = bound_y * (newWidth / bound_x) ;
		}

		pt->sm_controlPointDragPosX = (double)(newX1 + (newWidth / 2.0));
		pt->sm_controlPointDragPosY = (double)(newY1 + (newHeight / 2.0));
		if (this->GetFixedWidth())
			newWidth = bound_x;

		if (this->GetFixedHeight())
			newHeight = bound_y;

		pt->sm_controlPointDragEndWidth = newWidth;
		pt->sm_controlPointDragEndHeight = newHeight;
		this->GetEventHandler()->OnDrawOutline(dc, pt->sm_controlPointDragPosX, pt->sm_controlPointDragPosY, newWidth, newHeight);
	}
}

void wxShape::OnSizingEndDragLeft(wxControlPoint *pt, double WXUNUSED(x), double WXUNUSED(y), int WXUNUSED(keys), int WXUNUSED(attachment))
{
	m_canvas->ReleaseMouse();
	this->Recompute();
	this->ResetControlPoints();

	this->Erase();
	/*
	  if (!pt->m_eraseObject)
	    this->Show(FALSE);
	*/

	this->SetSize(pt->sm_controlPointDragEndWidth, pt->sm_controlPointDragEndHeight);

	// The next operation could destroy this control point (it does for label objects,
	// via formatting the text), so save all values we're going to use, or
	// we'll be accessing garbage.
	wxShape *theObject = this;
	wxShapeCanvas *theCanvas = m_canvas;
	bool eraseIt = pt->m_eraseObject;

	if (theObject->GetCentreResize())
		theObject->Move(theObject->GetX(), theObject->GetY());
	else
		theObject->Move(pt->sm_controlPointDragPosX, pt->sm_controlPointDragPosY);

	/*
	  if (!eraseIt)
	    theObject->Show(TRUE);
	*/

	double width, height;
	theObject->GetBoundingBoxMax(&width, &height);
	theObject->GetEventHandler()->OnEndSize(width, height);

	theCanvas->Refresh();
}



// Polygon control points

IMPLEMENT_DYNAMIC_CLASS(wxPolygonControlPoint, wxControlPoint)

wxPolygonControlPoint::wxPolygonControlPoint(wxShapeCanvas *theCanvas, wxShape *object, double size,
        wxRealPoint *vertex, double the_xoffset, double the_yoffset):
	wxControlPoint(theCanvas, object, size, the_xoffset, the_yoffset, 0)
{
	m_polygonVertex = vertex;
	m_originalDistance = 0.0;
}

wxPolygonControlPoint::~wxPolygonControlPoint()
{
}

// Calculate what new size would be, at end of resize
void wxPolygonControlPoint::CalculateNewSize(double x, double y)
{
	double bound_x;
	double bound_y;
	GetShape()->GetBoundingBoxMin(&bound_x, &bound_y);

	double dist = (double)sqrt((x - m_shape->GetX()) * (x - m_shape->GetX()) +
	                           (y - m_shape->GetY()) * (y - m_shape->GetY()));

	m_newSize.x = (double)(dist / this->m_originalDistance) * this->m_originalSize.x;
	m_newSize.y = (double)(dist / this->m_originalDistance) * this->m_originalSize.y;
}


// Implement resizing polygon or moving the vertex.
void wxPolygonControlPoint::OnDragLeft(bool draw, double x, double y, int keys, int attachment)
{
	m_shape->GetEventHandler()->OnSizingDragLeft(this, draw, x, y, keys, attachment);
}

void wxPolygonControlPoint::OnBeginDragLeft(double x, double y, int keys, int attachment)
{
	m_shape->GetEventHandler()->OnSizingBeginDragLeft(this, x, y, keys, attachment);
}

void wxPolygonControlPoint::OnEndDragLeft(double x, double y, int keys, int attachment)
{
	m_shape->GetEventHandler()->OnSizingEndDragLeft(this, x, y, keys, attachment);
}

// Control points ('handles') redirect control to the actual shape, to make it easier
// to override sizing behaviour.
void wxPolygonShape::OnSizingDragLeft(wxControlPoint *pt, bool WXUNUSED(draw), double x, double y, int WXUNUSED(keys), int WXUNUSED(attachment))
{
	wxPolygonControlPoint *ppt = (wxPolygonControlPoint *) pt;

	wxClientDC dc(GetCanvas());
	GetCanvas()->PrepareDC(dc);

	dc.SetLogicalFunction(OGLRBLF);

	wxPen dottedPen(wxColour(0, 0, 0), 1, wxDOT);
	dc.SetPen(dottedPen);
	dc.SetBrush((* wxTRANSPARENT_BRUSH));

#if 0 // keys & KEY_CTRL)
	{
		// TODO: mend this code. Currently we rely on altering the
		// actual points, but we should assume we're not, as per
		// the normal sizing case.
		m_canvas->Snap(&x, &y);

		// Move point
		ppt->m_polygonVertex->x = x - this->GetX();
		ppt->m_polygonVertex->y = y - this->GetY();
		ppt->SetX(x);
		ppt->SetY(y);
		((wxPolygonShape *)this)->CalculateBoundingBox();
		((wxPolygonShape *)this)->CalculatePolygonCentre();
	}
#else
	{
		ppt->CalculateNewSize(x, y);
	}
#endif

	this->GetEventHandler()->OnDrawOutline(dc, this->GetX(), this->GetY(),
	                                       ppt->GetNewSize().x, ppt->GetNewSize().y);
}

void wxPolygonShape::OnSizingBeginDragLeft(wxControlPoint *pt, double x, double y, int WXUNUSED(keys), int WXUNUSED(attachment))
{
	wxPolygonControlPoint *ppt = (wxPolygonControlPoint *) pt;

	wxClientDC dc(GetCanvas());
	GetCanvas()->PrepareDC(dc);

	this->Erase();

	dc.SetLogicalFunction(OGLRBLF);

	double bound_x;
	double bound_y;
	this->GetBoundingBoxMin(&bound_x, &bound_y);

	double dist = (double)sqrt((x - this->GetX()) * (x - this->GetX()) +
	                           (y - this->GetY()) * (y - this->GetY()));
	ppt->m_originalDistance = dist;
	ppt->m_originalSize.x = bound_x;
	ppt->m_originalSize.y = bound_y;

	if (ppt->m_originalDistance == 0.0) ppt->m_originalDistance = (double) 0.0001;

	wxPen dottedPen(wxColour(0, 0, 0), 1, wxDOT);
	dc.SetPen(dottedPen);
	dc.SetBrush((* wxTRANSPARENT_BRUSH));

#if 0 // keys & KEY_CTRL)
	{
		// TODO: mend this code. Currently we rely on altering the
		// actual points, but we should assume we're not, as per
		// the normal sizing case.
		m_canvas->Snap(&x, &y);

		// Move point
		ppt->m_polygonVertex->x = x - this->GetX();
		ppt->m_polygonVertex->y = y - this->GetY();
		ppt->SetX(x);
		ppt->SetY(y);
		((wxPolygonShape *)this)->CalculateBoundingBox();
		((wxPolygonShape *)this)->CalculatePolygonCentre();
	}
#else
	{
		ppt->CalculateNewSize(x, y);
	}
#endif

	this->GetEventHandler()->OnDrawOutline(dc, this->GetX(), this->GetY(),
	                                       ppt->GetNewSize().x, ppt->GetNewSize().y);

	m_canvas->CaptureMouse();
}

void wxPolygonShape::OnSizingEndDragLeft(wxControlPoint *pt, double WXUNUSED(x), double WXUNUSED(y), int keys, int WXUNUSED(attachment))
{
	wxPolygonControlPoint *ppt = (wxPolygonControlPoint *) pt;

	wxClientDC dc(GetCanvas());
	GetCanvas()->PrepareDC(dc);

	m_canvas->ReleaseMouse();
	dc.SetLogicalFunction(wxCOPY);

	// If we're changing shape, must reset the original points
	if (keys & KEY_CTRL)
	{
		((wxPolygonShape *)this)->CalculateBoundingBox();
		((wxPolygonShape *)this)->UpdateOriginalPoints();
	}
	else
	{
		SetSize(ppt->GetNewSize().x, ppt->GetNewSize().y);
	}

	((wxPolygonShape *)this)->CalculateBoundingBox();
	((wxPolygonShape *)this)->CalculatePolygonCentre();

	this->Recompute();
	this->ResetControlPoints();
	this->Move(this->GetX(), this->GetY());
	m_canvas->Refresh();
}

/*
 * Object region
 *
 */
IMPLEMENT_DYNAMIC_CLASS(wxShapeRegion, wxObject)

wxShapeRegion::wxShapeRegion()
{
	m_regionText = wxEmptyString;
	m_font = g_oglNormalFont;
	m_minHeight = 5.0;
	m_minWidth = 5.0;
	m_width = 0.0;
	m_height = 0.0;
	m_x = 0.0;
	m_y = 0.0;

	m_regionProportionX = -1.0;
	m_regionProportionY = -1.0;
	m_formatMode = FORMAT_CENTRE_HORIZ | FORMAT_CENTRE_VERT;
	m_regionName = wxEmptyString;
	m_textColour = wxT("BLACK");
	m_penColour = wxT("BLACK");
	m_penStyle = wxSOLID;
	m_actualPenObject = NULL;
}

wxShapeRegion::wxShapeRegion(wxShapeRegion &region)
{
	m_regionText = region.m_regionText;
	m_regionName = region.m_regionName;
	m_textColour = region.m_textColour;

	m_font = region.m_font;
	m_minHeight = region.m_minHeight;
	m_minWidth = region.m_minWidth;
	m_width = region.m_width;
	m_height = region.m_height;
	m_x = region.m_x;
	m_y = region.m_y;

	m_regionProportionX = region.m_regionProportionX;
	m_regionProportionY = region.m_regionProportionY;
	m_formatMode = region.m_formatMode;
	m_actualColourObject = region.GetActualColourObject();
	m_actualPenObject = NULL;
	m_penStyle = region.m_penStyle;
	m_penColour = region.m_penColour;

	ClearText();
	wxNode *node = region.m_formattedText.GetFirst();
	while (node)
	{
		wxShapeTextLine *line = (wxShapeTextLine *)node->GetData();
		wxShapeTextLine *new_line =
		    new wxShapeTextLine(line->GetX(), line->GetY(), line->GetText());
		m_formattedText.Append(new_line);
		node = node->GetNext();
	}
}

wxShapeRegion::~wxShapeRegion()
{
	ClearText();
}

void wxShapeRegion::ClearText()
{
	wxNode *node = m_formattedText.GetFirst();
	while (node)
	{
		wxShapeTextLine *line = (wxShapeTextLine *)node->GetData();
		wxNode *next = node->GetNext();
		delete line;
		delete node;
		node = next;
	}
}

void wxShapeRegion::SetFont(wxFont *f)
{
	m_font = f;
}

void wxShapeRegion::SetMinSize(double w, double h)
{
	m_minWidth = w;
	m_minHeight = h;
}

void wxShapeRegion::SetSize(double w, double h)
{
	m_width = w;
	m_height = h;
}

void wxShapeRegion::SetPosition(double xp, double yp)
{
	m_x = xp;
	m_y = yp;
}

void wxShapeRegion::SetProportions(double xp, double yp)
{
	m_regionProportionX = xp;
	m_regionProportionY = yp;
}

void wxShapeRegion::SetFormatMode(int mode)
{
	m_formatMode = mode;
}

void wxShapeRegion::SetColour(const wxString &col)
{
	m_textColour = col;
	m_actualColourObject = wxColour();
}

void wxShapeRegion::SetColour(const wxColour &col)
{
	m_actualColourObject = col;
}

const wxColour &wxShapeRegion::GetActualColourObject()
{
	if (!m_actualColourObject.IsOk())
		m_actualColourObject = wxTheColourDatabase->Find(GetColour());
	if (!m_actualColourObject.IsOk())
		m_actualColourObject = * wxBLACK;
	return m_actualColourObject;
}

void wxShapeRegion::SetPenColour(const wxString &col)
{
	m_penColour = col;
	m_actualPenObject = NULL;
}

// Returns NULL if the pen is invisible
// (different to pen being transparent; indicates that
// region boundary should not be drawn.)
wxPen *wxShapeRegion::GetActualPen()
{
	if (m_actualPenObject)
		return m_actualPenObject;

	if (!m_penColour) return NULL;
	if (m_penColour == wxT("Invisible"))
		return NULL;
	m_actualPenObject = wxThePenList->FindOrCreatePen(m_penColour, 1, m_penStyle);
	return m_actualPenObject;
}


