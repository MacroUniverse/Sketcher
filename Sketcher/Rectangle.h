#pragma once
#include "Element.h"

class CRectangle :  public CElement
{
public:
	virtual ~CRectangle();
	virtual void Draw(CDC* pDC, std::shared_ptr<CElement> pElement = nullptr) override;
	CRectangle(const CPoint& start, const CPoint& end, COLORREF color, int penWidth);
	virtual void Move(const CSize& aSize) override;
protected:
	CPoint m_BottomRight;
	CRectangle();
};

