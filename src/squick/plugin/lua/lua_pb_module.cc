
#include "lua_pb_module.h"

bool LuaPBModule::Awake()
{
	//mSourceTree.MapPath("", "../config");

#ifdef SQUICK_DEV
	mSourceTree.MapPath("", "../src/proto");
#else
	mSourceTree.MapPath("", "../config/proto");
#endif

	m_pImporter = new google::protobuf::compiler::Importer(&mSourceTree, &mErrorCollector);
	m_pFactory = new google::protobuf::DynamicMessageFactory();

	mnTime = pPluginManager->GetNowTime();
   
	return true;
}

bool LuaPBModule::Init()
{

	m_pLogModule = this->pPluginManager->FindModule<ILogModule>();

    return true;
}

bool LuaPBModule::AfterInit()
{

    return true;
}

bool LuaPBModule::Shut()
{
	delete m_pFactory;
	m_pFactory = nullptr;

	delete m_pImporter;
	m_pImporter = nullptr;

    return true;
}

bool LuaPBModule::ReadyUpdate()
{
	return true;
}

bool LuaPBModule::Update()
{
    return true;
}

bool LuaPBModule::BeforeShut()
{
    return true;
}

void LuaPBModule::ImportProtoFile(const std::string & strFile)
{
	const google::protobuf::FileDescriptor* pDesc = m_pImporter->Import(strFile);
	if (!pDesc)
	{
		m_pLogModule->LogError("unknow protocol  file to import struct name: " + strFile );
	};
}

void LuaPBModule::SetLuaState(lua_State * pState)
{
	m_pLuaState = pState;
}

void LuaPBModule::PrintMessage(const google::protobuf::Message & message, const bool bEncode)
{
	return;
	if (bEncode)
	{
		std::cout << "********begin encode***********" << std::endl;
		std::cout << message.DebugString() << std::endl;
		std::cout << "*********end encode************" << std::endl;
	}
	else
	{
		std::cout << "********begin decode***********" << std::endl;
		std::cout << message.DebugString() << std::endl;
		std::cout << "*********end decode************" << std::endl;
	}
}

LuaIntf::LuaRef LuaPBModule::Decode(const std::string& strMsgTypeName, const std::string& strData)
{
	const google::protobuf::Descriptor* pDescriptor = m_pImporter->pool()->FindMessageTypeByName(strMsgTypeName);
    if (!pDescriptor)
    {
		//throw Exception("unknow message struct name: " + strMsgTypeName);
		return LuaIntf::LuaRef(m_pLuaState, nullptr);
    }

    const google::protobuf::Message* pProtoType = m_pFactory->GetPrototype(pDescriptor);
    if (!pProtoType)
    {
		//throw Exception("cannot find the message body from factory: " + strMsgTypeName);

		return LuaIntf::LuaRef(m_pLuaState, nullptr);
    }

    //GC
    std::shared_ptr<google::protobuf::Message> xMessageBody(pProtoType->New());

    if (xMessageBody->ParseFromString(strData))
    {
		PrintMessage(*xMessageBody, false);
		return MessageToTbl(*xMessageBody);
    }

    return LuaIntf::LuaRef(m_pLuaState, nullptr);
}

const std::string LuaPBModule::Encode(const std::string& strMsgTypeName, const LuaIntf::LuaRef& luaTable)
{
    luaTable.checkTable();

    const google::protobuf::Descriptor* pDesc = m_pImporter->pool()->FindMessageTypeByName(strMsgTypeName);
    if (!pDesc)
    {
        return NULL_STR;
    }

    const google::protobuf::Message* pProtoType = m_pFactory->GetPrototype(pDesc);
    if (!pProtoType)
    {
        return NULL_STR;
    }

    //GC
    std::shared_ptr<google::protobuf::Message> xMessageBody(pProtoType->New());

	if (TblToMessage(luaTable, *xMessageBody))
	{
		PrintMessage(*xMessageBody, true);

		return xMessageBody->SerializeAsString();
	}

	return NULL_STR;
}

LuaIntf::LuaRef LuaPBModule::MessageToTbl(const google::protobuf::Message& message) const
{
	const google::protobuf::Descriptor* pDesc = message.GetDescriptor();
	if (pDesc)
	{
		LuaIntf::LuaRef tbl = LuaIntf::LuaRef::createTable(m_pLuaState);

		int nField = pDesc->field_count();
		for (int i = 0; i < nField; ++i)
		{
			const google::protobuf::FieldDescriptor* pField = pDesc->field(i);
			if (pField)
			{
				tbl[pField->name()] = GetField(message, pField);

				if (pField->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE)
				{
					//std::cout << field->name() << " = " << pReflection->GetString(message, field) << std::endl;
					//std::cout << pField->name() << " = " << tbl[pField->name()].value<std::string>() << std::endl;
				}
			}
		}

		return tbl;
	}


	return LuaIntf::LuaRef();
}

LuaIntf::LuaRef LuaPBModule::GetField(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field) const
{

	if (nullptr == field)
	{
		return LuaIntf::LuaRef();
	}

	if (field->is_repeated())
	{
		return GetRepeatedField(message, field);
	}

	const google::protobuf::Reflection* pReflection = message.GetReflection();
	if (nullptr == pReflection)
	{
		return LuaIntf::LuaRef();
	}

	google::protobuf::FieldDescriptor::CppType eCppType = field->cpp_type();
	switch (eCppType)
	{
		// Scalar field always has a default value.
	case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetInt32(message, field));
	case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetInt64(message, field));
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetUInt32(message, field));
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetUInt64(message, field));
	case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetDouble(message, field));
	case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetFloat(message, field));
	case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetBool(message, field));
	case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetEnumValue(message, field));
	case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetString(message, field));
	case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
	{
		// For message field, the default value is null.
		if (pReflection->HasField(message, field))
		{
#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
#pragma push_macro("GetMessage")
#undef GetMessage
			const google::protobuf::Message& subMsg = pReflection->GetMessage(message, field);
#pragma pop_macro("GetMessage")
#else
			const google::protobuf::Message& subMsg = pReflection->GetMessage(message, field);
#endif
			return MessageToTbl(subMsg);
		}
		return  LuaIntf::LuaRef(m_pLuaState, nullptr);
	}

	default:
		break;	
	}


	return LuaIntf::LuaRef();
}

LuaIntf::LuaRef LuaPBModule::GetRepeatedField(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field) const
{
	if (!field->is_repeated())
	{
		return LuaIntf::LuaRef();
	}

	const google::protobuf::Reflection* pReflection = message.GetReflection();
	if (nullptr == pReflection)
	{
		return LuaIntf::LuaRef();
	}

	LuaIntf::LuaRef tbl = LuaIntf::LuaRef::createTable(m_pLuaState);
	int nFldSize = pReflection->FieldSize(message, field);
	if (!field->is_map())
	{
		for (int index = 0; index < nFldSize; ++index)
		{
			tbl[index + 1] = GetRepeatedFieldElement(message, field, index);
		}

		return tbl;
	}

	// map
	if (field->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE)
	{
		return tbl;
	}

	for (int i = 0; i < nFldSize; ++i)
	{
		const google::protobuf::Message& entryMsg = pReflection->GetRepeatedMessage(message, field, i);
		LuaIntf::LuaRef entryTbl = MessageToTbl(message);
		const LuaIntf::LuaRef& key = entryTbl["key"];
		const LuaIntf::LuaRef& value = entryTbl["value"];
		tbl[key] = value;
	}

	return tbl;
}

LuaIntf::LuaRef LuaPBModule::GetRepeatedFieldElement(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field, int index) const
{
	if (!field->is_repeated())
	{
		return LuaIntf::LuaRef();
	}

	if (index < 0)
	{
		return LuaIntf::LuaRef();
	}

	const google::protobuf::Reflection* pReflection = message.GetReflection();
	if (nullptr == pReflection)
	{
		return LuaIntf::LuaRef();
	}

	if (index >= pReflection->FieldSize(message, field))
	{
		return LuaIntf::LuaRef();
	}

	google::protobuf::FieldDescriptor::CppType eCppType = field->cpp_type();
	switch (eCppType)
	{
	case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetRepeatedInt32(message, field, index));
	case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetRepeatedInt64(message, field, index));
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetRepeatedUInt32(message, field, index));
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetRepeatedUInt64(message, field, index));
	case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetRepeatedDouble(message, field, index));
	case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetRepeatedFloat(message, field, index));
	case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetRepeatedBool(message, field, index));
	case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetRepeatedEnumValue(message, field, index));
	case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
		return LuaIntf::LuaRefValue(m_pLuaState, pReflection->GetRepeatedString(message, field, index));
	case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
	{
		const google::protobuf::Message& subMsg = pReflection->GetRepeatedMessage(message, field, index);
		return MessageToTbl(subMsg);
	}
	default:
		break;
	}

	return LuaIntf::LuaRef();
}

const bool LuaPBModule::TblToMessage(const LuaIntf::LuaRef& luaTable, google::protobuf::Message& message)
{
	if (!luaTable.isTable())
	{
		return false;
	}

	const auto itrEnd = luaTable.end();
	for (auto itr = luaTable.begin(); itr != itrEnd; ++itr)
	{
		const LuaIntf::LuaRef& key = itr.key();
		if (LuaIntf::LuaTypeID::STRING != key.type())
		{
			continue;
		}

		const std::string& sKey = key.toValue<std::string>();	
		const LuaIntf::LuaRef& val = itr.value();

		if (!val.isTable())
		{
			//std::cout << sKey << " = " << val.toValue<std::string>() << std::endl;
		}

		SetField(message, sKey, val);
	}

	return true;
}

void LuaPBModule::SetField(google::protobuf::Message& message, const std::string & sField, const LuaIntf::LuaRef & luaValue)
{
	const google::protobuf::FieldDescriptor* pField = message.GetDescriptor()->FindFieldByName(sField);
	if (!pField)
	{
		return;
	}

	if (pField->is_repeated())
	{
		SetRepeatedField(message, pField, luaValue);
		return;
	}

	const google::protobuf::Reflection* pReflection = message.GetReflection();
	if (nullptr == pReflection)
	{
		return;
	}

	google::protobuf::FieldDescriptor::CppType eCppType = pField->cpp_type();
	switch (eCppType)
	{
	case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
		pReflection->SetInt32(&message, pField, luaValue.toValue<int32_t>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
		pReflection->SetInt64(&message, pField, luaValue.toValue<int64_t>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
		pReflection->SetUInt32(&message, pField, luaValue.toValue<uint32_t>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
		pReflection->SetUInt64(&message, pField, luaValue.toValue<uint64_t>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
		pReflection->SetDouble(&message, pField, luaValue.toValue<double>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
		pReflection->SetFloat(&message, pField, luaValue.toValue<float>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
		pReflection->SetBool(&message, pField, luaValue.toValue<bool>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
		// Support enum name.
		pReflection->SetEnumValue(&message, pField, GetEnumValue(message, luaValue, pField));
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
		pReflection->SetString(&message, pField, luaValue.toValue<std::string>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
		if (luaValue.isTable())
		{
			google::protobuf::Message* pSubMsg = pReflection->MutableMessage(&message, pField);
			if (pSubMsg)
			{
				TblToMessage(luaValue, *pSubMsg);
			}
		}
	default:
		break;
	}
}

void LuaPBModule::SetRepeatedField(google::protobuf::Message& messag, const google::protobuf::FieldDescriptor * field, const LuaIntf::LuaRef & luaTable)
{
	if (!field->is_repeated())
	{
		return;
	}

	if (!luaTable.isTable())
	{
		return;
	}

	if (field->is_map())
	{
		SetRepeatedMapField(messag, field, luaTable);
		return;
	}

	// non-map
	int len = luaTable.len();
	for (int i = 1; i <= len; ++i)
	{
		const LuaIntf::LuaRef& val = luaTable[i];
		AddToRepeatedField(messag, field, val);
	}
}

void LuaPBModule::SetRepeatedMapField(google::protobuf::Message& messag, const google::protobuf::FieldDescriptor * field, const LuaIntf::LuaRef & luaTable)
{
	if (!field->is_repeated())
	{
		return;
	}

	if (!field->is_map())
	{
		return;
	}

	if (!luaTable.isTable())
	{
		return;
	}

	const auto itrEnd = luaTable.end();
	for (auto itr = luaTable.begin(); itr != itrEnd; ++itr)
	{
		const LuaIntf::LuaRef& key = itr.key();
		const LuaIntf::LuaRef& val = itr.value();
		AddToMapField(messag, field, key, val);
	}
}

void LuaPBModule::AddToRepeatedField(google::protobuf::Message& message, const google::protobuf::FieldDescriptor * field, const LuaIntf::LuaRef & luaValue)
{
	if (!field->is_repeated())
	{
		return;
	}

	if (field->is_map())
	{
		return;
	}

	const google::protobuf::Reflection* pReflection = message.GetReflection();
	if (nullptr == pReflection)
	{
		return;
	}

	google::protobuf::FieldDescriptor::CppType eCppType = field->cpp_type();
	switch (eCppType)
	{
	case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
		pReflection->AddInt32(&message, field, luaValue.toValue<int32_t>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
		pReflection->AddInt64(&message, field, luaValue.toValue<int64_t>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
		pReflection->AddUInt32(&message, field, luaValue.toValue<uint32_t>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
		pReflection->AddUInt64(&message, field, luaValue.toValue<uint64_t>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
		pReflection->AddDouble(&message, field, luaValue.toValue<double>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
		pReflection->AddFloat(&message, field, luaValue.toValue<float>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
		pReflection->AddBool(&message, field, luaValue.toValue<bool>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
		// Support enum name.
		pReflection->AddEnumValue(&message, field, GetEnumValue(message, luaValue, field));
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
		pReflection->AddString(&message, field, luaValue.toValue<std::string>());
		break;
	case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
		if (luaValue.isTable())
		{
			google::protobuf::Message* pSubMsg = pReflection->AddMessage(&message, field);
			if (pSubMsg)
			{
				TblToMessage(luaValue, *pSubMsg);
			}
		}
		break;
	default:
		break;
	}
}

void LuaPBModule::AddToMapField(google::protobuf::Message& message, const google::protobuf::FieldDescriptor * field, const LuaIntf::LuaRef & key, const LuaIntf::LuaRef & val)
{
	if (!field->is_repeated())
	{
		return;
	}

	if (!field->is_map())
	{
		return;
	}

	if (field->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE)
	{
		return;
	}

	const google::protobuf::Reflection* pReflection = message.GetReflection();
	if (nullptr == pReflection)
	{
		return;
	}

	google::protobuf::Message* pMapEntry = pReflection->AddMessage(&message, field);
	if (!pMapEntry)
	{
		return;
	}

	const google::protobuf::Reflection* pMapReflection = pMapEntry->GetReflection();
	if (nullptr == pMapReflection)
	{
		return;
	}

	SetField(*pMapEntry, key.toValue<std::string>(), val);
}

int LuaPBModule::GetEnumValue(google::protobuf::Message& messag, const LuaIntf::LuaRef & luaValue, const google::protobuf::FieldDescriptor * field) const
{
	if (luaValue.type() != LuaIntf::LuaTypeID::STRING)
	{
		return luaValue.toValue<int>();
	}

	std::string sEnum = luaValue.toValue<string>();
	if (field->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_ENUM)
	{
		return 0;
	}

	const google::protobuf::EnumDescriptor* pEnum = field->enum_type();
	if (!pEnum)
	{
		return 0;
	}

	const google::protobuf::EnumValueDescriptor* pEnumVal = pEnum->FindValueByName(sEnum);
	if (pEnumVal)
	{
		return pEnumVal->number();
	}

	return luaValue.toValue<int>();  // "123" -> 123
}
