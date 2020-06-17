#pragma once

#include <mw/file/File.h>

// Removes a file when object goes out of scope
class FileRemover
{
public:
	FileRemover(const File& file) : m_path(file.GetPath()) {}
	FileRemover(const FilePath& path) : m_path(path) {}
	~FileRemover() { m_path.Remove(); }

private:
	FilePath m_path;
};