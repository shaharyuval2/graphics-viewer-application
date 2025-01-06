#include "Utils.h"
#include <windows.h>
#include <iostream>
#include <fstream>


//helper function to convert a quaternion to a 4x4 rotation matrix
void ConvertQuaternionToMatrix(const double quat[4], double mat[16])
{
	double yy2 = 2.0f * quat[1] * quat[1];
	double xy2 = 2.0f * quat[0] * quat[1];
	double xz2 = 2.0f * quat[0] * quat[2];
	double yz2 = 2.0f * quat[1] * quat[2];
	double zz2 = 2.0f * quat[2] * quat[2];
	double wz2 = 2.0f * quat[3] * quat[2];
	double wy2 = 2.0f * quat[3] * quat[1];
	double wx2 = 2.0f * quat[3] * quat[0];
	double xx2 = 2.0f * quat[0] * quat[0];
	mat[0*4+0] = - yy2 - zz2 + 1.0f;
	mat[0*4+1] = xy2 + wz2;
	mat[0*4+2] = xz2 - wy2;
	mat[0*4+3] = 0;
	mat[1*4+0] = xy2 - wz2;
	mat[1*4+1] = - xx2 - zz2 + 1.0f;
	mat[1*4+2] = yz2 + wx2;
	mat[1*4+3] = 0;
	mat[2*4+0] = xz2 + wy2;
	mat[2*4+1] = yz2 - wx2;
	mat[2*4+2] = - xx2 - yy2 + 1.0f;
	mat[2*4+3] = 0;
	mat[3*4+0] = mat[3*4+1] = mat[3*4+2] = 0;
	mat[3*4+3] = 1;
}



void displayMessage(const std::string& str)
{
	std::wstring wStr(str.begin(), str.end());
	MessageBox(NULL, wStr.c_str(), TEXT("Renderer"), MB_OK);
}


std::wstring getOpenFileName()
{
	const int strMaxLen = 10000;
	OPENFILENAME ofn = {0};
	TCHAR fileStr[strMaxLen] = {0};

	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = fileStr;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fileStr)/sizeof(TCHAR) - 1;

	GetOpenFileName(&ofn);
	return fileStr;
}
