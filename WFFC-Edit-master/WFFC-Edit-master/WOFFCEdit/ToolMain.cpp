#include "ToolMain.h"
#include "resource.h"
#include <vector>
#include <sstream>

//
ToolMain::~ToolMain()
{
	sqlite3_close(m_databaseConnection);		//close the database connection
}


int ToolMain::getCurrentSelectionID()
{
	return m_selectedObjectID;
}

std::string ToolMain::getCurrentSelectionName()
{
	return m_selectedObjectName;
}


void ToolMain::onActionInitialise(HWND handle, int width, int height)
{
	//window size, handle etc for directX
	m_width		= width;
	m_height	= height;

	//database connection establish
	int rc;
	rc = sqlite3_open("database/test.db", &m_databaseConnection);

	if (rc) 
	{
		TRACE("Can't open database");
		//if the database cant open. Perhaps a more catastrophic error would be better here
	}
	else 
	{
		TRACE("Opened database successfully");
	}
	onActionLoad();
}

void ToolMain::onActionWireframeMode()
{
	ToggleWireframeMode();
}

void ToolMain::onActionLightEnabled()
{
	ToggleLighting();
}

void ToolMain::onActionNormalEnabled()
{
	ToggleNormalMode();
}

void ToolMain::onActionLoad()
{
	//load current chunk and objects into lists
	//if (!m_sceneGraph.empty())		//is the vector empty
	//{
	//	m_sceneGraph.clear();		//if not, empty it
	//}

	//SQL
	int rc;
	char *sqlCommand;
	char *ErrMSG = 0;
	sqlite3_stmt *pResults;								//results of the query
	sqlite3_stmt *pResultsChunk;

	//OBJECTS IN THE WORLD
	//prepare SQL Text
	sqlCommand = "SELECT * from Objects";				//sql command which will return all records from the objects table.
	//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0 );
	
	//loop for each row in results until there are no more rows.  ie for every row in the results. We create and object
	while (sqlite3_step(pResults) == SQLITE_ROW)
	{	
		models.push_back(new vk::wrappers::Model());
		//Load model information for vulkan model
		models.back()->position = glm::vec4(sqlite3_column_double(pResults, 4), sqlite3_column_double(pResults, 5), sqlite3_column_double(pResults, 6), 1.0f);
		models.back()->rotation = glm::vec4(sqlite3_column_double(pResults, 7), sqlite3_column_double(pResults, 8), sqlite3_column_double(pResults, 9), 1.0f);
		models.back()->scale = glm::vec4(sqlite3_column_double(pResults, 10), sqlite3_column_double(pResults, 11), sqlite3_column_double(pResults, 12), 1.0f);
		models.back()->model_path  = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 2));
		models.back()->texture_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 3));
		models.back()->name = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 44));

		models.back()->ComputeMatrices();
		//After everything has loaded, compute the matrices
		
		//
		////Give the model to the vulkan renderer
		//_models.push_back(&newSceneModel);

		//Model load info
		//SceneObject newSceneObject;
		//newSceneObject.ID = sqlite3_column_int(pResults, 0);
		//newSceneObject.chunk_ID = sqlite3_column_int(pResults, 1);
		//newSceneObject.model_path		= reinterpret_cast<const char*>(sqlite3_column_text(pResults, 2));
		//newSceneObject.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 3));
		//newSceneObject.posX = sqlite3_column_double(pResults, 4);
		//newSceneObject.posY = sqlite3_column_double(pResults, 5);
		//newSceneObject.posZ = sqlite3_column_double(pResults, 6);
		//newSceneObject.rotX = sqlite3_column_double(pResults, 7);
		//newSceneObject.rotY = sqlite3_column_double(pResults, 8);
		//newSceneObject.rotZ = sqlite3_column_double(pResults, 9);
		//newSceneObject.scaX = sqlite3_column_double(pResults, 10);
		//newSceneObject.scaY = sqlite3_column_double(pResults, 11);
		//newSceneObject.scaZ = sqlite3_column_double(pResults, 12);
		//newSceneObject.render = sqlite3_column_int(pResults, 13);
		//newSceneObject.collision = sqlite3_column_int(pResults, 14);
		//newSceneObject.collision_mesh = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 15));
		//newSceneObject.collectable = sqlite3_column_int(pResults, 16);
		//newSceneObject.destructable = sqlite3_column_int(pResults, 17);
		//newSceneObject.health_amount = sqlite3_column_int(pResults, 18);
		//newSceneObject.editor_render = sqlite3_column_int(pResults, 19);
		//newSceneObject.editor_texture_vis = sqlite3_column_int(pResults, 20);
		//newSceneObject.editor_normals_vis = sqlite3_column_int(pResults, 21);
		//newSceneObject.editor_collision_vis = sqlite3_column_int(pResults, 22);
		//newSceneObject.editor_pivot_vis = sqlite3_column_int(pResults, 23);
		//newSceneObject.pivotX = sqlite3_column_double(pResults, 24);
		//newSceneObject.pivotY = sqlite3_column_double(pResults, 25);
		//newSceneObject.pivotZ = sqlite3_column_double(pResults, 26);
		//newSceneObject.snapToGround = sqlite3_column_int(pResults, 27);
		//newSceneObject.AINode = sqlite3_column_int(pResults, 28);
		//newSceneObject.audio_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 29));
		//newSceneObject.volume = sqlite3_column_double(pResults, 30);
		//newSceneObject.pitch = sqlite3_column_double(pResults, 31);
		//newSceneObject.pan = sqlite3_column_int(pResults, 32);
		//newSceneObject.one_shot = sqlite3_column_int(pResults, 33);
		//newSceneObject.play_on_init = sqlite3_column_int(pResults, 34);
		//newSceneObject.play_in_editor = sqlite3_column_int(pResults, 35);
		//newSceneObject.min_dist = sqlite3_column_double(pResults, 36);
		//newSceneObject.max_dist = sqlite3_column_double(pResults, 37);
		//newSceneObject.camera = sqlite3_column_int(pResults, 38);
		//newSceneObject.path_node = sqlite3_column_int(pResults, 39);
		//newSceneObject.path_node_start = sqlite3_column_int(pResults, 40);
		//newSceneObject.path_node_end = sqlite3_column_int(pResults, 41);
		//newSceneObject.parent_id = sqlite3_column_int(pResults, 42);
		//newSceneObject.editor_wireframe = sqlite3_column_int(pResults, 43);
		//newSceneObject.name = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 44));
	
	}

	sqlite3_close(m_databaseConnection);
	UpdateModelList(models);
	//Initialise the vulkan graphics framework with models loaded from SQLITE database
	InitialiseVulkanApplication();
}

void ToolMain::onActionSave()
{
	////SQL
	//int rc;
	//char *sqlCommand;
	//char *ErrMSG = 0;
	//sqlite3_stmt *pResults;								//results of the query
	//
	//
	////OBJECTS IN THE WORLD Delete them all
	////prepare SQL Text
	//sqlCommand = "DELETE FROM Objects";	 //will delete the whole object table.   Slightly risky but hey.
	//rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0);
	//sqlite3_step(pResults);
	//
	////Populate with our new objects
	//std::wstring sqlCommand2;
	//int numObjects = m_sceneGraph.size();	//Loop thru the scengraph.
	//
	//for (int i = 0; i < numObjects; i++)
	//{
	//	std::stringstream command;
	//	command << "INSERT INTO Objects " 
	//		<<"VALUES(" << m_sceneGraph.at(i).ID << ","
	//		<< m_sceneGraph.at(i).chunk_ID  << ","
	//		<< "'" << m_sceneGraph.at(i).model_path <<"'" << ","
	//		<< "'" << m_sceneGraph.at(i).tex_diffuse_path << "'" << ","
	//		<< m_sceneGraph.at(i).posX << ","
	//		<< m_sceneGraph.at(i).posY << ","
	//		<< m_sceneGraph.at(i).posZ << ","
	//		<< m_sceneGraph.at(i).rotX << ","
	//		<< m_sceneGraph.at(i).rotY << ","
	//		<< m_sceneGraph.at(i).rotZ << ","
	//		<< m_sceneGraph.at(i).scaX << ","
	//		<< m_sceneGraph.at(i).scaY << ","
	//		<< m_sceneGraph.at(i).scaZ << ","
	//		<< m_sceneGraph.at(i).render << ","
	//		<< m_sceneGraph.at(i).collision << ","
	//		<< "'" << m_sceneGraph.at(i).collision_mesh << "'" << ","
	//		<< m_sceneGraph.at(i).collectable << ","
	//		<< m_sceneGraph.at(i).destructable << ","
	//		<< m_sceneGraph.at(i).health_amount << ","
	//		<< m_sceneGraph.at(i).editor_render << ","
	//		<< m_sceneGraph.at(i).editor_texture_vis << ","
	//		<< m_sceneGraph.at(i).editor_normals_vis << ","
	//		<< m_sceneGraph.at(i).editor_collision_vis << ","
	//		<< m_sceneGraph.at(i).editor_pivot_vis << ","
	//		<< m_sceneGraph.at(i).pivotX << ","
	//		<< m_sceneGraph.at(i).pivotY << ","
	//		<< m_sceneGraph.at(i).pivotZ << ","
	//		<< m_sceneGraph.at(i).snapToGround << ","
	//		<< m_sceneGraph.at(i).AINode << ","
	//		<< "'" << m_sceneGraph.at(i).audio_path << "'" << ","
	//		<< m_sceneGraph.at(i).volume << ","
	//		<< m_sceneGraph.at(i).pitch << ","
	//		<< m_sceneGraph.at(i).pan << ","
	//		<< m_sceneGraph.at(i).one_shot << ","
	//		<< m_sceneGraph.at(i).play_on_init << ","
	//		<< m_sceneGraph.at(i).play_in_editor << ","
	//		<< m_sceneGraph.at(i).min_dist << ","
	//		<< m_sceneGraph.at(i).max_dist << ","
	//		<< m_sceneGraph.at(i).camera << ","
	//		<< m_sceneGraph.at(i).path_node << ","
	//		<< m_sceneGraph.at(i).path_node_start << ","
	//		<< m_sceneGraph.at(i).path_node_end << ","
	//		<< m_sceneGraph.at(i).parent_id << ","
	//		<< m_sceneGraph.at(i).editor_wireframe << ","
	//		<< "'" << m_sceneGraph.at(i).name << "'"
	//		<< ")";
	//	std::string sqlCommand2 = command.str();
	//	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand2.c_str(), -1, &pResults, 0);
	//	sqlite3_step(pResults);	
	//}
	MessageBox(NULL, L"Objects Saved", L"Notification", MB_OK);
}

void ToolMain::onActionSaveTerrain()
{

}

void ToolMain::Tick(MSG *msg)
{
	if (m_toolInputCommands.mouse_lb_down)
	{
		render = true;
		m_selectedObjectID = MousePicking(m_toolInputCommands);
		m_toolInputCommands.mouse_lb_down = false;

	}

	//if (render == true)
	//{
		//Renderer Update Call
		VulkanDeferredApplication::Update(WindowRECT);
	//}
}

void ToolMain::UpdateInput(MSG * msg)
{

	switch (msg->message)
	{
		//Global inputs,  mouse position and keys etc
	case WM_KEYDOWN:
		m_keyArray[msg->wParam] = true;
		break;

	case WM_KEYUP:
		m_keyArray[msg->wParam] = false;
		break;

	case WM_MOUSEMOVE:
		m_toolInputCommands.mouse_x = GET_X_LPARAM(msg->lParam);
		m_toolInputCommands.mouse_y = GET_Y_LPARAM(msg->lParam);
		break;

	case WM_LBUTTONDOWN:	//mouse button down,  you will probably need to check when its up too
		//set some flag for the mouse button in inputcommands
		m_toolInputCommands.mouse_lb_down = true;


		break;
	}
	//here we update all the actual app functionality that we want.  This information will either be used int toolmain, or sent down to the renderer (Camera movement etc
	//WASD movement
	if (m_keyArray['W'])
	{
		m_toolInputCommands.forward = true;
	}
	else m_toolInputCommands.forward = false;
	
	if (m_keyArray['S'])
	{
		m_toolInputCommands.back = true;
	}
	else m_toolInputCommands.back = false;
	if (m_keyArray['A'])
	{
		m_toolInputCommands.left = true;
	}
	else m_toolInputCommands.left = false;

	if (m_keyArray['D'])
	{
		m_toolInputCommands.right = true;
	}
	else m_toolInputCommands.right = false;
	//rotation
	if (m_keyArray['E'])
	{
		m_toolInputCommands.rotRight = true;
	}
	else m_toolInputCommands.rotRight = false;
	if (m_keyArray['Q'])
	{
		m_toolInputCommands.rotLeft = true;
	}
	else m_toolInputCommands.rotLeft = false;

	//WASD
}

