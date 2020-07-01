// CONSOLE 1 - creator of the named shared memory block
#include <iostream>
#include <windows.h>
#include <conio.h>
struct MyData{
	int i;
	float f;
	char c;
	bool b;
	double d;
};
int main() {
	MyData myData = { 99, 7.56f, 'A', true, 69.42 };
	// open a named shared memory block
	HANDLE fileHandle = CreateFileMapping(INVALID_HANDLE_VALUE, // a handle to anexisting virtual file, or invalid  
		nullptr, // optional security attributes  
		PAGE_READWRITE, // read/write access control  
		0, sizeof(MyData), // size of the memory block,   
		L"MySharedMemory");
	if (fileHandle == nullptr) {
		std::cout << "Could not create file mapping object: " << GetLastError()
			<< std::endl;
		return 1;}
	// map the memory from the shared block to a pointer we can manipulate
	MyData* data = (MyData*)MapViewOfFile(fileHandle, FILE_MAP_ALL_ACCESS, 0, 
		0, sizeof(MyData));
	if (data == nullptr) {
		std::cout << "Could not map view of file: " << GetLastError() 
			<< std::endl;
		CloseHandle(fileHandle);
		return 1
			;}
	// write to the memory block
	*data = myData;
	// wait for a keypress to close
	_getch();
	// unmap the memory block since we're done with it
	UnmapViewOfFile(data);
	// close the shared file
	CloseHandle(fileHandle); return 0;
}