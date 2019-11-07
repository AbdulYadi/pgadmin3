//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Portions Copyright (C) 1998 - 2011, Julian Smart
// Portions Copyright (C) 2011 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// canvas.h - wxShapeCanvas
//
//////////////////////////////////////////////////////////////////////////

#ifndef _OGL_CANVAS_H_
#define _OGL_CANVAS_H_

// Drag states
#define NoDragging             0
#define StartDraggingLeft      1
#define ContinueDraggingLeft   2
#define StartDraggingRight     3
#define ContinueDraggingRight  4

#ifdef __WXMSW__
#define OGL_USE_BUFFERED_PAINT 1
#else
#define OGL_USE_BUFFERED_PAINT 0
#endif


extern const wxChar *wxShapeCanvasNameStr;

// When drag_count reaches 0, process drag message

class wxDiagram;

class wxShapeCanvas: public wxScrolledWindow
{
	DECLARE_DYNAMIC_CLASS(wxShapeCanvas)
public:
	wxShapeCanvas(wxWindow *parent = NULL, wxWindowID id = -1,
	              const wxPoint &pos = wxDefaultPosition,
	              const wxSize &size = wxDefaultSize,
	              long style = wxBORDER | wxRETAINED,
	              const wxString &name = wxShapeCanvasNameStr);
	~wxShapeCanvas();

	inline void SetDiagram(wxDiagram *diag)
	{
		m_shapeDiagram = diag;
	}
	inline wxDiagram *GetDiagram() const
	{
		return m_shapeDiagram;
	}

	virtual void OnLeftClick(double x, double y, int keys = 0);
	virtual void OnRightClick(double x, double y, int keys = 0);

	virtual void OnDragLeft(bool draw, double x, double y, int keys = 0); // Erase if draw false
	virtual void OnBeginDragLeft(double x, double y, int keys = 0);
	virtual void OnEndDragLeft(double x, double y, int keys = 0);

	virtual void OnDragRight(bool draw, double x, double y, int keys = 0); // Erase if draw false
	virtual void OnBeginDragRight(double x, double y, int keys = 0);
	virtual void OnEndDragRight(double x, double y, int keys = 0);

	// Find object for mouse click, of given wxClassInfo (NULL for any type).
	// If notImage is non-NULL, don't find an object that is equal to or a descendant of notImage
	virtual wxShape *FindShape(double x, double y, int *attachment, wxClassInfo *info = NULL, wxShape *notImage = NULL);
	wxShape *FindFirstSensitiveShape(double x, double y, int *new_attachment, int op);
	wxShape *FindFirstSensitiveShape1(wxShape *image, int op);

	// Redirect to wxDiagram object
	virtual void AddShape(wxShape *object, wxShape *addAfter = NULL);
	virtual void InsertShape(wxShape *object);
	virtual void RemoveShape(wxShape *object);
	virtual bool GetQuickEditMode();
	virtual void Redraw(wxDC &dc);
	void Snap(double *x, double *y);

	// Returns the current scroll position
	wxPoint GetCurrentPixelScrollPosition();

	void ResetDragState()
	{
		m_dragState = NoDragging;
	}

#if OGL_USE_BUFFERED_PAINT
	/// Recreate buffer bitmap if necessary
	bool RecreateBuffer(const wxSize &size = wxDefaultSize);
#endif

	virtual void DrawBackground(wxDC &dc, bool transformed = FALSE);

	// Events
	void OnPaint(wxPaintEvent &event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnEraseBackground(wxEraseEvent &event);

protected:
	wxDiagram        *m_shapeDiagram;
	int               m_dragState;
	double             m_oldDragX, m_oldDragY;     // Previous drag coordinates
	double             m_firstDragX, m_firstDragY; // INITIAL drag coordinates
	bool              m_checkTolerance;           // Whether to check drag tolerance
	wxShape          *m_draggedShape;
	int               m_draggedAttachment;

#if OGL_USE_BUFFERED_PAINT
	/// Buffer bitmap
	wxBitmap                m_bufferBitmap;
#endif

	DECLARE_EVENT_TABLE()
};

#endif
// _OGL_CANVAS_H_
