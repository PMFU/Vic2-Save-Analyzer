#pragma once

#include <string>

//@TODO: Make this so it works across platforms in opening a file selection dialogue box
//Open Windows File Browsing Dialogue and return the filename and path for the chosen file
const std::string openFilePickerFilename();

//Open Windows File Browsing Dialogue and return the file contents
const std::string openFilePickerFile();