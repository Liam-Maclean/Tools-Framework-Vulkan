
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
	afx_msg void HandModel(vk::wrappers::Model* model);
	afx_msg void HandBackModel(vk::wrappers::Model& model);

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TRANSFORMATION_DIALOG};
#endif

	CEdit positionX;
	CEdit positionY;
	CEdit positionZ;

	CEdit rotationX;
	CEdit rotationY;
	CEdit rotationZ;

	CEdit scaleX;
	CEdit scaleY;

	CEdit scaleZ;
	vk::wrappers::Model* selectedModel;



protected:
	afx_msg void End();		//kill the dialogue
	CListBox m_listBox;

	void InitialiseWindowValues();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnEnChangePosition();
};

