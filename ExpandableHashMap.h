// ExpandableHashMap.h

#ifndef EXPANDABLEHASHMAP_H
#define EXPANDABLEHASHMAP_H

#include <vector>
#include <list>
#include <utility>

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5);
	~ExpandableHashMap();
	void reset();
	int size() const;
	void associate(const KeyType& key, const ValueType& value);

	// for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	// for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	// C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
	std::vector<std::list<std::pair<KeyType, ValueType>>> m_map;
	size_t m_size;
	double m_maxLoadFactor;
	unsigned int getBucketNumber(const KeyType& key) const
	{
		unsigned int hasher(const KeyType& k);
		unsigned int h = hasher(key);
		return h % m_map.size();
	}
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
	: m_map(8), m_size(0)
{
	m_maxLoadFactor = (maximumLoadFactor > 0 ? maximumLoadFactor : 0.5);
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
	m_map.clear();
	m_size = 0;
	m_map.resize(8);
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
	return m_size; 
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
	//if key already exists, then update the value and return
	ValueType* ptr = find(key);
	if (ptr != nullptr)
	{
		*ptr = value;
		return;
	}

	//insert a new association
	unsigned int bucketNum = getBucketNumber(key);
	m_map[bucketNum].emplace_back(key, value);
	m_size++;

	//rehash if necessary
	if (static_cast<double>(m_size) / m_map.size() > m_maxLoadFactor)
	{
		std::vector<std::list<std::pair<KeyType, ValueType>>> temp(m_map.size() * 2);
		std::swap(m_map, temp);
		for (size_t k = 0; k < temp.size(); k++)
		{
			for (auto it = temp[k].begin(); it != temp[k].end(); it++)
				m_map[getBucketNumber(it->first)].emplace_back(it->first, it->second);
		}
	}
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
	unsigned int bucketNum = getBucketNumber(key);
	for (auto it = m_map[bucketNum].begin(); it != m_map[bucketNum].end(); it++)
	{
		if (it->first == key)
			return &(it->second);
	}
	return nullptr;
}

#endif // !EXPANDABLEHASHMAP_H