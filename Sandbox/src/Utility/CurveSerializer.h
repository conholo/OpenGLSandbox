#pragma once

#include <string>
#include <vector>
#include "Engine/Rendering/Line.h"

class CurveSerializer
{
public:
	static void SerializeCurve(const std::string& saveName, const Engine::Ref<Engine::BezierCurve>& curve);
	static void DeserializeAndWriteToCurve(const std::string& saveName, const Engine::Ref<Engine::BezierCurve>& curve);
	
	static std::vector<std::string> GetCurveSaves();

private:
	static std::string s_DirectoryPath;
};