#pragma once

#include <string>

void ConvertQuaternionToMatrix(const double quat[4], double mat[16]);
void displayMessage(const std::string& str);
std::wstring getOpenFileName();