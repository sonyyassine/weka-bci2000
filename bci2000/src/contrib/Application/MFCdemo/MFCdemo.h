// MFCdemo.h : Haupt-Header-Datei f�r die Anwendung MFCDEMO
//

#if !defined(AFX_MFCDEMO_H__84343563_3F97_4804_B2CD_924D5BDF6726__INCLUDED_)
#define AFX_MFCDEMO_H__84343563_3F97_4804_B2CD_924D5BDF6726__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// Hauptsymbole

/////////////////////////////////////////////////////////////////////////////
// CMFCdemoApp:
// Siehe MFCdemo.cpp f�r die Implementierung dieser Klasse
//

class CMFCdemoApp : public CWinApp
{
public:
	CMFCdemoApp();

// �berladungen
	// Vom Klassenassistenten generierte �berladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CMFCdemoApp)
	public:
	//}}AFX_VIRTUAL

// Implementierung

	//{{AFX_MSG(CMFCdemoApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif // !defined(AFX_MFCDEMO_H__84343563_3F97_4804_B2CD_924D5BDF6726__INCLUDED_)
