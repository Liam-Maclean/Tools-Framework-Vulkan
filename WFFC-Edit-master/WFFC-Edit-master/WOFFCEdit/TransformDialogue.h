
#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "afxwin.h"
#include "SceneObject.h"
#include <vector>

class TransformDialogue : public CDialogEx
{
	DECLARE_DYNAMIC(TransformDialogue)

public:
	TransformDialogue(CWnd* pParent = NULL); // Modeless
	virtual ~TransformDialogue();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TRANSFORMATION_DIALOG};
#endif

protected:
	afx_msg void End();		//kill the dialogue

	DECLARE_MESSAGE_MAP()

};

