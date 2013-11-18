// dibview.cpp : implementation of the CDibView class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "diblook.h"

#include "dibdoc.h"
#include "dibview.h"
#include "dibapi.h"
#include "mainfrm.h"
#include <algorithm>
#include <vector>

#include "HRTimer.h"
#include <Math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define BEGIN_PROCESSING() INCEPUT_PRELUCRARI()
#define PI 3.1416

#define END_PROCESSING(Title) SFARSIT_PRELUCRARI(Title)

#define INCEPUT_PRELUCRARI() \
	CDibDoc* pDocSrc=GetDocument();										\
	CDocTemplate* pDocTemplate=pDocSrc->GetDocTemplate();				\
	CDibDoc* pDocDest=(CDibDoc*) pDocTemplate->CreateNewDocument();		\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	HDIB hBmpDest = (HDIB)::CopyHandle((HGLOBAL)hBmpSrc);				\
	if ( hBmpDest==0 ) {												\
		pDocTemplate->RemoveDocument(pDocDest);							\
		return;															\
	}																	\
	BYTE* lpD = (BYTE*)::GlobalLock((HGLOBAL)hBmpDest);					\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpD)->bmiHeader)); \
	RGBQUAD *bmiColorsDst = ((LPBITMAPINFO)lpD)->bmiColors;	\
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpDst = (BYTE*)::FindDIBBits((LPSTR)lpD);	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\
	HRTimer my_timer;	\
	my_timer.StartTimer();	\

#define BEGIN_SOURCE_PROCESSING \
	CDibDoc* pDocSrc=GetDocument();										\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpS)->bmiHeader)); \
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\
	


#define END_SOURCE_PROCESSING	\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
    EndWaitCursor();												\
/////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------
#define SFARSIT_PRELUCRARI(Titlu)	\
	double elapsed_time_ms = my_timer.StopTimer();	\
	CString Title;	\
	Title.Format("%s - Proc. time = %.2f ms", Titlu, elapsed_time_ms);	\
	::GlobalUnlock((HGLOBAL)hBmpDest);								\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
    EndWaitCursor();												\
	pDocDest->SetHDIB(hBmpDest);									\
	pDocDest->InitDIBData();										\
	pDocDest->SetTitle((LPCSTR)Title);									\
	CFrameWnd* pFrame=pDocTemplate->CreateNewFrame(pDocDest,NULL);	\
	pDocTemplate->InitialUpdateFrame(pFrame,pDocDest);	\

/////////////////////////////////////////////////////////////////////////////
// CDibView

IMPLEMENT_DYNCREATE(CDibView, CScrollView)

BEGIN_MESSAGE_MAP(CDibView, CScrollView)
	//{{AFX_MSG_MAP(CDibView)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_MESSAGE(WM_DOREALIZE, OnDoRealize)
	ON_COMMAND(ID_PROCESSING_PARCURGERESIMPLA, OnProcessingParcurgereSimpla)
	//}}AFX_MSG_MAP

	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_COMMAND(ID_PROCESSING_L1, &CDibView::OnRANSAC_Line)
	ON_COMMAND(ID_PROCESSING_L3, &CDibView::OnProcessingL3)
	ON_COMMAND(ID_PROCESSING_L4, &CDibView::OnProcessingL4)
	ON_COMMAND(ID_PROCESSING_L5, &CDibView::OnProcessingL5)
	ON_COMMAND(ID_PROCESSING_L6, &CDibView::OnProcessingL6)
	ON_COMMAND(ID_FINALPROJECT_LDA, &CDibView::OnFinalprojectLda)
	ON_COMMAND(ID_PROCESSING_L7, &CDibView::OnProcessingL7)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDibView construction/destruction

CDibView::CDibView()
{
}

CDibView::~CDibView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDibView drawing

void CDibView::OnDraw(CDC* pDC)
{
	CDibDoc* pDoc = GetDocument();

	HDIB hDIB = pDoc->GetHDIB();
	if (hDIB != NULL)
	{
		LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);
		int cxDIB = (int) ::DIBWidth(lpDIB);         // Size of DIB - x
		int cyDIB = (int) ::DIBHeight(lpDIB);        // Size of DIB - y
		::GlobalUnlock((HGLOBAL) hDIB);
		CRect rcDIB;
		rcDIB.top = rcDIB.left = 0;
		rcDIB.right = cxDIB;
		rcDIB.bottom = cyDIB;
		CRect rcDest;
		if (pDC->IsPrinting())   // printer DC
		{
			// get size of printer page (in pixels)
			int cxPage = pDC->GetDeviceCaps(HORZRES);
			int cyPage = pDC->GetDeviceCaps(VERTRES);
			// get printer pixels per inch
			int cxInch = pDC->GetDeviceCaps(LOGPIXELSX);
			int cyInch = pDC->GetDeviceCaps(LOGPIXELSY);

			//
			// Best Fit case -- create a rectangle which preserves
			// the DIB's aspect ratio, and fills the page horizontally.
			//
			// The formula in the "->bottom" field below calculates the Y
			// position of the printed bitmap, based on the size of the
			// bitmap, the width of the page, and the relative size of
			// a printed pixel (cyInch / cxInch).
			//
			rcDest.top = rcDest.left = 0;
			rcDest.bottom = (int)(((double)cyDIB * cxPage * cyInch)
					/ ((double)cxDIB * cxInch));
			rcDest.right = cxPage;
		}
		else   // not printer DC
		{
			rcDest = rcDIB;
		}
		::PaintDIB(pDC->m_hDC, &rcDest, pDoc->GetHDIB(),
			&rcDIB, pDoc->GetDocPalette());
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDibView printing

BOOL CDibView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CDibView commands


LRESULT CDibView::OnDoRealize(WPARAM wParam, LPARAM)
{
	ASSERT(wParam != NULL);
	CDibDoc* pDoc = GetDocument();
	if (pDoc->GetHDIB() == NULL)
		return 0L;  // must be a new document

	CPalette* pPal = pDoc->GetDocPalette();
	if (pPal != NULL)
	{
		CMainFrame* pAppFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
		ASSERT_KINDOF(CMainFrame, pAppFrame);

		CClientDC appDC(pAppFrame);
		// All views but one should be a background palette.
		// wParam contains a handle to the active view, so the SelectPalette
		// bForceBackground flag is FALSE only if wParam == m_hWnd (this view)
		CPalette* oldPalette = appDC.SelectPalette(pPal, ((HWND)wParam) != m_hWnd);

		if (oldPalette != NULL)
		{
			UINT nColorsChanged = appDC.RealizePalette();
			if (nColorsChanged > 0)
				pDoc->UpdateAllViews(NULL);
			appDC.SelectPalette(oldPalette, TRUE);
		}
		else
		{
			TRACE0("\tSelectPalette failed in CDibView::OnPaletteChanged\n");
		}
	}

	return 0L;
}

void CDibView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	ASSERT(GetDocument() != NULL);

	SetScrollSizes(MM_TEXT, GetDocument()->GetDocSize());
}


void CDibView::OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView)
{
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	if (bActivate)
	{
		ASSERT(pActivateView == this);
		OnDoRealize((WPARAM)m_hWnd, 0);   // same as SendMessage(WM_DOREALIZE);
	}
}

void CDibView::OnEditCopy()
{
	CDibDoc* pDoc = GetDocument();
	// Clean clipboard of contents, and copy the DIB.

	if (OpenClipboard())
	{
		BeginWaitCursor();
		EmptyClipboard();
		SetClipboardData (CF_DIB, CopyHandle((HANDLE) pDoc->GetHDIB()) );
		CloseClipboard();
		EndWaitCursor();
	}
}



void CDibView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetHDIB() != NULL);
}


void CDibView::OnEditPaste()
{
	HDIB hNewDIB = NULL;

	if (OpenClipboard())
	{
		BeginWaitCursor();

		hNewDIB = (HDIB) CopyHandle(::GetClipboardData(CF_DIB));

		CloseClipboard();

		if (hNewDIB != NULL)
		{
			CDibDoc* pDoc = GetDocument();
			pDoc->ReplaceHDIB(hNewDIB); // and free the old DIB
			pDoc->InitDIBData();    // set up new size & palette
			pDoc->SetModifiedFlag(TRUE);

			SetScrollSizes(MM_TEXT, pDoc->GetDocSize());
			OnDoRealize((WPARAM)m_hWnd,0);  // realize the new palette
			pDoc->UpdateAllViews(NULL);
		}
		EndWaitCursor();
	}
}


void CDibView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(::IsClipboardFormatAvailable(CF_DIB));
}

void CDibView::OnProcessingParcurgereSimpla() 
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();

	// Makes a grayscale image by equalizing the R, G, B components from the LUT
	for (int k=0;  k < iColors ; k++)
		bmiColorsDst[k].rgbRed=bmiColorsDst[k].rgbGreen=bmiColorsDst[k].rgbBlue=k;

	
	//  Goes through the bitmap pixels and performs their negative	
	for (int i=0;i<dwHeight;i++)
		for (int j=0;j<dwWidth;j++)
		  {	
			lpDst[i*w+j]= 255 - lpSrc[i*w+j]; //makes image negative
	  }

	END_PROCESSING("Negativ imagine");
}


typedef struct Point {
	int x;
	int y;
} Point;


Point points[1000];
void CDibView::OnRANSAC_Line()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();
		double t	= 10;
	double p		= 0.99;
	double www		= 0.3;
	double s		= 2;

	double a, b, c;

	Point start, stop;

	int n			= 0;
	int inliers		= 0;
	double percent	= 0;
	int p1			= 0;
	int p2			= 0;

	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			if (lpSrc[i*w+j] == 0)
			{
				points[n++].x = j;
				points[n].y = i;
			}
		}
	}

	srand(time(NULL));
	

	while (percent < 30)
	{
		p1 = rand() % n;
		p2 = rand() % n;

		start = points[p1];
		stop  = points[p2];

		a = points[p1].y - points[p2].y;
		b = points[p2].x - points[p1].x;
		c = points[p1].x * points[p2].y - points[p2].x * points[p1].y;

		inliers = 0;

		for (int i = 0; i < n; i++)
		{
			double d = abs(a * points[i].x + b * points[i].y + c) / sqrt(a * a + b * b); 
			if (d < t)
			{
				inliers++;
			}
		}

		percent = (inliers * 100 / n);

	}


	CDC dc;
	dc.CreateCompatibleDC(0);
	CBitmap ddBitmap;
	HBITMAP hDDBitmap = CreateDIBitmap(::GetDC(0),
		&((LPBITMAPINFO)lpS)->bmiHeader, CBM_INIT, lpSrc,
		(LPBITMAPINFO)lpS, DIB_RGB_COLORS);
	ddBitmap.Attach(hDDBitmap);
	CBitmap* pTempBmp = dc.SelectObject(&ddBitmap);
	CPen pen(PS_SOLID, 1, RGB(255,0,0));
	CPen *pTempPen = dc.SelectObject(&pen);
	// drawing a line from point (x1,y1) to point (x2,y2)
	//int x1 = points[p1].x;
	//int y1 = points[p1].y;
	//int x2 = points[p2].x;
	//int y2 = points[p2].y;

	int x1 = 0;
	int y1 = -c/b;
	int x2 = dwWidth;
	int y2 = ( -a*dwWidth - c )/ b;

	dc.MoveTo(x1,dwHeight-1-y1);
	dc.LineTo(x2,dwHeight-1-y2);
	dc.SelectObject(pTempPen);
	dc.SelectObject(pTempBmp);
	GetDIBits(dc.m_hDC, ddBitmap, 0, dwHeight, lpDst,
		(LPBITMAPINFO)lpD, DIB_RGB_COLORS);

	END_PROCESSING("RANSAC - Line");


}

struct hough_data {
	int v;
	int rho;
	int theta;
};

bool hough_compare_desc(
	const hough_data &a,
	const hough_data &b) 
{
	return a.v > b.v;
}


std::vector <hough_data> hlist;

int *H;
int diagSize;


void CDibView::OnProcessingL3()
{
	BEGIN_PROCESSING();

	//int H[360][200];

	diagSize = sqrt( (double) (dwWidth * dwWidth + dwHeight * dwHeight));

	H = (int *) malloc(diagSize * 360 * sizeof(int));

	for (int i = 0; i<diagSize; i++)
	{
		for (int j = 0; j < 360; j++)
		{
			H[i * 360 + j] = 0;
		}
	}

	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			if (lpSrc[i*w+j] == 255)
			{
				for (int t = 0; t < 360; t++)
				{
					double trad = (t * PI) / 180;
					int ro = j * cos(trad) + i * sin (trad);
					if (ro >= 0 && ro < 360){
						H[ro * 360 + t] += 1;
					}
				}
			}
		}
	}

	for (int i = 0; i<360; i++)
	{
		for (int j = 0; j < diagSize; j++)
		{
			hough_data hd;
			hd.v =  H[i * diagSize + j];
			hd.rho = j;
			hd.theta = i;
			hlist.push_back(hd);
		}
	}

	std::sort(hlist.begin(), hlist.end(), hough_compare_desc);



		CDC dc;
	dc.CreateCompatibleDC(0);
	CBitmap ddBitmap;
	HBITMAP hDDBitmap = CreateDIBitmap(::GetDC(0),
		&((LPBITMAPINFO)lpS)->bmiHeader, CBM_INIT, lpSrc,
		(LPBITMAPINFO)lpS, DIB_RGB_COLORS);
	ddBitmap.Attach(hDDBitmap);
	CBitmap* pTempBmp = dc.SelectObject(&ddBitmap);
	CPen pen(PS_SOLID, 1, RGB(255,0,0));
	CPen *pTempPen = dc.SelectObject(&pen);
	// drawing a line from point (x1,y1) to point (x2,y2)

	for (int i = 0; i < 15; i++)
	{
		hough_data hd = hlist.at(i);
		double trad = (hd.theta * PI) / 180;

		int x1 = 0;
		int y1 = abs( hd.rho / sin(trad));
		int x2 = dwWidth;
		int y2 =abs (( hd.rho - x2 * cos(trad) ) / sin (trad));

	/*	int x1 = 0;
		int y1 = 0;
		int x2 = dwWidth;
		int y2 = dwHeight;*/

		dc.MoveTo(x1,y1);
		dc.LineTo(x2,y2);
		dc.SelectObject(pTempPen);
		dc.SelectObject(pTempBmp);
		GetDIBits(dc.m_hDC, ddBitmap, 0, dwHeight, lpDst,
			(LPBITMAPINFO)lpD, DIB_RGB_COLORS);

	}

	//for (int i = 0; i < dwHeight; i++)
	//{
	//	for (int j = 0; j < dwWidth; j++)
	//	{
	//		if 
	//		lpDst[i*w+j] = H[i * diagSize + j];
	//	}
	//}

	END_PROCESSING("HOUGH - Lines");

}


void CDibView::OnProcessingL4()
{
	BEGIN_PROCESSING();

	for (int i = 0; i < dwHeight-1; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			lpDst[i*w+j] = H[i * 360 + j];
		}
	}

	END_PROCESSING("Display H");

}


void CDibView::OnProcessingL5()
{
	
	BEGIN_PROCESSING();
	
	int mask[3][3] = {{7, 5, 7}, {5, 0, 5}, {7, 5, 7}};

	int *DT;
	DT = (int *) malloc(w * dwHeight * sizeof (int) );
	

	//initialize DT
	for (int i = 0; i < dwHeight-1; i++)
		for (int j = 0; j < dwWidth; j++)
			DT[i*w+j] = lpSrc[i*w+j];

	//First parse
	for (int i = 1; i < dwHeight-1; i++)
	{
		for (int j = 1; j < dwWidth - 1; j++)
		{	
			int min = DT[i*w+j];
			int tmp;

			for (int k = 0; k < 2; k++)
			{
				for (int l = 0; l < 3; l++)
				{
					if (k == 0 || (k == 1 && l < 2))
					{
						tmp = DT[ (i + k - 1)*w + (j + l-1)] + mask[k][l];
						if (tmp < min)
							min = tmp;
					}
				}
			}
			
			DT[i*w+j] = min;
		}
	}


	
	//Second parse
	for (int i = dwHeight - 2; i > 0; i--)
	{
		for (int j = dwWidth - 2; j > 0; j--)
		{	
			int min = DT[i*w+j];
			int tmp;

			for (int k = 1; k < 3; k++)
			{
				for (int l = 0; l < 3; l++)
				{
					if (k == 2 || (k == 1 && l > 0))
					{
						tmp = DT[ (i + k - 1)*w + (j + l-1)] + mask[k][l];
						if (tmp < min)
							min = tmp;
					}
				}
			}
			
			//DT[i*w+j] = min;
		}
	}

	//Copy result to output image
	for (int i = 0; i < dwHeight; i++)
		for (int j = 0; j < dwWidth; j++)
			lpDst[i*w+j] = DT[i*w+j];


	END_PROCESSING("Distance transform");
}


void CDibView::OnProcessingL6()
{
	BEGIN_PROCESSING();

	int dx[] = {-1, 0, 1};
	int dy[] = {1, 0, -1};

	
	int *DX, *DY, *Magnitude, *Orientation, *Hystogram;
	DX = (int *) malloc(w * dwHeight * sizeof (int) );
	DY = (int *) malloc(w * dwHeight * sizeof (int) );
	Magnitude = (int *) malloc(w * dwHeight * sizeof (int) );
	Orientation = (int *) malloc(w * dwHeight * sizeof (int) );
	Hystogram = (int *) malloc(w * dwHeight * 9 / 16);
	

	for (int i = 1; i < dwHeight-1; i++)
	{
		for (int j = 1; j < dwWidth-1; j++)
		{
			DX[i*w+j] = abs (lpSrc[i*w+j+1] - lpSrc[i*w+j-1]);
			DY[i*w+j] = abs (lpSrc[(i-1)*w+j] - lpSrc[(i+1)*w+j]);
			Magnitude[i*w+j] = sqrt ( DX[i*w+j] * DX[i*w+j] + DY[i*w+j] * DY[i*w+j] );
			Orientation[i*w+j] = atan2( DY[i*w+j], DX[i*w+j] ) * 57.2957795;
		}
	}

	int cellHeight = dwHeight / 16;
	int cellWidth = dwWidth / 16;

	for (int i = 1; i < cellHeight ; i++)
	{
		for (int j = 1; j < cellWidth; j++)
		{
			//parse each cell

			for (int ii = 0; ii < 16; i++)
			{
				for (int jj = 0; jj < 16; j++)
				{
					int value = Magnitude[(i * 16 + ii)* w + (j*16 + jj) ] / 26;
					if (value > 9)
					{
						//error
						//int aaaaa = 10 / 0;
					}

					Hystogram[i * cellHeight];
				
				}
			}

		}
	}

	for (int i = 0; i < dwHeight; i++)
		for (int j = 0; j < dwWidth; j++)
			lpDst[i*w+j] = Magnitude[i*w+j];






	END_PROCESSING("HOG");
}


//#define RED		0;
//#define BLUE	1;
//
//#define X		0;
//#define Y		1;

void CDibView::OnFinalprojectLda()
{
	BEGIN_PROCESSING();

	Point *red, *blue;
	red		= (Point *) malloc(dwHeight * dwWidth * sizeof(Point) / 10);
	blue	= (Point *) malloc(dwHeight * dwWidth * sizeof(Point) / 10);

	int countRed = 0,
		countBlue = 0;

	int RED		= 0,
		BLUE	= 1,
		X		= 0,
		Y		= 1;

	double u[2][2],
		s1[2][2],
		s2[2][2],
		sw[2][2];


	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			u[i][j]  = 0;
			s1[i][j] = 0;
			s2[i][j] = 0;
			sw[i][j] = 0;
		}
	}

	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			byte pixel =  lpSrc[i*w+j];

			//search red dots
			if (bmiColorsSrc[pixel].rgbRed == 255 && bmiColorsSrc[pixel].rgbBlue == 0){
				red[countRed].x = i;
				red[countRed].y = j;
				
				u[RED][X]+=i;
				u[RED][Y]+=j;

				countRed++;
			}
			//search blue dots
			if (bmiColorsSrc[pixel].rgbRed == 0 && bmiColorsSrc[pixel].rgbBlue == 255){
				blue[countBlue].x = i;
				blue[countBlue].y = j;
				
				u[BLUE][X]+=i;
				u[BLUE][Y]+=j;

				countBlue++;
			}
		}
	}

	for (int i = 0; i < 2; i++) 
	{
		u[RED][i] = u[RED][i] / countRed;
		u[BLUE][i] = u[BLUE][i] / countBlue;
	}


	//S1
	for (int i = 0; i < countRed; i++)
	{
		double xx = red[i].x - u[RED][X];
		double yy = red[i].y - u[RED][Y]; 
		
		s1[0][0] = xx * xx;
		s1[0][1] = xx * yy;
		s1[1][0] = yy * xx;
		s1[1][1] = yy * yy;
	}

	s1[0][0] /= 5;
	s1[0][1] /= 5;
	s1[1][0] /= 5;
	s1[1][1] /= 5;

	//S2
	for (int i = 0; i < countBlue; i++)
	{
		double xx = blue[i].x - u[BLUE][X];
		double yy = blue[i].y - u[BLUE][Y]; 
		
		s2[0][0] = xx * xx;
		s2[0][1] = xx * yy;
		s2[1][0] = yy * xx;
		s2[1][1] = yy * yy;
	}

	s2[0][0] /= 5;
	s2[0][1] /= 5;
	s2[1][0] /= 5;
	s2[1][1] /= 5;


	//Scatter matrix
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			sw[i][j] = s1[i][j] + s2[i][j];
		}
	}


	END_PROCESSING("LDA");
}



#include <afxdisp.h>
void CDibView::OnProcessingL7()
{
	// In the include section please add:
	// #include <afxdisp.h>
	BEGIN_PROCESSING();
	BYTE *lpSI,*lpSrcI;
	DWORD dwWidthI,dwHeightI,wI;
	HDIB hBmpSrcI;
	CFile fileIn;
	CFileException fE;
	AfxEnableControlContainer();
	char buffer[MAX_PATH];
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	SHGetPathFromIDList(SHBrowseForFolder(&bi), buffer);
	if (strcmp(buffer,"")==0) return;
	char directoryPath[MAX_PATH];
	CFileFind fFind;
	int nextFile;
	CString msg;
	strcpy(directoryPath,buffer);
	strcat(directoryPath,"\\face*.bmp"); // search files with the name face* and having the extension .bmp
	nextFile=fFind.FindFile(directoryPath);
	int nrImages=0;

	byte **images = new byte*[400];
	double **covariance = new double*[400];

	int p = 400;
	int imageSize = 19 * 19;

	//images = (double *) malloc( p * imageSize * sizeof(double) );
	for (int i = 0; i < p; i++)
	{
		images[i] = (byte*) malloc(imageSize * sizeof(byte));
		covariance[i] = (double*) malloc (p * sizeof(double));
		for (int j = 0; j< p; j++) 
		{
			covariance[i][j] = 0;
		}
	}
	
	double *u;
	double *d;
	u = (double *) malloc( imageSize * sizeof(double) );
	d = (double *) malloc( imageSize * sizeof(double) );

	
	

	while (nextFile)
	{
		
		nextFile=fFind.FindNextFile();
		CString fnIn=fFind.GetFilePath();
		fileIn.Open(fnIn, CFile::modeRead | CFile::shareDenyWrite, &fE);
		hBmpSrcI = (HDIB)::ReadDIBFile(fileIn);
		fileIn.Close();
		lpSI = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrcI);
		dwWidthI = ::DIBWidth((LPSTR)lpSI);
		dwHeightI = ::DIBHeight((LPSTR)lpSI);
		lpSrcI=(BYTE*)::FindDIBBits((LPSTR)lpSI);
		DWORD wI=WIDTHBYTES(dwWidthI*8);

		int imgPos = 0;
		for (int i = 0; i < dwHeightI; i++) {
			for (int j = 0; j < dwWidthI; j++) {
				images[nrImages][imgPos] = lpSrcI[i*w+j];
				imgPos++;
			}
		}
		 
		nrImages++;
		::GlobalUnlock((HGLOBAL)hBmpSrcI);
	}
		
	for (int i = 0; i < imageSize; i++)
	{
		u[i] = 0;
		d[i] = 0;
	}

	//Mean value
	for (int i = 0; i < nrImages; i++) 
	{
		//each picture
		for (int j = 0; j < imageSize; j++)
		{
			//each pixel in picture
			u[j] += images[i][j];
		}
	}

	for (int i = 0; i < imageSize; i++)
	{
		u[i] /= nrImages;
	}


	//Standard deviation

	for (int i = 0; i < nrImages; i++) 
	{
		//each picture
		for (int j = 0; j < imageSize; j++)
		{
			//each pixel in picture
			d[j] += pow (( u[j] - images[i][j] ), 2);
		}
	}

	for (int i = 0; i < imageSize; i++)
	{
		d[i] = sqrt( d[i] / nrImages );
	}

	//FILE  *f = fopen("out.txt", "w");

	for (int i = 0; i < imageSize; i++)
	{
		for (int j = 0; j < imageSize; j ++) 
		{
			for (int ii = 0; ii < p; ii++)
			{
				covariance[i][j] += (images[ii][i] - u[i]) * (images[ii][j] - u[j]);
			}
			covariance[i][j] /= p;
		}
	}

	int ppp = 0;
	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			lpDst[i*w+j] = u[ppp];
			ppp++;
		}
	}



	msg.Format("Found and processed %d images",nrImages);
	AfxMessageBox(msg);





	END_PROCESSING("Covariance");
}
