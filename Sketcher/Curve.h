#pragma once
#include "Element.h"
#include <vector>

class CCurve :  public CElement
{
public:
	virtual ~CCurve();
	virtual void Draw(CDC* pDC, std::shared_ptr<CElement> pElement = nullptr) override;

	CCurve(const CPoint& first, const CPoint& second, COLORREF color, int penWidth);
	void AddSegment(const CPoint& point);
	virtual void Move(const CSize& aSize) override;

protected:
	std::vector<CPoint> m_Points;
	CCurve();
};

