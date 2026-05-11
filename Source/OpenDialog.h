#pragma once
#include <string>
#include <minwindef.h>
#include <commdlg.h>

class OpenDialog
{
public:
	static std::string OpenLoadFileDialog()
	{
		char fileName[MAX_PATH] = "";

		OPENFILENAMEA ofn{};
		ofn.lStructSize = sizeof(ofn);

		static const char filter[] =
			"All Files (*.*)\0*.*\0";

		ofn.lpstrFilter = filter;
		ofn.lpstrFile = fileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;;

		if (GetOpenFileNameA(&ofn))
		{
			return std::string(fileName);
		}

		return "";
	}
};
