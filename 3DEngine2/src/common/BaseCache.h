#pragma once
#include "pch.h"

namespace tde
{
	template<typename K, typename T>
	class ICache
	{
	public:
		class KeyNotFoundException : public std::exception
		{};

		virtual bool Insert(const K& aKey, const T& aItem) = 0;
		virtual bool InsertIfNotExists(const K& aKey, const T& aItem) = 0;
		virtual bool Exists(const K& aKey) const = 0;
		virtual T Get(const K& aKey) const = 0;
	};

	//	NOT thread safe
	template<typename K, typename T>
	class BaseCache : public ICache<K, T>
	{
	public:
		bool Insert(const K& aKey, const T& aItem) override;
		bool InsertIfNotExists(const K& aKey, const T& aItem) override;
		bool Exists(const K& aKey) const override;
		T Get(const K& aKey) const override;
	private:
		std::unordered_map<K, T> mCache;
	};

	template<typename K, typename T>
	inline bool BaseCache<K, T>::Insert(const K& aKey, const T& aItem)
	{
		mCache[aKey] = aItem;
		return true;
	}

	template<typename K, typename T>
	inline bool BaseCache<K, T>::InsertIfNotExists(const K& aKey, const T& aItem)
	{
		if (Exists(aKey))
		{
			return false;
		}
		Insert(aKey, aItem);
		return true;
	}

	template<typename K, typename T>
	inline bool BaseCache<K, T>::Exists(const K& aKey) const
	{
		return mCache.find(aKey) != mCache.end();
	}

	template<typename K, typename T>
	inline T BaseCache<K, T>::Get(const K& aKey) const
	{
		if (!Exists(aKey))
		{
			throw ICache<K, T>::KeyNotFoundException();
		}
		return mCache.at(aKey);
	}
}