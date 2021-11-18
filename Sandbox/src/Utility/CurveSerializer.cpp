#include "CurveSerializer.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <glm/glm.hpp>

std::string CurveSerializer::s_DirectoryPath = "assets/curve-saves/";

void CurveSerializer::SerializeCurve(const std::string& saveName, const Engine::Ref<Engine::BezierCurve>& curve, const std::string& subDirectoryName)
{
	std::string path = s_DirectoryPath + subDirectoryName + "/" + saveName + std::string(".txt");

	if (!std::filesystem::exists(s_DirectoryPath + subDirectoryName))
		std::filesystem::create_directory(s_DirectoryPath + subDirectoryName);


	// Overwrite if it already exists.
	if (std::filesystem::exists(path.c_str()))
	{
		std::ofstream clearFile(path.c_str(), std::ofstream::out | std::ofstream::trunc);
		clearFile.close();
	}

	std::ofstream writeFile(path.c_str());
	writeFile << "Name: " << saveName << std::endl;
	writeFile << "Vertex Count: " << curve->GetVertices().size() << "\n\n";

	glm::vec3 lineColor = curve->GetCurveColor();
	glm::vec3 curveTranslation = curve->GetTransform()->GetPosition();
	glm::vec3 curveRotation = curve->GetTransform()->GetRotation();
	glm::vec3 curveScale = curve->GetTransform()->GetScale();

	writeFile << "Curve Color: (" << std::to_string(lineColor.x) << "," << std::to_string(lineColor.y) << "," << std::to_string(lineColor.z) << ")\n";
	writeFile << "Translation: (" << std::to_string(curveTranslation.x) << "," << std::to_string(curveTranslation.y) << "," << std::to_string(curveTranslation.z) << ")\n";
	writeFile << "Rotation: (" << std::to_string(curveRotation.x) << "," << std::to_string(curveRotation.y) << "," << std::to_string(curveRotation.z) << ")\n";
	writeFile << "Scale: (" << std::to_string(curveScale.x) << "," << std::to_string(curveScale.y) << "," << std::to_string(curveScale.z) << ")\n\n";

	uint32_t index = 0;
	for (auto lineVertex : curve->GetVertices())
	{
		std::string lineBegin = "Line Vertex - " + std::to_string(index) + ":\n\t";
		glm::vec3 position = lineVertex.Position;
		std::string positionString = "Position: (" + std::to_string(position.x) + "," + std::to_string(position.y) + "," + std::to_string(position.z) + ")\n";
		
		writeFile << lineBegin << positionString;
		index++;
	}

	writeFile.close();
}

static void ReadVec3FromFile(glm::vec3& output, const std::string& result, uint32_t cursorPosition)
{
	uint32_t componentIndex = 0;

	while (componentIndex < 3)
	{
		size_t comma = result.find_first_of(",", cursorPosition);
		size_t location = componentIndex == 2 ? result.find_first_of(")", cursorPosition) : comma;
		std::string component = result.substr(cursorPosition, location - cursorPosition);

		output[componentIndex++] = std::stof(component);

		cursorPosition = location + 1;
	}
}

void CurveSerializer::DeserializeAndWriteToCurve(const std::string& saveName, const Engine::Ref<Engine::BezierCurve>& curve)
{
	std::string path = s_DirectoryPath + saveName;

	if (!std::filesystem::exists(path.c_str()))
	{
		std::cout << "File with name: " << saveName << std::endl;
		return;
	}

	std::cout << "Loading Curve: " << saveName << ".  Please wait."<< std::endl;
	
	std::vector<Engine::LineVertex> vertices;

	std::ifstream in(path, std::ios::in | std::ios::binary);
	std::string result;
	if (in)
	{
		in.seekg(0, std::ios::end);
		size_t size = in.tellg();
		if (size != -1)
		{
			result.resize(size);
			in.seekg(0, std::ios::beg);
			in.read(&result[0], size);
		}
	}

	glm::vec3 color;
	glm::vec3 translation;
	glm::vec3 rotation;
	glm::vec3 scale;

	const char* colorStart = "Curve Color: (";
	size_t cursorPos = result.find(colorStart, 0);
	cursorPos += strlen(colorStart);
	ReadVec3FromFile(color, result, cursorPos);

	const char* translationStart = "Translation: (";
	cursorPos = result.find(translationStart, 0);
	cursorPos += strlen(translationStart);
	ReadVec3FromFile(translation, result, cursorPos);

	const char* rotationStart = "Rotation: (";
	cursorPos = result.find(rotationStart, 0);
	cursorPos += strlen(rotationStart);
	ReadVec3FromFile(rotation, result, cursorPos);

	const char* scaleStart = "Scale: (";
	cursorPos = result.find(scaleStart, 0);
	cursorPos += strlen(scaleStart);
	ReadVec3FromFile(scale, result, cursorPos);


	const char* start = "Line Vertex";
	cursorPos = result.find(start, 0);
	size_t fileLength = strlen(result.c_str());

	while (cursorPos < fileLength - 1)
	{
		// End of LineVertex Line
		cursorPos = result.find_first_of("\r\n", cursorPos);
		// Start of Position line
		size_t positionLine = result.find_first_not_of("\r\n\t", cursorPos);
		size_t positionLength = strlen("Position");
		cursorPos += positionLength;
		size_t afterFirstParenthesis = result.find_first_of("(", cursorPos) + 1;
		size_t lengthUntilLastParenthesis = result.find_first_of(")", afterFirstParenthesis);
		cursorPos = afterFirstParenthesis;

		glm::vec3 vertexPosition;
		uint32_t componentIndex = 0;
		while (componentIndex < 3)
		{
			size_t comma = result.find_first_of(",", cursorPos);
			size_t location = componentIndex == 2 ? result.find_first_of(")", cursorPos) : comma;
			std::string component = result.substr(cursorPos, location - cursorPos);

			vertexPosition[componentIndex++] = std::stof(component);

			cursorPos = location + 1;
		}

		cursorPos++;
		vertices.push_back({ vertexPosition });
	}

	curve->SetVertices(vertices);
	curve->SetCurveColor(color);
	curve->GetTransform()->SetPosition(translation);
	curve->GetTransform()->SetRotation(rotation);
	curve->GetTransform()->SetScale(scale);
}

std::vector<std::string> CurveSerializer::GetCurveSavesFromDirectory(const std::string& subDirectory)
{
	std::string path = s_DirectoryPath + subDirectory + "/";
	std::vector<std::string> files;
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string name = entry.path().filename().string();
		files.push_back(subDirectory + "/" + name);
	}

	return files;
}

std::unordered_map<std::string, std::vector<std::string>> CurveSerializer::GetAllCurvePaths()
{
	std::vector<std::string> subDirectories;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(s_DirectoryPath))
	{
		if (!entry.is_directory()) continue;
		subDirectories.push_back(entry.path().filename().string());
	}

	std::unordered_map<std::string, std::vector<std::string>> result;
	for (uint32_t i = 0; i < subDirectories.size(); i++)
		result[subDirectories[i]] = (GetCurveSavesFromDirectory(subDirectories[i]));

	return result;
}
