#include "MFCMain.h"
#include "resource.h"


BEGIN_MESSAGE_MAP(MFCMain, CWinApp)
	ON_COMMAND(ID_FILE_QUIT,	&MFCMain::MenuFileQuit)
	ON_COMMAND(ID_FILE_SAVETERRAIN, &MFCMain::MenuFileSaveTerrain)
	ON_COMMAND(ID_EDIT_SELECT, &MFCMain::MenuEditSelect)
	ON_COMMAND(ID_BUTTON40001,	&MFCMain::ToolBarButton1)
	ON_COMMAND(ID_WIREFRAME, &MFCMain::WireFrameButton)
	ON_COMMAND(ID_LIGHTING_ENABLED, &MFCMain::LightsButton)
	ON_COMMAND(ID_NORMAL_VIEW, &MFCMain::NormalsButton)
	ON_COMMAND(ID_EDIT_MODELPREVIEW, &MFCMain::ModelPreviewButton)
	ON_COMMAND(ID_EDIT_MODELTRANSFORMS, &MFCMain::transformationButton)
	ON_COMMAND(ID_NORMAL_VIEW, &MFCMain::NormalsButton)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_TOOL, &CMyFrame::OnUpdatePage)
END_MESSAGE_MAP()

BOOL MFCMain::InitInstance()
{
	//instanciate the mfc frame
	m_frame = new CMyFrame();
	renderer = new Renderer();
	m_pMainWnd = m_frame;

	m_frame->Create(	NULL,
					_T("World Of Flim-Flam Craft Editor"),
					WS_OVERLAPPEDWINDOW,
					CRect(100, 100, 1024, 768),
					NULL,
					NULL,
					0,
					NULL
				);
	
	
	//get the rect from the MFC window so we can get its dimensions
//m_toolHandle = Frame->GetSafeHwnd();						//handle of main window
	m_toolHandle = m_frame->m_DirXView.GetSafeHwnd();				//handle of directX child window
	m_frame->m_DirXView.GetWindowRect(&WindowRECT);
	m_frame->DrawMenuBar();
	m_width = WindowRECT.Width();
	m_height = WindowRECT.Height();

	m_frame->ShowWindow(SW_SHOW);
	m_frame->UpdateWindow();
	m_ToolSystem = new ToolMain(renderer, WindowRECT.Width(), WindowRECT.Height(), GetModuleHandle(nullptr), m_frame->m_DirXView.GetSafeHwnd());
	m_ToolSystem->onActionInitialise(m_toolHandle, 800, 600);

	return TRUE;
}

int MFCMain::Run()
{
	MSG msg;
	BOOL bGotMsg;

	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	while (WM_QUIT != msg.message)
	{
		if (true)
		{
			bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
		}
		else
		{
			bGotMsg = (GetMessage(&msg, NULL, 0U, 0U) != 0);
		}

		if (bGotMsg)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			m_ToolSystem->UpdateInput(&msg);
		}
		else
		{	
			

			int ID = m_ToolSystem->getCurrentSelectionID();

			std::wstring statusString = L"Selected Object: " + std::to_wstring(ID);
			m_ToolSystem->Tick(&msg);


			if (m_ToolTransformationDialogue)
			{
				vk::wrappers::Model model;
				m_ToolTransformationDialogue.HandBackModel(model);
				m_ToolSystem->UpdateModelTransform(model, ID);
			}

			//send current object ID to status bar in The main frame
			m_frame->m_wndStatusBar.SetPaneText(1, statusString.c_str(), 1);	
		}
	}

	return (int)msg.wParam;
}

void MFCMain::MenuFileQuit()
{
	//will post message to the message thread that will exit the application normally
	PostQuitMessage(0);
}

void MFCMain::MenuFileSaveTerrain()
{
	m_ToolSystem->onActionSaveTerrain();
}

void MFCMain::MenuEditSelect()
{
	//SelectDialogue m_ToolSelectDialogue(NULL, m_ToolSystem->models);		//create our dialoguebox //modal constructor
	//m_ToolSelectDialogue.DoModal();	// start it up modal

	//modeless dialogue must be declared in the class.   If we do local it will go out of scope instantly and destroy itself
	m_ToolSelectDialogue.Create(IDD_DIALOG1);	//Start up modeless
	m_ToolSelectDialogue.ShowWindow(SW_SHOW);	//show modeless
	m_ToolSelectDialogue.SetObjectData(m_ToolSystem->models, &m_ToolSystem->m_selectedObjectID);
}

void MFCMain::ToolBarButton1()
{
	m_ToolSystem->onActionSave();
}

void MFCMain::WireFrameButton()
{
	m_ToolSystem->onActionWireframeMode();
}

void MFCMain::LightsButton()
{
	m_ToolSystem->onActionLightEnabled();
}

void MFCMain::NormalsButton()
{
	m_ToolSystem->onActionNormalEnabled();
}

void MFCMain::ModelPreviewButton()
{
	//modeless dialogue must be declared in the class.   If we do local it will go out of scope instantly and destroy itself
	m_ToolModelPreviewDialogue.Create(IDD_MODEL_PREVIEW);	//Start up modeless
	m_ToolModelPreviewDialogue.ShowWindow(SW_SHOW);	//show modeless
	m_ToolModelPreviewDialogue.InitialiseRenderer(m_ToolSystem->GetRenderer());
}

void MFCMain::transformationButton()
{
	m_ToolTransformationDialogue.Create(IDD_TRANSFORMATION_DIALOG);
	m_ToolTransformationDialogue.HandModel(m_ToolSystem->onActionTransformWindowEnabled());
	m_ToolTransformationDialogue.ShowWindow(SW_SHOW);
	
}


MFCMain::MFCMain()
{
}


MFCMain::~MFCMain()
{
}
