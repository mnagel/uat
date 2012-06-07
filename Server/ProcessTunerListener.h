#pragma once

class ThreadObserver {
	public:
		virtual void threadFinished(void* context);
};
