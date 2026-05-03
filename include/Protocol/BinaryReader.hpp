#pragma once

#include "View.hpp"
#include "Buffer.hpp"
#include "FieldProxy.hpp"

class BinaryReader
{
public:
	BinaryReader(const Buffer& buf) :m_view(buf) {};

    template<typename T>
    const T& Read(std::size_t offset) { return m_view.Read<T>(offset); }

    std::string_view ReadString(std::size_t offset) { return m_view.ReadString(offset); }

    template<typename T>
    std::span<const T> ReadArray(std::size_t offset) { return m_view.ReadArray<T>(offset); }

    template<typename T>
    const T& ReadTable(std::string_view name) { return m_view.ReadTable<T>(name); }

    template<typename T>
    const T& ReadTable(std::size_t index) { return m_view.ReadTable<T>(index); }

    std::string_view ReadTableString(std::string_view name) { return m_view.ReadTableString(name); }
    std::string_view ReadTableString(std::size_t index) { return m_view.ReadTableString(index); }

    template<typename T>
    std::span<const T> ReadTableArr(std::string_view name) { return m_view.ReadTableArr<T>(name); }

    template<typename T>
    std::span<const T> ReadTableArr(std::size_t index) { return m_view.ReadTableArr<T>(index); }

	FieldProxy operator[](std::string_view name)
	{
		return m_view[name];
	}

private:
	View m_view;
};