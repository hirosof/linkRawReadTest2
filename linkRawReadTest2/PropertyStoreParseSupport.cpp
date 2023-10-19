#include "PropertyStoreParseSupport.hpp"


HRESULT hirosof::Supports::PropertyStoreParse::ParsePropertyStore( PropertyStoreData* pData, void* pRawData, uint32_t rawDataSize ) {

    if ( pData == nullptr )  return E_POINTER;
    
    if ( pRawData == nullptr ) return E_POINTER;

    if ( rawDataSize < 4 ) return E_FAIL;
    
    uint8_t* pBytesData = reinterpret_cast<uint8_t*>( pRawData );

    pData->pStoreTop = pRawData;
    pData->storeSize = rawDataSize;
    pData->storageData.clear( );

    PropertyStorageData storageData;
    uint32_t offset = 0;
    HRESULT hr;

    while ( offset  <= rawDataSize  ) {

        hr = ParsePropertyStorage( &storageData, pBytesData + offset );

        if ( hr == S_OK ) {
            offset += storageData.storageSize;
            pData->storageData.push_back( storageData );
        } else if ( hr == S_FALSE ) {
            break;
        } else {
            return E_FAIL;
        }
    }

    return (pData->storageData.empty() )? S_FALSE : S_OK;
}

HRESULT hirosof::Supports::PropertyStoreParse::ParsePropertyStorage( PropertyStorageData* pData, void* pRawData ) {

    if ( pData == nullptr )  return E_POINTER;

    if ( pRawData == nullptr ) return E_POINTER;

    uint8_t* pBytesData = reinterpret_cast<uint8_t*>( pRawData );
    uint32_t storageSize = *reinterpret_cast<uint32_t*>( pBytesData );


    if ( storageSize == 0 ) return S_FALSE;

    uint32_t* pVersion = reinterpret_cast<uint32_t*>( pBytesData + sizeof( uint32_t ) );
    GUID* pFormatID = reinterpret_cast<GUID*>( pVersion + 1 );
    uint8_t* pValueTop = reinterpret_cast<uint8_t*>( pFormatID + 1 );

    if ( *pVersion != 0x53505331 ) return E_FAIL;
    

    PropertyValueNameFormatType  formatType = PropertyValueNameFormatType::String;

    if ( pFormatID->Data1 != 0xd5cdd505 ) formatType = PropertyValueNameFormatType::Integer;
    if (( formatType == PropertyValueNameFormatType::String) && ( pFormatID->Data2 != 0x2e9c ))  formatType = PropertyValueNameFormatType::Integer;
    if ( ( formatType == PropertyValueNameFormatType::String ) && ( pFormatID->Data3 != 0x101b )) formatType = PropertyValueNameFormatType::Integer;
    if ( ( formatType == PropertyValueNameFormatType::String ) && ( *reinterpret_cast<uint64_t*>( pFormatID->Data4 ) != 0xaef92c2b00089793 )) {
        formatType = PropertyValueNameFormatType::Integer;
    }



    pData->pStorageTop = pRawData;
    pData->storageSize = storageSize;
    pData->version = *pVersion;
    pData->formatID = *pFormatID;
    pData->formatType = formatType;

    uint32_t offset = 0;
    //HRESULT hr;


    if ( formatType == PropertyValueNameFormatType::String ) {

    } else if ( formatType == PropertyValueNameFormatType::Integer ) {

    } else {
        return E_FAIL;
    }
    


    return S_OK;
}
