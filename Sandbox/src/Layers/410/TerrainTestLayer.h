#pragma once

#include "Engine.h"



class TerrainTestLayer : public Engine::Layer
{
public:
	TerrainTestLayer();
	~TerrainTestLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& e) override;
	void OnImGuiRender() override;

private:
	int m_LOD = 3;
	bool m_Wireframe = false;
	Engine::Camera m_Camera;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::Terrain> m_Terrain;
	Engine::Ref<Engine::Light> m_Light;
};