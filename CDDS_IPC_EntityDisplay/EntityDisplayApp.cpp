#include "EntityDisplayApp.h"
#include "raylib.h"

#include <iostream>



EntityDisplayApp::EntityDisplayApp(int screenWidth, int screenHeight) : m_screenWidth(screenWidth), m_screenHeight(screenHeight)
{

}

EntityDisplayApp::~EntityDisplayApp()
{

}

bool EntityDisplayApp::Startup()
{
	// gain access to a named shared memory block that already exists
	InitWindow(m_screenWidth, m_screenHeight, "EntityDisplayApp");
	SetTargetFPS(60);

	m_filehandleSize = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"MySharedMemorySize");

	m_filehandleData = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"MySharedMemoryData");

	return true;
}

void EntityDisplayApp::Shutdown()
{
	// close the shared file
	CloseHandle(m_filehandleData);
	CloseHandle(m_filehandleSize);

	CloseWindow();        // Close window and OpenGL context
}
bool isTrue = true;

void EntityDisplayApp::Update(float deltaTime)
{
	ReadMySharedMemory();
}

void EntityDisplayApp::Draw()
{
	BeginDrawing();

	ClearBackground(RAYWHITE);

	// draw entities
	for (int i = 0; i < m_numEntities && m_entities != nullptr; i++)
	{
		auto& entity = m_entities[i];

		DrawRectanglePro(
			Rectangle{ entity.x, entity.y, entity.size, entity.size }, // rectangle
			Vector2{ entity.size / 2, entity.size / 2 }, // origin
			entity.rotation,
			Color{ entity.r, entity.g, entity.b, 255 });
	}

	// output some text, uses the last used colour
	DrawText("Press ESC to quit", 630, 15, 12, LIGHTGRAY);

	EndDrawing();
}

bool EntityDisplayApp::ReadMySharedMemory()
{
	//error checking if couldnt find handle
	if (m_filehandleData == nullptr || m_filehandleSize== nullptr)
	{
		std::cout << "Could not create file mapping object: " << GetLastError() << std::endl;
		return 1;
	}
	//map the memory from the shared block to a pointer we can manipulate 
	//numEnties is going to hold the 'size' from entityEditor.ccp which is the number of entities 
	int* numEntities = (int*)MapViewOfFile(m_filehandleSize, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
	//Maps entity data
	Entity* entities = (Entity*)MapViewOfFile(m_filehandleData, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Entity) * m_numEntities);

	//if the size if different or we havent created memory for out entites
	if (*numEntities != m_numEntities)
	{
		//make m_numEnties equal to the number of entities passed from EntityEditor.cpp
		m_numEntities = *numEntities;
		delete[] m_entities;
		//creates a new array with the size of int found in memory at m_fileHandleSize
		m_entities = new Entity[m_numEntities];
	}
	//puts the entities from memory into m_entities pointer
	for (int i = 0; i < m_numEntities; i++)
	{
		m_entities[i] = entities[i];
	}
	// unmap the memory block since we're done with it
	UnmapViewOfFile(entities);
	UnmapViewOfFile(numEntities);

	return 0;
}