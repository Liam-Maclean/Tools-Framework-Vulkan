#include "TransformDialogue.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(TransformDialogue, CDialogEx)

//Message map.  Just like MFCMAIN.cpp.  This is where we catch button presses etc and point them to a handy dandy method.
BEGIN_MESSAGE_MAP(TransformDialogue, CDialogEx)

	ON_EN_CHANGE(IDC_POSITION_X, &TransformDialogue::OnEnChangePosition)
	ON_EN_CHANGE(IDC_SCALE_X, &TransformDialogue::OnEnChangePosition)
	ON_EN_CHANGE(IDC_ROTATION_X, &TransformDialogue::OnEnChangePosition)
	ON_EN_CHANGE(IDC_POSITION_Y, &TransformDialogue::OnEnChangePosition)
	ON_EN_CHANGE(IDC_SCALE_Y, &TransformDialogue::OnEnChangePosition)
	ON_EN_CHANGE(IDC_ROTATION_Y, &TransformDialogue::OnEnChangePosition)
	ON_EN_CHANGE(IDC_POSITION_Z, &TransformDialogue::OnEnChangePosition)
	ON_EN_CHANGE(IDC_SCALE_Z, &TransformDialogue::OnEnChangePosition)
	ON_EN_CHANGE(IDC_ROTATION_Z, &TransformDialogue::OnEnChangePosition)
END_MESSAGE_MAP()


TransformDialogue::TransformDialogue(CWnd * pParent)
	:CDialogEx(IDD_TRANSFORMATION_DIALOG, pParent)
{
}

TransformDialogue::~TransformDialogue()
{

}


void TransformDialogue::End()
{
	DestroyWindow();
}

void TransformDialogue::InitialiseWindowValues()
{
	CString text;

	CEdit* positionX = (CEdit*)GetDlgItem(IDC_POSITION_X);
	text.Format(_T("%f"), selectedModel->position.x);
	positionX->ReplaceSel(text);

	CEdit* positionY = (CEdit*)GetDlgItem(IDC_POSITION_Y);
	text.Format(_T("%f"), selectedModel->position.y);
	positionY->ReplaceSel(text);

	CEdit* positionZ = (CEdit*)GetDlgItem(IDC_POSITION_Z);
	text.Format(_T("%f"), selectedModel->position.z);
	positionZ->ReplaceSel(text);


	CEdit* scaleX = (CEdit*)GetDlgItem(IDC_SCALE_X);
	text.Format(_T("%f"), selectedModel->scale.x);
	scaleX->ReplaceSel(text);

	CEdit* scaleY = (CEdit*)GetDlgItem(IDC_SCALE_Y);
	text.Format(_T("%f"), selectedModel->scale.y);
	scaleY->ReplaceSel(text);

	CEdit* scaleZ = (CEdit*)GetDlgItem(IDC_SCALE_Z);
	text.Format(_T("%f"), selectedModel->scale.z);
	scaleZ->ReplaceSel(text);


	CEdit* rotationX = (CEdit*)GetDlgItem(IDC_ROTATION_X);
	text.Format(_T("%f"), selectedModel->rotation.x);
	rotationX->ReplaceSel(text);

	CEdit* rotationY = (CEdit*)GetDlgItem(IDC_ROTATION_Y);
	text.Format(_T("%f"), selectedModel->rotation.y);
	rotationY->ReplaceSel(text);

	CEdit* rotationZ = (CEdit*)GetDlgItem(IDC_ROTATION_Z);
	text.Format(_T("%f"), selectedModel->rotation.z);
	rotationZ->ReplaceSel(text);

}

void TransformDialogue::HandModel(vk::wrappers::Model* model)
{
	selectedModel = model;
	InitialiseWindowValues();
}

void TransformDialogue::HandBackModel(vk::wrappers::Model& model)
{
	model = *selectedModel;
}

void TransformDialogue::OnEnChangePosition()
{
	CString text;
	//UINT nCountOfCharacters =
	GetDlgItemText(IDC_POSITION_X, text);
	selectedModel->position.x = _ttof(text);

	GetDlgItemText(IDC_POSITION_Y, text);
	selectedModel->position.y = _ttof(text);

	GetDlgItemText(IDC_POSITION_Z, text);
	selectedModel->position.z = _ttof(text);



	GetDlgItemText(IDC_ROTATION_X, text);
	selectedModel->rotation.x = _ttof(text);

	GetDlgItemText(IDC_ROTATION_Y, text);
	selectedModel->rotation.y = _ttof(text);

	GetDlgItemText(IDC_ROTATION_Z, text);
	selectedModel->rotation.z = _ttof(text);



	GetDlgItemText(IDC_SCALE_X, text);
	selectedModel->scale.z = _ttof(text);

	GetDlgItemText(IDC_SCALE_Y, text);
	selectedModel->scale.z = _ttof(text);

	GetDlgItemText(IDC_SCALE_Z, text);
	selectedModel->scale.z = _ttof(text);

}
