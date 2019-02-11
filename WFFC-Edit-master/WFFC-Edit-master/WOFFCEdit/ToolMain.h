#pragma once

#include <afxext.h>
#include "pch.h"
#include "sqlite3.h"
#include "SceneObject.h"
#include "InputCommands.h"
#include "VulkanRenderer/VulkanProject/VulkanDeferredApplication.h"
#include <vector>


class ToolMain : VulkanDeferredApplication
{
public: //methods
	ToolMain(Renderer* renderer, int width, int height, HINSTANCE instance, HWND window)
		:VulkanDeferredApplication(renderer, width, height,instance, window)
	{
		m_currentChunk = 0;		//default value
		m_selectedObject = 0;	//initial selection ID
		m_sceneGraph.clear();	//clear the vector for the scenegraph
		m_databaseConnection = NULL;

		//zero input commands
		m_toolInputCommands.forward = false;
		m_toolInputCommands.back = false;
		m_toolInputCommands.left = false;
		m_toolInputCommands.right = false;
	}
	~ToolMain();

	//onAction - These are the interface to MFC
	int		getCurrentSelectionID();										//returns the selection number of currently selected object so that It can be displayed.
	void	onActionInitialise(HWND handle, int width, int height);			//Passes through handle and hieght and width and initialises DirectX renderer and SQL LITE
	void	onActionFocusCamera();
	void	onActionLoad();													//load the current chunk
	afx_msg	void	onActionSave();											//save the current chunk
	afx_msg void	onActionSaveTerrain();									//save chunk geometry

	void	Tick(MSG *msg);
	void	UpdateInput(MSG *msg);

public:	//variables
	std::vector<SceneObject>    m_sceneGraph;	//our scenegraph storing all the objects in the current chunk
	int m_selectedObject;						//ID of current Selection

private:	//methods
	void	onContentAdded();


		
private:	//variables
	HWND	m_toolHandle;		//Handle to the  window
	InputCommands m_toolInputCommands;		//input commands that we want to use and possibly pass over to the renderer
	CRect	WindowRECT;		//Window area rectangle. 
	char	m_keyArray[256];
	sqlite3 *m_databaseConnection;	//sqldatabase handle

	int m_width;		//dimensions passed to directX
	int m_height;
	int m_currentChunk;			//the current chunk of thedatabase that we are operating on.  Dictates loading and saving. 
	

	
};
