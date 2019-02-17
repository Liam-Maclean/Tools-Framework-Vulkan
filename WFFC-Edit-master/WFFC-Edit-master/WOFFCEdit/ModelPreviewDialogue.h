
#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "afxwin.h"
#include "SceneObject.h"
#include "VulkanRenderer/VulkanProject/PreviewWindowRender.h"
#include <vector>
class ModelPreviewDialogue : public CDialogEx
{
	DECLARE_DYNAMIC(ModelPreviewDialogue)
	PreviewWindowRender* windowPreview;

public:
	ModelPreviewDialogue(CWnd* pParent = NULL); // Modeless
	virtual ~ModelPreviewDialogue();

	void InitialiseRenderer(Renderer* renderer);

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MODEL_PREVIEW};
#endif

protected:
	afx_msg void End();		//kill the dialogue

	DECLARE_MESSAGE_MAP()
};

