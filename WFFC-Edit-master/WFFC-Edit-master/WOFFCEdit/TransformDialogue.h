
#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "afxwin.h"
//#include "SceneObject.h"
#include <vector>
#include "VulkanRenderer/VulkanProject/Shared.h"

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


	std::vector<vk::wrappers::Model*> * sceneModels;

protected:
	afx_msg void End();		//kill the dialogue
	CListBox m_listBox;

	DECLARE_MESSAGE_MAP()

};

