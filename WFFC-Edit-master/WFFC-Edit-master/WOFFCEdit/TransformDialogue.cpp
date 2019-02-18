#include "TransformDialogue.h"


IMPLEMENT_DYNAMIC(TransformDialogue, CDialogEx)

//Message map.  Just like MFCMAIN.cpp.  This is where we catch button presses etc and point them to a handy dandy method.
BEGIN_MESSAGE_MAP(TransformDialogue, CDialogEx)

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
