//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Portions Copyright (C) 1998 - 2011, Julian Smart
// Portions Copyright (C) 2011 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// misc.h - Miscellaneous utilities for OGL
//
//////////////////////////////////////////////////////////////////////////

#ifndef _OGL_MISC_H_
#define _OGL_MISC_H_

// List to use when copying objects; may need to associate elements of new objects
// with elements of old objects, e.g. when copying constraint.s
extern wxList oglObjectCopyMapping;

/*
 * TEXT FORMATTING FUNCTIONS
 *
 */

// Centres the given list of wxShapeTextLine strings in the given box
// (changing the positions in situ). Doesn't actually draw into the DC.
void oglCentreText(wxDC &dc, wxList *text, double m_xpos, double m_ypos,
                   double width, double height,
                   int formatMode = FORMAT_CENTRE_HORIZ | FORMAT_CENTRE_VERT);

// Given a string, returns a list of strings that fit within the given
// width of box. Height is ignored.
wxArrayString *oglFormatText(wxDC &dc, const wxString &text, double width, double height, int formatMode = 0);

// Centres the list of wxShapeTextLine strings, doesn't clip.
// Doesn't actually draw into the DC.
void oglCentreTextNoClipping(wxDC &dc, wxList *text_list,
                             double m_xpos, double m_ypos, double width, double height);

// Gets the maximum width and height of the given list of wxShapeTextLines.
void oglGetCentredTextExtent(wxDC &dc, wxList *text_list,
                             double m_xpos, double m_ypos, double width, double height,
                             double *actual_width, double *actual_height);

// Actually draw the preformatted list of wxShapeTextLines.
void oglDrawFormattedText(wxDC &context, wxList *text_list,
                          double m_xpos, double m_ypos, double width, double height,
                          int formatMode = FORMAT_CENTRE_HORIZ | FORMAT_CENTRE_VERT);

// Give it a list of points, finds the centre.
void oglFindPolylineCentroid(wxList *points, double *x, double *y);

void oglCheckLineIntersection(double x1, double y1, double x2, double y2,
                              double x3, double y3, double x4, double y4,
                              double *ratio1, double *ratio2);

void oglFindEndForPolyline(double n, double xvec[], double yvec[],
                           double x1, double y1, double x2, double y2, double *x3, double *y3);


void oglFindEndForBox(double width, double height,
                      double x1, double y1,         // Centre of box (possibly)
                      double x2, double y2,         // other end of line
                      double *x3, double *y3);      // End on box edge

void oglFindEndForCircle(double radius,
                         double x1, double y1,  // Centre of circle
                         double x2, double y2,  // Other end of line
                         double *x3, double *y3);

void oglGetArrowPoints(double x1, double y1, double x2, double y2,
                       double length, double width,
                       double *tip_x, double *tip_y,
                       double *side1_x, double *side1_y,
                       double *side2_x, double *side2_y);

/*
 * Given an ellipse and endpoints of a line, returns the point at which
 * the line touches the ellipse in values x4, y4.
 * This function assumes that the centre of the ellipse is at x1, y1, and the
 * ellipse has a width of a1 and a height of b1. It also assumes you are
 * wanting to draw an arc FROM point x2, y2 TOWARDS point x3, y3.
 * This function calculates the x,y coordinates of the intersection point of
 * the arc with the ellipse.
 * Author: Ian Harrison
 */

void oglDrawArcToEllipse(double x1, double y1, double a1, double b1, double x2, double y2, double x3, double y3,
                         double *x4, double *y4);

bool oglRoughlyEqual(double val1, double val2, double tol = 0.00001);

extern wxFont          *g_oglNormalFont;
extern wxPen           *g_oglBlackPen;
extern wxPen           *g_oglWhiteBackgroundPen;
extern wxPen           *g_oglTransparentPen;
extern wxBrush         *g_oglWhiteBackgroundBrush;
extern wxPen           *g_oglBlackForegroundPen;

extern wxFont          *oglMatchFont(int point_size);

extern wxString         oglColourToHex(const wxColour &colour);
extern wxColour         oglHexToColour(const wxString &hex);
extern void             oglDecToHex(unsigned int dec, char *buf);
extern unsigned int     oglHexToDec(char *buf);


#endif
// _OGL_MISC_H_
