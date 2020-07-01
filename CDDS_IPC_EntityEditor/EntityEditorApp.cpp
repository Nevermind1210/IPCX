#include "EntityEditorApp.h"
#include <random>
#include <time.h>
#include "raylib.h"



#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "raygui.h"
#include <iostream>
#include <conio.h>

EntityEditorApp::EntityEditorApp(int screenWidth, int screenHeight) : m_screenWidth(screenWidth), m_screenHeight(screenHeight)
{

}

EntityEditorApp::~EntityEditorApp()
{

}

bool EntityEditorApp::Startup()
{
	// open a named shared memory block
	m_filehandleSize = CreateFileMapping(INVALID_HANDLE_VALUE, // a handle to an existing virtual file, or invalid
		nullptr, // optional security attributes
		PAGE_READWRITE, // read/write access control
		0, sizeof(int), // size of the memory block, 
		L"MySharedMemorySize");

	// open a named shared memory block
	m_filehandleData = CreateFileMapping(INVALID_HANDLE_VALUE, // a handle to an existing virtual file, or invalid
		nullptr, // optional security attributes
		PAGE_READWRITE, // read/write access control
		0, sizeof(Entity) * ENTITY_COUNT + sizeof(int), // size of the memory block, 
		L"MySharedMemoryData");

	InitWindow(m_screenWidth, m_screenHeight, "EntityDisplayApp");
	SetTargetFPS(60);

	srand(time(nullptr));
	for (auto& entity : m_entities) {
		entity.x = rand() % m_screenWidth;
		entity.y = rand() % m_screenHeight;
		entity.size = 10;
		entity.speed = rand() % 100;
		entity.rotation = rand() % 360;
		entity.r = rand() % 255;
		entity.g = rand() % 255;
		entity.b = rand() % 255;
	}

	return true;
}

void EntityEditorApp::Shutdown()
{

	CloseWindow();        // Close window and OpenGL context

	// close the shared file
	CloseHandle(m_filehandleData);
	CloseHandle(m_filehandleSize);
}

void EntityEditorApp::Update(float deltaTime)
{

	WriteToSharedMemory();
	// select an entity to edit
	static int selection = 0;
	static bool selectionEditMode = false;
	static bool xEditMode = false;
	static bool yEditMode = false;
	static bool rotationEditMode = false;
	static bool sizeEditMode = false;
	static bool speedEditMode = false;
	static Color colorPickerValue = WHITE;


	if (GuiSpinner(Rectangle{ 90, 25, 125, 25 }, "Entity", &selection, 0, ENTITY_COUNT - 1, selectionEditMode)) selectionEditMode = !selectionEditMode;

	int intX = (int)m_entities[selection].x;
	int intY = (int)m_entities[selection].y;
	int intRotation = (int)m_entities[selection].rotation;
	int intSize = (int)m_entities[selection].size;
	int intSpeed = (int)m_entities[selection].speed;


	// display editable stats within a GUI	
	GuiGroupBox(Rectangle{ 25, 70, 480, 220 }, "Entity Properties");

	if (GuiValueBox(Rectangle{ 90, 90, 125, 25 }, "x", &intX, 0, m_screenWidth, xEditMode)) xEditMode = !xEditMode;
	m_entities[selection].x = intX;

	if (GuiValueBox(Rectangle{ 90, 120, 125, 25 }, "y", &intY, 0, m_screenHeight, yEditMode)) yEditMode = !yEditMode;
	m_entities[selection].y = intY;

	m_entities[selection].rotation = GuiSlider(Rectangle{ 90, 150, 125, 25 }, "rotation", TextFormat("%2.2f", m_entities[selection].rotation), m_entities[selection].rotation, 0, 360);
	m_entities[selection].size = GuiSlider(Rectangle{ 90, 180, 125, 25 }, "size", TextFormat("%2.2f", m_entities[selection].size), m_entities[selection].size, 0, 100);
	m_entities[selection].speed = GuiSlider(Rectangle{ 90, 210, 125, 25 }, "speed", TextFormat("%2.2f", m_entities[selection].speed), m_entities[selection].speed, 0, 100);

	colorPickerValue = GuiColorPicker(Rectangle{ 260, 90, 156, 162 }, Color{ m_entities[selection].r, m_entities[selection].g, m_entities[selection].b });
	m_entities[selection].r = colorPickerValue.r;
	m_entities[selection].g = colorPickerValue.g;
	m_entities[selection].b = colorPickerValue.b;


	// move entities

	for (int i = 0; i < ENTITY_COUNT; i++)
	{
		if (selection == i)
			continue;

		float s = sinf(m_entities[i].rotation) * m_entities[i].speed;
		float c = cosf(m_entities[i].rotation) * m_entities[i].speed;
		m_entities[i].x -= s * deltaTime;
		m_entities[i].y += c * deltaTime;

		// wrap position around the screen
		m_entities[i].x = fmod(m_entities[i].x, m_screenWidth);
		if (m_entities[i].x < 0)
			m_entities[i].x += m_screenWidth;
		m_entities[i].y = fmod(m_entities[i].y, m_screenHeight);
		if (m_entities[i].y < 0)
			m_entities[i].y += m_screenHeight;
	}
}

void EntityEditorApp::Draw()
{
	BeginDrawing();

	ClearBackground(RAYWHITE);

	// draw entities
	for (auto& entity : m_entities)
	{
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


bool EntityEditorApp::WriteToSharedMemory()
{
	//error checking if handle wasnt created yet...
	if (m_filehandleData == nullptr || m_filehandleSize == nullptr)
	{
		std::cout << "Could not create file mapping object: " << GetLastError() << std::endl;
		return 1;
	}
	//this creates a space in memory thie size of an int 4 bytes to store the number of entities we need to pass to the DisplayApp
	int* size = (int*)MapViewOfFile(m_filehandleSize, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));

	//creates handle for entity data
	unsigned char* data = (unsigned char*)MapViewOfFile(m_filehandleData, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Entity) * ENTITY_COUNT);

	//casting char to entity pointer
	Entity* entities = (Entity*)data;
	data += sizeof(Entity) * ENTITY_COUNT;

	//loop though our entities and put them in the memory for out mapped view
	for (int i = 0; i < ENTITY_COUNT; i++)
	{
		entities[i] = m_entities[i];
	}
	//makes size  equal to the number of entities
	*size = ENTITY_COUNT;

	// unmap the memory block since we're done with it
	UnmapViewOfFile(data);
	UnmapViewOfFile(size);

	return 0;
}