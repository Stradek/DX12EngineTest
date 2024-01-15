/*
	Copyright (c) 2023 Piotr Stradowski. All rights reserved.
	Software distributed under the permissive MIT License.
*/

#ifdef _DEBUG

#include "Debug.h"

#include "Common/TracingFunctionNames.h"

namespace Engine::Debug
{
	namespace Performance
	{

		void PerformanceProfiler::AddFrameStart(const char* const name)
		{
			assert(m_frameDataInProgress[name].IsValid() == false);

			FrameData newFrameData;
			newFrameData.startTime = Common::Time::GetTimeNow();

			m_frameDataInProgress[name] = newFrameData;
		}

		void PerformanceProfiler::AddFrameEnd(const char* const name)
		{
			FrameData& frameDataToFinish = m_frameDataInProgress.at(name);
			frameDataToFinish.endTime = Common::Time::GetTimeNow();

			m_performanceData.frameData[name].push_back(frameDataToFinish);

			// reset frame data
			frameDataToFinish = FrameData();
		}

		void PerformanceProfiler::IncreaseCounter(const char* const name)
		{
			m_performanceData.frameCounters[name].value++;
		}

		bool PerformanceProfiler::IsFrameFinished(const FrameData& frameData) const
		{
			return frameData.endTime != std::chrono::high_resolution_clock::time_point::min();
		}

		Engine::Debug::Performance::PerformanceData PerformanceProfiler::GetPerformanceData()
		{
			return m_performanceData;
		}

		void PerformanceProfiler::ResetFinishedData()
		{
			// remove all finished frames
			m_performanceData = PerformanceData();

			// reset counters
			m_performanceData.frameCounters.clear();
		}

		Engine::Debug::Performance::PerformanceData PerformanceProfiler::PopData()
		{
			PerformanceData finishedData = GetPerformanceData();
			ResetFinishedData();

			return finishedData;
		}
	}

	DebugManager::DebugManager() :
		m_debugInfoRefreshTime(std::chrono::seconds(1)),
		m_performanceProfiler(Performance::PerformanceProfiler())
	{

	}

	void DebugManager::StartUp()
	{
		
	}

	void DebugManager::StartDebugManagerClock()
	{
		m_debugUpdateClock.Start();
	}


	Common::Time::Duration DebugManager::CalculateAverageFrameDuration(const std::vector<Performance::FrameData>& frameDataList, const size_t frameCounter)
	{
		if (frameCounter < 1)
		{
			ENGINE_WARN("Engine Update Counter is less than 1. Cannot calculate average duration.");
			return Common::Time::Duration::min();
		}

		Common::Time::Duration frameDurationSum{};
		for (const Performance::FrameData& frameData : frameDataList)
		{
			frameDurationSum += frameData.startTime - frameData.endTime;
		}

		return frameDurationSum / frameCounter;
	}

	void DebugManager::LogPerformanceInfo(const double deltaTime)
	{
		Performance::PerformanceData performanceData = GetPerformanceProfiler().PopData();

		const size_t engineUpdateCounter = performanceData.frameCounters[sl_Engine_Update].value;
		const size_t renderFrameCounter = performanceData.frameCounters[sl_Engine_RenderFrame].value;

		const std::vector<Performance::FrameData> engineUpdateData = performanceData.frameData[sl_Engine_Update];
		const std::vector<Performance::FrameData> engineRenderFrameData = performanceData.frameData[sl_Engine_RenderFrame];

		Common::Time::Duration engineUpdateAvgDuration = CalculateAverageFrameDuration(engineUpdateData, engineUpdateCounter);
		Common::Time::Duration renderFrameAvgDuration = CalculateAverageFrameDuration(engineRenderFrameData, renderFrameCounter);
		
		const char* performanceLoggingInfoString =
			"\n"
			"Counters:\n"
			"Frames per Second(FPS): \t\t{}\n"
			"Engine Updates per Second: \t\t{}\n"
			"===========================================\n"
			"Timers (ms):\n"
			"Delta Time: \t\t\t{:.4f}\n"
			"Engine Update: \t\t\t{:.4f}\n"
			"Render Frame: \t\t\t{:.4f}\n"
			"===========================================\n";

		float engineUpdateAvgDurationMilliseconds = Common::Time::DurationCast<float, Common::Time::Miliseconds>(engineUpdateAvgDuration);
		float renderFrameAvgDurationMilliseconds = Common::Time::DurationCast<float, Common::Time::Miliseconds>(renderFrameAvgDuration);

		ENGINE_LOG(performanceLoggingInfoString,
			renderFrameCounter, engineUpdateCounter,
			deltaTime * 1000, engineUpdateAvgDurationMilliseconds, renderFrameAvgDurationMilliseconds);
	}

	void DebugManager::Update(const double deltaTime)
	{
		if (m_debugUpdateClock.GetDuration() >= m_debugInfoRefreshTime)
		{
			LogPerformanceInfo(deltaTime);
			LogMemoryInfo();

			m_debugUpdateClock.Start();
		}
	}

	void DebugManager::ShutDown()
	{

	}

	void DebugManager::LogMemoryInfo()
	{

	}
}

#endif // _DEBUG