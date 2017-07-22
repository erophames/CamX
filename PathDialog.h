
#if !defined(AFX_PATHDIALOG_H__0F70BC86_11DB_11D4_B012_0000E8DD8DAA__INCLUDED_)
#define AFX_PATHDIALOG_H__0F70BC86_11DB_11D4_B012_0000E8DD8DAA__INCLUDED_

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__C5C1396E_1275_11D4_B013_0000E8DD8DAA__INCLUDED_)
#define AFX_STDAFX_H__C5C1396E_1275_11D4_B013_0000E8DD8DAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include "defines.h"

#ifdef WIN32

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C5C1396E_1275_11D4_B013_0000E8DD8DAA__INCLUDED_)

class CPathDialog;

// CPathDialogSub - intercepts messages from child controls
class CPathDialogSub : public CWnd
{
	friend CPathDialog;
public:
	CPathDialog* m_pPathDialog;
protected:
    afx_msg void OnOK();              // OK button clicked
	afx_msg void OnChangeEditPath();
    DECLARE_MESSAGE_MAP()
private:
};

/////////////////////////////////////////////////////////////////////////////
// CPathDialog dialog

class CPathDialog
{
	friend CPathDialogSub;
	// Construction
public:
	CPathDialog(LPCTSTR lpszCaption=NULL,
		LPCTSTR lpszTitle=NULL,
		LPCTSTR lpszInitialPath=NULL, 
		CWnd* pParent = NULL);
	
	CString GetPathName();
	virtual int DoModal();
	
	static int Touch(LPCTSTR lpPath, bool bValidate=true);
	static int MakeSurePathExists(LPCTSTR lpPath);
	static bool IsFileNameValid(LPCTSTR lpFileName);
	int ConcatPath(LPTSTR lpRoot, LPCTSTR lpMorePath);
	
private:
	static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam, LPARAM pData);
	
	LPCTSTR m_lpszCaption;
	LPCTSTR m_lpszInitialPath;
	
	TCHAR m_szPathName[MAX_PATH];
	
	BROWSEINFO m_bi;
	HWND m_hWnd;
	CWnd*	m_pParentWnd;
	bool m_bParentDisabled;
	bool m_bGetSuccess;
	
	CPathDialogSub m_PathDialogSub;
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#endif

#endif // !defined(AFX_PATHDIALOG_H__0F70BC86_11DB_11D4_B012_0000E8DD8DAA__INCLUDED_)
