#pragma once

#include <afxwin.h> 
#include <afxext.h>
#include <afx.h>
#include "pch.h"
#include "ToolMain.h"
#include "resource.h"
#include "MFCFrame.h"
#include "SelectDialogue.h"
#include "ModelPreviewDialogue.h"
#include "TransformDialogue.h"
#include "VulkanRenderer/VulkanProject/Renderer.h"

class MFCMain : public CWinApp 
{
public:
	MFCMain();
	~MFCMain();
	BOOL InitInstance();
	int  Run();

private:

	CMyFrame * m_frame;	//handle to the frame where all our UI is
	HWND m_toolHandle;	//Handle to the MFC window
	ToolMain* m_ToolSystem;	//Instance of Tool System that we interface to. 
	Renderer* renderer;

	CRect WindowRECT;	//Window area rectangle. 
	SelectDialogue m_ToolSelectDialogue;			//for modeless dialogue, declare it here
	TransformDialogue m_ToolTransformationDialogue;
	ModelPreviewDialogue m_ToolModelPreviewDialogue;

	int m_width;		
	int m_height;
	
	//Interface funtions for menu and toolbar etc requires
	afx_msg void MenuFileQuit();
	afx_msg void MenuFileSaveTerrain();
	afx_msg void MenuEditSelect();
	afx_msg	void ToolBarButton1();
	afx_msg	void WireFrameButton();
	afx_msg	void LightsButton();
	afx_msg	void NormalsButton();
	afx_msg void ModelPreviewButton();
	afx_msg void transformationButton();



	DECLARE_MESSAGE_MAP()	// required macro for message map functionality  One per class
};
