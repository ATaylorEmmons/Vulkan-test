#pragma once

#include "projectDefs.h"

std::string readFileIntoString(std::string path) {
	std::ifstream in(path);

	std::stringstream buffer;
	buffer << in.rdbuf();
	in.close();
	return buffer.str();
}



void writeLine(std::string path, std::string str) {
	std::ofstream out(path, std::ios_base::app);

	out << std::endl;
	out << str;
	out.close();
}

void clearFile(std::string path) {
	std::ofstream out(path);

	out << std::endl;
	out.close();
}

static std::vector<char> readBinary(std::string& path) {
	std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
       	writeLine(DebugPath, "Failed to find file: " + path);
    }

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

void initDebug() {
	clearFile(DebugPath);
}

