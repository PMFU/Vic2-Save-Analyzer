#pragma once

#include <string>

//@TODO: If someone knows how or can do it, make this so it is cross platform for opening a file selection dialogue box
//Open Windows File Browsing Dialogue and return the filename and path for the chosen file
const std::string openFilePickerFilename();

//Open Windows File Browsing Dialogue and return the file contents
const std::string openFilePickerFile();