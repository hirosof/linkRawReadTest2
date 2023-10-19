#pragma once
#include <Windows.h>
#include <cstdint>
#include <vector>
namespace hirosof {


	namespace Supports {

		namespace PropertyStoreParse{


			struct TypedPropertyValue {
				uint16_t formatType;
				void* pValue;
			};


			struct  IntergerNamedPropertyValue {
				uint32_t valueSize;
				uint32_t id;
				TypedPropertyValue typedValue;
			};


			struct StringNamedPropertyValue {
				uint32_t valueSize;
				uint32_t nameSize;
				wchar_t* pName;
				TypedPropertyValue typedValue;
			};



			enum struct PropertyValueNameFormatType {
				Integer = 0,
				String
			};

			struct PropertyStorageData {
				void* pStorageTop;
				uint32_t storageSize;
				uint32_t version;
				GUID formatID;
				PropertyValueNameFormatType formatType;
				std::vector<StringNamedPropertyValue> stringNamedValues;
				std::vector<IntergerNamedPropertyValue> integerNamedValues;
			};



#pragma warning(push)
#pragma warning(disable:26495)
			struct PropertyStoreData {
				void* pStoreTop;
				uint32_t storeSize;
				std::vector<PropertyStorageData> storageData;
			};
#pragma warning(pop)







			HRESULT ParsePropertyStore( PropertyStoreData* pData, void* pRawData, uint32_t rawDataSize );
			HRESULT ParsePropertyStorage( PropertyStorageData* pData, void* pRawData );


		}

	}

}