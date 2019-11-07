//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Portions Copyright (C) 1998 - 2011, Julian Smart
// Portions Copyright (C) 2011 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// divided.h - wxDividedShape
//
//////////////////////////////////////////////////////////////////////////

#ifndef _OGL_DIVIDED_H_
#define _OGL_DIVIDED_H_

/*
 * Definition of a region
 *
 */

/*
 * Box divided into horizontal regions
 *
 */

extern wxFont *g_oglNormalFont;
class wxDividedShape: public wxRectangleShape
{
	DECLARE_DYNAMIC_CLASS(wxDividedShape)

public:
	wxDividedShape(double w = 0.0, double h = 0.0);
	~wxDividedShape();

	void OnDraw(wxDC &dc);
	void OnDrawContents(wxDC &dc);

	void SetSize(double w, double h, bool recursive = TRUE);

	void MakeControlPoints();
	void ResetControlPoints();

	void MakeMandatoryControlPoints();
	void ResetMandatoryControlPoints();

#if wxUSE_PROLOGIO
	void WriteAttributes(wxExpr *clause);
	void ReadAttributes(wxExpr *clause);
#endif

	void Copy(wxShape &copy);

	// Set all region sizes according to proportions and
	// this object total size
	void SetRegionSizes();

	// Edit region colours/styles
	void EditRegions();

	// Attachment points correspond to regions in the divided box
	bool GetAttachmentPosition(int attachment, double *x, double *y,
	                           int nth = 0, int no_arcs = 1, wxLineShape *line = NULL);
	bool AttachmentIsValid(int attachment) const;
	int GetNumberOfAttachments() const;

	// Invoke editor on CTRL-right click
	void OnRightClick(double x, double y, int keys = 0, int attachment = 0);
};

#endif
// _OGL_DIVIDED_H_

