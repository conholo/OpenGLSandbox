#pragma once
extern Engine::Application* Engine::CreateApplication();

int main()
{
	Engine::Log::Init();
	auto* app = Engine::CreateApplication();
	app->Run();
	delete app;
}