
// SketcherView.cpp : implementation of the CSketcherView class
//

#include "stdafx.h"
#include "Resource.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Sketcher.h"
#endif

#include "SketcherDoc.h"
#include "SketcherView.h"
#include "Line.h"
#include "Rectangle.h"
#include "Circle.h"
#include "Curve.h"
#include <memory>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CSketcherView, CScrollView)

BEGIN_MESSAGE_MAP(CSketcherView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_ELEMENT_MOVE, &CSketcherView::OnElementMove)
	ON_COMMAND(ID_ELEMENT_DELETE, &CSketcherView::OnElementDelete)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_ELEMENT_SENDTOBACK, &CSketcherView::OnElementSendtoback)
END_MESSAGE_MAP()

// CSketcherView construction/destruction

CSketcherView::CSketcherView(){}

CSketcherView::~CSketcherView(){}

BOOL CSketcherView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// CSketcherView printing

BOOL CSketcherView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CSketcherView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CSketcherView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

// CSketcherView diagnostics
#ifdef _DEBUG
void CSketcherView::AssertValid() const
{
	CView::AssertValid();
}

void CSketcherView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSketcherDoc* CSketcherView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSketcherDoc)));
	return (CSketcherDoc*)m_pDocument;
}
#endif //_DEBUG


// CSketcherView drawing
void CSketcherView::OnDraw(CDC* pDC)
{
	CSketcherDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	//Draw the sketch
	//for (auto iter = pDoc->begin(); iter != pDoc->end(); ++iter)
	for (const auto& pElement : *pDoc)
	{
		if (pDC->RectVisible(pElement->GetEnclosingRect()))
			pElement->Draw(pDC, m_pSelected);
	}
}

//Create an element of the current type
std::shared_ptr<CElement> CSketcherView::CreateElement() const
{
	CSketcherDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc); //verify the pointer is good

	COLORREF color{ static_cast<COLORREF>(pDoc->GetElementColor()) };

	int penWidth{ pDoc->GetPenWidth() };

	switch (pDoc->GetElementType())
	{
	case ElementType::RECTANGLE:
		return std::make_shared<CRectangle>(m_FirstPoint, m_SecondPoint, color, penWidth);
	case ElementType::CIRCLE:
		return std::make_shared<CCircle>(m_FirstPoint, m_SecondPoint, color, penWidth);
	case ElementType::CURVE:
		return std::make_shared<CCurve>(m_FirstPoint, m_SecondPoint, color, penWidth);
	case ElementType::LINE:
		return std::make_shared<CLine>(m_FirstPoint, m_SecondPoint, color, penWidth);
	default:
		AfxMessageBox(_T("Bad Element code"), MB_OK);
		AfxAbort();
		return nullptr;
	}
}


// CSketcherView message handlers

void CSketcherView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (this == GetCapture())
		ReleaseCapture();
	if (m_pTempElement)
	{
		GetDocument()->AddElement(m_pTempElement);
		InvalidateRect(&m_pTempElement->GetEnclosingRect());
		m_pTempElement.reset();
	}
}

void CSketcherView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CClientDC aDC{ this };
	OnPrepareDC(&aDC);
	aDC.DPtoLP(&point);
	if (m_MoveMode)
	{
		m_MoveMode = false;
		ReleaseCapture();
		auto pElement(m_pSelected);
		m_pSelected.reset();
		GetDocument()->UpdateAllViews(nullptr, 0, pElement.get());
	}
	else
	{
		m_FirstPoint = point;
		SetCapture();
	}
}

void CSketcherView::OnMouseMove(UINT nFlags, CPoint point)
{
	CClientDC aDC{ this };
	OnPrepareDC(&aDC);
	aDC.DPtoLP(&point);

	if (m_MoveMode)
	{
		MoveElement(aDC, point);
	}
	else if ((nFlags & MK_LBUTTON) && (this == GetCapture()))
	{
		m_SecondPoint = point;
		if (m_pTempElement)
		{
			if (ElementType::CURVE == GetDocument()->GetElementType())
			{
				std::dynamic_pointer_cast<CCurve>(m_pTempElement)->AddSegment(m_SecondPoint);
				m_pTempElement->Draw(&aDC);
				return;
			}
			else //not a curve
			{
				aDC.SetROP2(R2_NOTXORPEN);
				m_pTempElement->Draw(&aDC);
			}
		}

		m_pTempElement = CreateElement();
		m_pTempElement->Draw(&aDC);
	}
	else
	{
		auto pOldSelected = m_pSelected;
		m_pSelected = GetDocument()->FindElement(point);
		if (m_pSelected != pOldSelected)
		{
			if (m_pSelected)
				GetDocument()->UpdateAllViews(nullptr, 0, m_pSelected.get());
			if (pOldSelected)
				GetDocument()->UpdateAllViews(nullptr, 0, pOldSelected.get());
		}
	}
}

void CSketcherView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_MoveMode)
	{
		m_MoveMode = false;
	}
	else
	{
		CScrollView::OnRButtonUp(nFlags, point);
	}
}

void CSketcherView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_MoveMode)
	{
		CClientDC aDC{ this };
		OnPrepareDC(&aDC);
		MoveElement(aDC, m_FirstPos);
		m_pSelected.reset();
		GetDocument()->UpdateAllViews(nullptr);
		ReleaseCapture();
	}
}

void CSketcherView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize DocSize{ 20000, 20000 };
	SetScrollSizes(MM_LOENGLISH, DocSize, CSize{ 500,500 }, CSize{ 20,20 });
}

void CSketcherView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (pHint)
	{
		CClientDC aDC{ this };
		OnPrepareDC(&aDC);
		CRect aRect{ dynamic_cast<CElement*>(pHint)->GetEnclosingRect() };
		aDC.LPtoDP(aRect);
		InvalidateRect(aRect);
	}
	else
	{
		InvalidateRect(nullptr);
	}
}

void CSketcherView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT_MENU);
	CMenu* pContext{};
	if (m_pSelected)
	{
		pContext = menu.GetSubMenu(0);
	}
	else
	{
		pContext = menu.GetSubMenu(1);

		ElementColor color{ GetDocument()->GetElementColor() };
		menu.CheckMenuItem(ID_COLOR_BLACK,
			(ElementColor::BLACK == color ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
		menu.CheckMenuItem(ID_COLOR_RED,
			(ElementColor::RED == color ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
		menu.CheckMenuItem(ID_COLOR_GREEN,
			(ElementColor::GREEN == color ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
		menu.CheckMenuItem(ID_COLOR_BLUE,
			(ElementColor::BLUE == color ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);

		ElementType type{ GetDocument()->GetElementType() };
		menu.CheckMenuItem(ID_ELEMENT_LINE,
			(ElementType::LINE == type ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
		menu.CheckMenuItem(ID_ELEMENT_RECTANGLE,
			(ElementType::RECTANGLE == type ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
		menu.CheckMenuItem(ID_ELEMENT_CIRCLE,
			(ElementType::CIRCLE == type ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
		menu.CheckMenuItem(ID_ELEMENT_CURVE,
			(ElementType::CURVE == type ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
	}

	ASSERT(pContext != nullptr);

	pContext->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

void CSketcherView::OnElementMove()
{
	CClientDC aDC{ this };
	OnPrepareDC(&aDC);
	GetCursorPos(&m_CursorPos);
	ScreenToClient(&m_CursorPos);
	aDC.DPtoLP(&m_CursorPos);
	m_FirstPos = m_CursorPos;
	m_MoveMode = true;
	SetCapture();
}

void CSketcherView::OnElementDelete()
{
	if (m_pSelected)
	{
		GetDocument()->DeleteElement(m_pSelected);
		m_pSelected.reset();
	}
}

void CSketcherView::MoveElement(CClientDC& aDC, const CPoint& point)
{
	CSize distance{ point - m_CursorPos };
	m_CursorPos = point;

	//if (m_pSelected)
	//{
	CSketcherDoc* pDoc{ GetDocument() };
	pDoc->UpdateAllViews(this, 0L, m_pSelected.get());
	aDC.SetROP2(R2_NOTXORPEN);
	m_pSelected->Draw(&aDC, m_pSelected);
	m_pSelected->Move(distance);
	m_pSelected->Draw(&aDC, m_pSelected);
	pDoc->UpdateAllViews(this, 0L, m_pSelected.get());
	//}
}


void CSketcherView::OnElementSendtoback()
{
	GetDocument()->SendToBack(m_pSelected);
}
