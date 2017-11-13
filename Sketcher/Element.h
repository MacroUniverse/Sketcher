#pragma once
#include "afx.h"
#include <memory>

class CElement :  public CObject
{
protected:
	CPoint m_StartPoint;
	int m_PenWidth;
	COLORREF m_Color;
	CRect m_EnclosingRect;

public:
	virtual ~CElement();
	virtual void Draw(CDC* pDC, std::shared_ptr<CElement> pElement=nullptr) {}
	virtual void Move(const CSize& aSize) {}

	// Get the element enclosing rectangle
	const CRect& GetEnclosingRect() const { return m_EnclosingRect; }

protected:
	CElement();
	CElement(const CPoint& start, COLORREF color, int penWidth = 1);

	void CreatePen(CPen& aPen, std::shared_ptr<CElement> pElement)
	{
		if (!aPen.CreatePen(PS_SOLID, m_PenWidth, 
			this == pElement.get()? RGB(134,95,197) : m_Color))
		{
			//Pen creation failed
			AfxMessageBox(_T("Pen creation failed."), MB_OK);
			AfxAbort();
		}
		return;
	}
};

