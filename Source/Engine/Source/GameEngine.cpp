/*
	Copyright (c) 2023 Piotr Stradowski. All rights reserved.
	Software distributed under the permissive MIT License.
*/

#include <Engine/GameEngine.h>

#include <Common/Time.h>
#include <Common/Clock.h>

#include <stdio.h>

namespace Engine
{
	enum EngineBootingSequenceState
	{
		NotStarted,
		Initializing,
		RunningFirstEngineUpdate,
		FinishingBootingSequence,
		Finished,
	};

	Common::Time::Clock m_engineTickClock;
	Common::Time::Clock m_engineOneSecondClock;

	Common::Time::Clock m_engineUpdateClock;
	Common::Time::Clock m_renderFrameClock;

	Common::Time::Clock m_currentOperationClock;

	const unsigned int m_targetEngineUpdatesPerSecond	= 20;
	const unsigned int m_targetFramesPerSecond			= 60;
	const double m_targetEngineUpdateTime				= (double) Common::Time::SECOND_IN_MILLISECONDS / m_targetEngineUpdatesPerSecond;
	const double m_targetRenderFrameTime				= (double) Common::Time::SECOND_IN_MILLISECONDS / m_targetFramesPerSecond;

	unsigned int m_engineUpdatesThisSecondCounter	= 0;
	unsigned int m_renderedFramesThisSecondCounter	= 0;

	unsigned int m_engineUpdatesLastSecondCounter	= 0;
	unsigned int m_renderedFramesLastSecondCounter	= 0;

	double m_lastEngineUpdateDuration	= 0;
	double m_lastRenderedFrameDuration	= 0;

	double m_deltaTime = 0;

	EngineBootingSequenceState m_engineBootingState = NotStarted;

	// TODO: replace this later with event system
	unsigned int m_debugUpdateQueue = 0;

	EngineBootingSequenceState GetBootingSequenceState()
	{
		return m_engineBootingState;
	}

	void SetBootingSequenceState(EngineBootingSequenceState bootingState)
	{
		// assert(m_engineBootingState != bootingState)

		m_engineBootingState = bootingState;
	}

	void SetNextBootingSquenceState()
	{
		EngineBootingSequenceState currentBootingSequenceState = GetBootingSequenceState();
		if (currentBootingSequenceState < EngineBootingSequenceState::Finished) // change to assert(currentBootingSequenceState < EngineBootingSequenceState::Finished)
		{
			EngineBootingSequenceState nextBootingSequenceState = static_cast<EngineBootingSequenceState>(currentBootingSequenceState + 1);
			SetBootingSequenceState(nextBootingSequenceState);
		}
	}


	void FinishBootingSequence()
	{
		m_engineTickClock.Start();
		m_engineOneSecondClock.Start();

		m_engineUpdateClock.Start();
		m_renderFrameClock.Start();

		ClearEngineCounters();
	}

	void RunBootingSequence()
	{
		SetBootingSequenceState(EngineBootingSequenceState::Initializing);

		while (GetBootingSequenceState() != EngineBootingSequenceState::Finished)
		{
			switch (GetBootingSequenceState())
			{
			case EngineBootingSequenceState::Initializing:
				InitDependencies();
				PreInit();
				Init();

				SetNextBootingSquenceState();
				break;
			case EngineBootingSequenceState::RunningFirstEngineUpdate:
				m_engineTickClock.Start(); // should I move it to Init? should I replace it with const number?
				m_deltaTime = m_engineTickClock.GetDurationAsDouble();
				Update(m_deltaTime);

				SetNextBootingSquenceState();
				break;
			case EngineBootingSequenceState::FinishingBootingSequence:
				FinishBootingSequence();

				SetBootingSequenceState(EngineBootingSequenceState::Finished);
				break;
			}
		}
	}

	void InitDependencies()
	{
		return;
	}

	void PreInit()
	{
		return;
	}

	void Init()
	{
		return;
	}

	void Destroy()
	{
		return;
	}

	void Update(double deltaTime)
	{
		m_currentOperationClock.Start();

		/* START UPDATE */

		if (m_debugUpdateQueue > 0)
		{
			printf("[FPS: %d] Game ms: %f; Render ms: %f; Previous CPU Tick(DeltaTime) ms: %f\n", m_renderedFramesLastSecondCounter, m_lastEngineUpdateDuration, m_lastRenderedFrameDuration, m_deltaTime);
			printf("          Game Ticks per Second: %d; Render Updates per Second: %d\n", m_engineUpdatesLastSecondCounter, m_renderedFramesLastSecondCounter);

			--m_debugUpdateQueue;
		}

		/* END UPDATE */

		m_currentOperationClock.Stop();
		m_lastEngineUpdateDuration = m_currentOperationClock.GetDurationAsDouble();

		++m_engineUpdatesThisSecondCounter;
	}

	void Render()
	{
		m_currentOperationClock.Start();

		/* START RENDER */



		/* END RENDER */

		m_currentOperationClock.Stop();
		m_lastRenderedFrameDuration = m_currentOperationClock.GetDurationAsDouble();

		++m_renderedFramesThisSecondCounter;
	}

	void ClearEngineCounters()
	{
		m_engineUpdatesLastSecondCounter = m_engineUpdatesThisSecondCounter;
		m_renderedFramesLastSecondCounter = m_renderedFramesThisSecondCounter;

		m_engineUpdatesThisSecondCounter = 0;
		m_renderedFramesThisSecondCounter = 0;
	}

	void Run()
	{
		// while here just to be safe as see all booting issues before starting engine ticks
		while (GetBootingSequenceState() != EngineBootingSequenceState::Finished)
		{
			RunBootingSequence();
		}

		while (true) // need to create good mechanism for breaking from this while when getting event or anything
		{
			m_engineTickClock.Start();

			if (m_engineUpdateClock.GetDurationAsDouble() >= m_targetEngineUpdateTime)
			{
				Update(m_deltaTime);

				m_engineUpdateClock.Reset();
			}

			if (m_renderFrameClock.GetDurationAsDouble() >= m_targetRenderFrameTime)
			{
				Render();
				m_renderFrameClock.Reset();
			}

			if (m_engineOneSecondClock.GetDurationAsDouble() >= Common::Time::SECOND_IN_MILLISECONDS)
			{
				ClearEngineCounters();
				m_debugUpdateQueue++;

				m_engineOneSecondClock.Reset();
			}

			m_deltaTime = m_engineTickClock.GetDurationAsDouble();

			m_engineTickClock.Reset();
		}
	}
}