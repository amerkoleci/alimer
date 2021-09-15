// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Core/Object.h"
#include <memory>
#include <array>
#include <unordered_map>

namespace Alimer
{
	namespace details
	{
		struct Context
		{
			/// Object factories.
			std::unordered_map<StringId32, std::unique_ptr<ObjectFactory>> factories;

			void RegisterFactory(ObjectFactory* factory)
			{
				factories[factory->GetType()].reset(factory);
			}

            RefPtr<Object> CreateObject(StringId32 type, const std::string_view& name)
			{
				auto it = factories.find(type);
				return it != factories.end() ? it->second->Create(name) : nullptr;
			}

			const std::string& GetTypeNameFromType(StringId32 type)
			{

				auto it = factories.find(type);
				return it != factories.end() ? it->second->GetTypeName() : kEmptyString;
			}
		};

		Context& context()
		{
			static Context s_context;
			return s_context;
		}
	}

	TypeInfo::TypeInfo(const char* typeName_, const TypeInfo* baseTypeInfo_)
		: type(typeName_)
		, typeName(typeName_)
		, baseTypeInfo(baseTypeInfo_)
	{
	}

	bool TypeInfo::IsTypeOf(StringId32 type) const
	{
		const TypeInfo* current = this;
		while (current)
		{
			if (current->GetType() == type)
				return true;

			current = current->GetBaseTypeInfo();
		}

		return false;
	}

	bool TypeInfo::IsTypeOf(const TypeInfo* typeInfo) const
	{
		if (typeInfo == nullptr)
			return false;

		const TypeInfo* current = this;
		while (current)
		{
			if (current == typeInfo || current->GetType() == typeInfo->GetType())
				return true;

			current = current->GetBaseTypeInfo();
		}

		return false;
	}

	/* Object */
	bool Object::IsInstanceOf(StringId32 type) const
	{
		return GetTypeInfo()->IsTypeOf(type);
	}

	bool Object::IsInstanceOf(const TypeInfo* typeInfo) const
	{
		return GetTypeInfo()->IsTypeOf(typeInfo);
	}

	void Object::RegisterFactory(ObjectFactory* factory)
	{
		if (!factory)
			return;

		details::context().RegisterFactory(factory);
	}

    RefPtr<Object> Object::CreateObject(StringId32 objectType, const std::string_view& name)
	{
		return details::context().CreateObject(objectType, name);
	}

	const std::string& Object::GetTypeNameFromType(StringId32 type)
	{
		return details::context().GetTypeNameFromType(type);
	}
}
