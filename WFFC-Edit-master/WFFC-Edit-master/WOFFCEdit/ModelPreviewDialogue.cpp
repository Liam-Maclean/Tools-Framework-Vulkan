#include "ModelPreviewDialogue.h"

IMPLEMENT_DYNAMIC(ModelPreviewDialogue, CDialogEx)

//Message map.  Just like MFCMAIN.cpp.  This is where we catch button presses etc and point them to a handy dandy method.
BEGIN_MESSAGE_MAP(ModelPreviewDialogue, CDialogEx)

END_MESSAGE_MAP()

ModelPreviewDialogue::ModelPreviewDialogue(CWnd * pParent)
	:CDialogEx(IDD_MODEL_PREVIEW, pParent)
{
	//windowPreview = new PreviewWindowRender()
}

ModelPreviewDialogue::~ModelPreviewDialogue()
{
}

void ModelPreviewDialogue::InitialiseRenderer(Renderer* renderer)
{
	windowPreview = new PreviewWindowRender(renderer, 100, 100, GetModuleHandle(nullptr),this->GetSafeHwnd());
	windowPreview->InitialiseVulkanApplication();
}


void ModelPreviewDialogue::End()
{
	DestroyWindow();
}
