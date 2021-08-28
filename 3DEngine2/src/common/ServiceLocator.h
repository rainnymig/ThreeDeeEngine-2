#pragma once
#include "pch.h"

namespace tde
{
	template<typename I>
	class ServiceLocator
	{
	public:
		static void Provide(std::shared_ptr<I> apService);
		static std::shared_ptr<I> Get();
	private:
		static std::shared_ptr<I> mpService;
	};

	template<typename I>
	inline void ServiceLocator<I>::Provide(std::shared_ptr<I> apService)
	{
		mpService = apService;
	}

	template<typename I>
	inline std::shared_ptr<I> ServiceLocator<I>::Get()
	{
		return mpService;
	}
}