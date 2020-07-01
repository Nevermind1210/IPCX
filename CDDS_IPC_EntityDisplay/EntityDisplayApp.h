#pragma once
#include <vector>
#include"fixWindows.h"
#include <windows.h>



struct Entity 
{
	float x = 0, y = 0;
	float rotation = 0;
	float speed = 0;
	unsigned char r = 0, g = 0, b = 0;
	float size = 1;
};

class EntityDisplayApp  
{
public:
	EntityDisplayApp(int screenWidth = 800, int screenHeight = 450);
	~EntityDisplayApp();

	bool Startup();
	void Shutdown();

	void Update(float deltaTime);
	void Draw();

	bool ReadMySharedMemory();


protected:
	int m_screenWidth;
	int m_screenHeight;

	HANDLE m_filehandleData = nullptr;
	HANDLE m_filehandleSize = nullptr;

	int m_numEntities;
	Entity* m_entities = nullptr;

};