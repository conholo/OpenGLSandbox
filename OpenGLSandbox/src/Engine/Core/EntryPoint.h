#pragma once

extern Engine::Application* Engine::CreateApplication();

int main()
{
	auto* app = Engine::CreateApplication();
	app->Run();
	delete app;
}