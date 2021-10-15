#pragma once

namespace Engine
{
	class Time
	{
	public:
		Time(float time = 0.0f)
			:m_Time(time)
		{
		}

		operator float() const { return m_Time; }

		static float DeltaTimeMilliseconds() { return s_DeltaTime * 1000.0f; }
		static float ElapsedMilliseconds() { return s_Elapsed * 1000.0f; }

		static void Tick();

		static float DeltaTime() { return s_DeltaTime; }
		static float Elapsed() { return s_Elapsed; }


	private:
		static float s_Elapsed;
		static float s_LastFrameTime;
		static float s_DeltaTime;
		float m_Time = 0.0f;
	};
}