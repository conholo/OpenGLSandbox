#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Engine/Rendering/Line.h"

class CurveSerializer
{
public:
	static void SerializeCurve(const std::string& saveName, const Engine::Ref<Engine::BezierCurve>& curve, const std::string& subDirectoryName);
	static void DeserializeAndWriteToCurve(const std::string& saveName, const Engine::Ref<Engine::BezierCurve>& curve);
	
	static std::vector<std::string> GetCurveSavesFromDirectory(const std::string& subDirectory);
	static std::unordered_map<std::string, std::vector<std::string>> GetAllCurvePaths();

private:
	static std::string s_DirectoryPath;
};