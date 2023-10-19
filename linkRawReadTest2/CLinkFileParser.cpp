#include "CLinkFileParser.hpp"

hirosof::LinkFile::CLinkFileParser::CLinkFileParser( ) {

	this->Reset( );

}

hirosof::LinkFile::CLinkFileParser::~CLinkFileParser( ) {
}

void hirosof::LinkFile::CLinkFileParser::Reset( void ) {

	if ( this->m_rawData.m_pData != nullptr ) this->m_rawData.Free( );
	this->m_Parsed = false;
	this->m_IDList.IDListSize = 0;
	this->m_IDList.pIDList = nullptr;
	this->m_LinkInfo.pLinkInfo = nullptr;
	m_rawDataSize = 0;


	m_sd_Name_String.pRawData = nullptr;
	m_sd_Relative_Path.pRawData = nullptr;
	m_sd_Working_DIR.pRawData = nullptr;
	m_sd_Command_Line_Arguments.pRawData = nullptr;
	m_sd_Icon_Location.pRawData = nullptr;
	m_hasStringData = false;
	m_hasExtraData = false;

	this->m_extra.mp_ConsoleDataBlock = nullptr;
	this->m_extra.mp_ConsoleFEDataBlock = nullptr;
	this->m_extra.mp_DarwinDataBlock = nullptr;
	this->m_extra.mp_EnvironmentVariableDataBlock = nullptr;
	this->m_extra.mp_IconEnvironmentDataBlock = nullptr;
	this->m_extra.mp_KnownFolderDataBlock = nullptr;
	this->m_extra.m_PropertyStoreDataBlock.pHeader = nullptr;
	this->m_extra.m_PropertyStoreDataBlock.pData = nullptr;

	this->m_extra.mp_ShimDataBlock = nullptr;
	this->m_extra.mp_SpecialFolderDataBlock = nullptr;
	this->m_extra.mp_TrackerDataBlock = nullptr;
	this->m_extra.m_VistaAndAboveIDListDataBlock.pHeader = nullptr;
	this->m_extra.m_VistaAndAboveIDListDataBlock.pIDList = nullptr;
	this->m_extra.m_VistaAndAboveIDListDataBlock.IDListSize = 0;

	this->m_unclassified_and_unparsed_extra.clear( );
}

HRESULT hirosof::LinkFile::CLinkFileParser::Load( const wchar_t* pFilePath ) {


	if ( pFilePath == nullptr ) {
		return E_POINTER;
	}

	IStream *pStream;
	HRESULT hr;

	hr = SHCreateStreamOnFileW( pFilePath, STGM_READ, &pStream );

	if ( hr != S_OK ) {
		return hr;
	}


	hr = this->Load( pStream );

	pStream->Release( );

	return hr;
}

HRESULT hirosof::LinkFile::CLinkFileParser::Load( const char* pFilePath ) {
	if ( pFilePath == nullptr ) {
		return E_POINTER;
	}


	IStream *pStream;
	HRESULT hr;

	hr = SHCreateStreamOnFileA( pFilePath, STGM_READ, &pStream );

	if ( hr != S_OK ) {
		return hr;
	}


	hr = this->Load( pStream );

	pStream->Release( );
	return hr;
}


HRESULT hirosof::LinkFile::CLinkFileParser::Load(  IStream* pStream ) {
	HRESULT hr;
	STATSTG stat;

	if ( pStream == nullptr ) {
		return E_POINTER;
	}


	LARGE_INTEGER li;
	li.QuadPart = 0;
	hr = pStream->Seek( li, STREAM_SEEK_SET, NULL );
	if ( hr != S_OK ) {
		return hr;
	}

	hr = pStream->Stat( &stat, STATFLAG_NONAME );
	if ( hr != S_OK ) {
		return hr;
	}


	if ( stat.cbSize.QuadPart < sizeof( ShellLinkHeader ) ) {
		return E_FAIL;
	}


	ShellLinkHeader header;
	hr = pStream->Read( &header, sizeof( ShellLinkHeader ), nullptr );
	if ( hr != S_OK ) {
		return E_FAIL;
	}

	if ( header.HeaderSize != 0x0000004C ) {
		return E_FAIL;
	}


	if ( header.LinkCLSID.Data1 != 0x00021401 ) {

		return E_FAIL;

	}
	if ( header.LinkCLSID.Data2 != 0 ) {

		return E_FAIL;

	}
	if ( header.LinkCLSID.Data3 != 0 ) {
		return E_FAIL;
	}

	if ( *reinterpret_cast<uint64_t*>( header.LinkCLSID.Data4 ) != 0x46000000000000c0 ) {
		return E_FAIL;
	}


	CHeapPtr<uint8_t> data;
	if ( data.Allocate( static_cast<size_t>( stat.cbSize.QuadPart ) ) == false ) {
		return E_FAIL;
	}


	memcpy( data.m_pData, &header, sizeof( ShellLinkHeader ) );

	hr = pStream->Read( data.m_pData + sizeof( ShellLinkHeader ), static_cast<ULONG>( stat.cbSize.QuadPart - sizeof( ShellLinkHeader ) ), nullptr );
	if ( hr != S_OK ) {
		return E_FAIL;
	}



	this->Reset( );
	
	this->m_rawData = data;
	this->m_rawDataSize = stat.cbSize.QuadPart;

	return S_OK;

}

HRESULT hirosof::LinkFile::CLinkFileParser::Parse( void ) {
	if ( this->m_rawData == nullptr ) {
		return E_FAIL;
	}

	uint8_t* pCurrent = this->m_rawData.m_pData + sizeof( ShellLinkHeader );

	ShellLinkHeader* pHeader = reinterpret_cast<ShellLinkHeader*>( this->m_rawData.m_pData );

	if ( pHeader->LinkFlags.item.HasLinkTargetIDList ) {
		this->m_IDList.IDListSize = *reinterpret_cast<uint16_t*>( pCurrent );
		this->m_IDList.pIDList = reinterpret_cast<ITEMIDLIST*>( pCurrent + sizeof( uint16_t ) );
		pCurrent += sizeof( uint16_t ) + this->m_IDList.IDListSize;
	}

	if ( pHeader->LinkFlags.item.HasLinkInfo ) {

		LinkInfoExtra lie;
		lie.pLinkInfo = reinterpret_cast<LinkInfo*>( pCurrent );

		HRESULT hr = this->ParseLinkInfo( &lie );

		this->m_LinkInfo = lie;

		pCurrent += lie.pLinkInfo->LinkInfoSize;
	}


	ELinkFlagsBitFieldItems sd_names[] = {
		ELinkFlagsBitFieldItems::HasName ,
		ELinkFlagsBitFieldItems::HasRelativePath ,
		ELinkFlagsBitFieldItems::HasWorkingDir,
		ELinkFlagsBitFieldItems::HasArguments ,
		ELinkFlagsBitFieldItems::HasIconLocation
	};

	StringData* pDataArray[] = {
		&m_sd_Name_String,
		&m_sd_Relative_Path,
		&m_sd_Working_DIR,
		&m_sd_Command_Line_Arguments,
		&m_sd_Icon_Location,
	};

	for ( StringData* pdata : pDataArray ) {
		pdata->pRawData = nullptr;
		pdata->CountCharacters = 0;
		pdata->bUnicode = pHeader->LinkFlags.item.IsUnicode;
		pdata->String.clear( );
		pdata->StringUnicode.clear( );
	}

	m_hasStringData = false;

	for ( size_t i = 0; i < ( sizeof( sd_names ) / sizeof( ELinkFlagsBitFieldItems ) ); i++ ) {

		ELinkFlagsBitFieldItems name = sd_names[i];

		if ( pHeader->LinkFlags.raw & ( 1 << (int) name ) ) {
			m_hasStringData = true;

			pDataArray[i]->pRawData = pCurrent;
			pDataArray[i]->CountCharacters = *reinterpret_cast<uint16_t*>( pDataArray[i]->pRawData );


			void* pBody = pCurrent + sizeof( uint16_t );


			pCurrent += sizeof( uint16_t );

			if ( pDataArray[i]->CountCharacters != 0 ) {

				if ( pHeader->LinkFlags.item.IsUnicode ) {
					pDataArray[i]->StringUnicode.append( reinterpret_cast<wchar_t*>( pBody ), pDataArray[i]->CountCharacters );
					pCurrent += pDataArray[i]->CountCharacters * sizeof( wchar_t );
				} else {
					pDataArray[i]->String.append( reinterpret_cast<char*>( pBody ), pDataArray[i]->CountCharacters );
					pCurrent += pDataArray[i]->CountCharacters * sizeof( char );
				}
			}
		}
	}


	m_hasExtraData = false;
	while ( 1 ) {

		uint32_t  u32value = *reinterpret_cast<uint32_t*>( pCurrent );
		if ( u32value < 4 ) break;
		this->ClassifyExtraData( reinterpret_cast<ExtraData_General*>( pCurrent ) );
		this->m_unclassified_and_unparsed_extra.push_back( reinterpret_cast<ExtraData_General*>( pCurrent ) );
		pCurrent += u32value;
	}

	this->m_Parsed = true;
	return S_OK;
}


const hirosof::LinkFile::ShellLinkHeader* hirosof::LinkFile::CLinkFileParser::getHeaderPtr( void ) const {

	if ( this->m_rawData == nullptr ) {
		return nullptr;
	}

	return  reinterpret_cast<ShellLinkHeader*>( this->m_rawData.m_pData );
}

bool hirosof::LinkFile::CLinkFileParser::IsParsed( void ) const {
	return this->m_Parsed;
}

bool hirosof::LinkFile::CLinkFileParser::GetLinkTargetIDList( LinkTargetIDList* pIDList ) const {
	if ( pIDList != nullptr ) {
		if ( this->m_Parsed ) {
			*pIDList = this->m_IDList;
			return true;
		}
	}
	return false;
}

const hirosof::LinkFile::LinkTargetIDList* hirosof::LinkFile::CLinkFileParser::GetLinkTargetIDListPtr( void ) const {
	return  ( this->m_Parsed ) ? &this->m_IDList : nullptr;
}

const ITEMIDLIST* hirosof::LinkFile::CLinkFileParser::GetTargetItemIDList( void ) const{
	return  ( this->m_Parsed ) ? this->m_IDList.pIDList : nullptr; 
}

const  hirosof::LinkFile::LinkInfoExtra* hirosof::LinkFile::CLinkFileParser::GetLinkInfo( void ) const {
	return  ( this->m_Parsed && ( this->m_LinkInfo.pLinkInfo!=nullptr)) ? &this->m_LinkInfo : nullptr;
}

const hirosof::LinkFile::StringData hirosof::LinkFile::CLinkFileParser::getNameString( void ) const {
	return this->m_sd_Name_String;
}

const hirosof::LinkFile::StringData hirosof::LinkFile::CLinkFileParser::getRelativePath( void ) const {
	return this->m_sd_Relative_Path;
}

const hirosof::LinkFile::StringData hirosof::LinkFile::CLinkFileParser::getWorkingDir( void ) const {
	return this->m_sd_Working_DIR;
}

const hirosof::LinkFile::StringData hirosof::LinkFile::CLinkFileParser::getCommandLineArguments( void ) const {
	return this->m_sd_Command_Line_Arguments;
}

const hirosof::LinkFile::StringData hirosof::LinkFile::CLinkFileParser::getIconLocation( void ) const {
	return this->m_sd_Icon_Location;
}

bool hirosof::LinkFile::CLinkFileParser::hasStringData( void ) const {
	return this->m_hasStringData;
}

bool hirosof::LinkFile::CLinkFileParser::hasExtraData( void ) const {
	return this->m_hasExtraData;
}

const hirosof::LinkFile::ExtraData_ConsoleDataBlock* hirosof::LinkFile::CLinkFileParser::getExtraConsoleDataBlockPtr( void ) const {
	return this->m_extra.mp_ConsoleDataBlock;
}

const hirosof::LinkFile::ExtraData_ConsoleFEDataBlock* hirosof::LinkFile::CLinkFileParser::getExtraConsoleFEDataBlockPtr( void ) const {
	return this->m_extra.mp_ConsoleFEDataBlock;
}

const hirosof::LinkFile::ExtraData_DarwinDataBlock* hirosof::LinkFile::CLinkFileParser::getExtraDarwinDataBlockPtr( void ) const {
	return this->m_extra.mp_DarwinDataBlock;
}

const hirosof::LinkFile::ExtraData_EnvironmentVariableDataBlock* hirosof::LinkFile::CLinkFileParser::getExtraEnvironmentVariableDataBlockPtr( void ) const {
	return this->m_extra.mp_EnvironmentVariableDataBlock;
}

const hirosof::LinkFile::ExtraData_IconEnvironmentDataBlock* hirosof::LinkFile::CLinkFileParser::getExtraIconEnvironmentDataBlockPtr( void ) const {
	return this->m_extra.mp_IconEnvironmentDataBlock;
}

const hirosof::LinkFile::ExtraData_KnownFolderDataBlock* hirosof::LinkFile::CLinkFileParser::getExtraKnownFolderDataBlockPtr( void ) const {
	return this->m_extra.mp_KnownFolderDataBlock;
}

const hirosof::LinkFile::ExtraData_PropertyStoreDataBlock hirosof::LinkFile::CLinkFileParser::getExtraPropertyStoreDataBlock( void ) const {
	return this->m_extra.m_PropertyStoreDataBlock;
}

const hirosof::LinkFile::ExtraData_ShimDataBlock* hirosof::LinkFile::CLinkFileParser::getExtraShimDataBlockPtr( void ) const {
	return this->m_extra.mp_ShimDataBlock;
}

const hirosof::LinkFile::ExtraData_SpecialFolderDataBlock* hirosof::LinkFile::CLinkFileParser::getExtraSpecialFolderDataBlockPtr( void ) const {
	return this->m_extra.mp_SpecialFolderDataBlock;
}

const hirosof::LinkFile::ExtraData_TrackerDataBlock* hirosof::LinkFile::CLinkFileParser::getExtraTrackerDataBlockPtr( void ) const {
	return this->m_extra.mp_TrackerDataBlock;
}

const hirosof::LinkFile::ExtraData_VistaAndAboveIDListDataBlock hirosof::LinkFile::CLinkFileParser::getExtraVistaAndAboveIDListDataBlock( void ) const {
	return this->m_extra.m_VistaAndAboveIDListDataBlock;
}

const std::vector<hirosof::LinkFile::ExtraData_General*> hirosof::LinkFile::CLinkFileParser::getUnclassifiedAndUnparsedExtraDataBlocks( void ) const {
	return this->m_unclassified_and_unparsed_extra;
}

HRESULT hirosof::LinkFile::CLinkFileParser::ParseLinkInfo( LinkInfoExtra* plie ) const {

	if ( plie == nullptr ) return E_POINTER;

	uint8_t* pLinkInfoTop = reinterpret_cast<uint8_t*>( plie->pLinkInfo );

	if ( ( plie->pLinkInfo->LinkInfoHeaderSize != 0x1c ) && ( plie->pLinkInfo->LinkInfoHeaderSize < 0x24 ) ) {
		return E_FAIL;
	}

	if ( plie->pLinkInfo->LinkInfoFlags & 1 ) {
		plie->volumeIDExt.pVolumeID = reinterpret_cast<VolumeID*>( pLinkInfoTop + plie->pLinkInfo->VolumeIDOffset );
		plie->pLocalBasePath = reinterpret_cast<char*>( pLinkInfoTop + plie->pLinkInfo->LocalBasePathOffset );
		if ( plie->pLinkInfo->LinkInfoHeaderSize >= 0x24 ) {
			plie->pLocalBasePathUnicode = reinterpret_cast<wchar_t*>( pLinkInfoTop + plie->pLinkInfo->optional.LocalBasePathOffsetUnicode );
		} else {
			plie->pLocalBasePathUnicode = nullptr;
		}
	} else {
		plie->volumeIDExt.pVolumeID = nullptr; 
		plie->pLocalBasePath = nullptr; 
		plie->pLocalBasePathUnicode = nullptr;
	}

	plie->pCommonPathSuffix = reinterpret_cast<char*>( pLinkInfoTop + plie->pLinkInfo->CommonPathSuffixOffset );
	if ( plie->pLinkInfo->LinkInfoHeaderSize >= 0x24 ) {
		plie->pCommonPathSuffixUnicode = reinterpret_cast<wchar_t*>( pLinkInfoTop + plie->pLinkInfo->optional.CommonPathSuffixOffsetUnicode );
	} else {
		plie->pCommonPathSuffixUnicode = nullptr;
	}

	if ( plie->pLinkInfo->LinkInfoFlags & 2 ) {
		plie->commonNetworkRelativeLinkExt.pCommonNetworkRelativeLink = reinterpret_cast<CommonNetworkRelativeLink*>( pLinkInfoTop + plie->pLinkInfo->CommonNetworkRelativeLinkOffset );
	} else {
		plie->commonNetworkRelativeLinkExt.pCommonNetworkRelativeLink = nullptr;
	}


	this->ParseVolumeID( &plie->volumeIDExt );
	this->ParseCommonNetworkRelativeLink( &plie->commonNetworkRelativeLinkExt );

	return S_OK;
}

HRESULT hirosof::LinkFile::CLinkFileParser::ParseVolumeID( VolumeIDExtra* pvide ) const {
	if ( pvide == nullptr ) return E_POINTER;

	pvide->pVolumeLabel = nullptr;
	pvide->pVolumeLabelUnicode = nullptr;

	if ( pvide->pVolumeID == nullptr ) {
		return S_FALSE;
	}


	uint8_t* pVolumeIDTop = reinterpret_cast<uint8_t*>( pvide->pVolumeID);


	if ( pvide->pVolumeID->VolumeIDSize <= 0x10 ) {
		return E_FAIL;
	}


	if ( pvide->pVolumeID->VolumeLabelOffset == 0x00000014 ) {
		pvide->pVolumeLabelUnicode = reinterpret_cast<wchar_t*>( pVolumeIDTop + pvide->pVolumeID->optional.VolumeLabelOffsetUnicode );
	} else {
		pvide->pVolumeLabel = reinterpret_cast<char*>( pVolumeIDTop + pvide->pVolumeID->VolumeLabelOffset );
	}


	return S_OK;
}

HRESULT hirosof::LinkFile::CLinkFileParser::ParseCommonNetworkRelativeLink( CommonNetworkRelativeLinkExtra* pcnrle ) const {
	if ( pcnrle == nullptr ) return E_POINTER;

	CommonNetworkRelativeLink* pLink = pcnrle->pCommonNetworkRelativeLink;
	uint8_t* pCommonNetworkRelativeLinkTop = reinterpret_cast<uint8_t*>( pLink );

	pcnrle->pDeviceName = nullptr;
	pcnrle->pDeviceNameUnicode = nullptr;
	pcnrle->pNetName = nullptr;
	pcnrle->pNetNameUnicode = nullptr;

	if ( pLink == nullptr ) {

		return S_FALSE;
	}


	if ( pLink->CommonNetworkRelativeLinkSize < 0x00000014 ) {
		return E_FAIL;
	}

	pcnrle->pNetName = reinterpret_cast<char*>( pCommonNetworkRelativeLinkTop + pLink->NetNameOffset );

	if ( pLink->NetNameOffset > 0x00000014 ) {
		pcnrle->pNetNameUnicode = reinterpret_cast<wchar_t*>( pCommonNetworkRelativeLinkTop + pLink->optional.NetNameOffsetUnicode );
	}


	if ( pLink->CommonNetworkRelativeLinkFlags & 1 ) {
		pcnrle->pDeviceName = reinterpret_cast<char*>( pCommonNetworkRelativeLinkTop + pLink->DeviceNameOffset );
		if ( pLink->NetNameOffset > 0x00000014 ) {
			pcnrle->pDeviceNameUnicode = reinterpret_cast<wchar_t*>( pCommonNetworkRelativeLinkTop + pLink->optional.DeviceNameOffsetUnicode );
		}
	}

	return S_OK;
}

HRESULT hirosof::LinkFile::CLinkFileParser::ClassifyExtraData( ExtraData_General* pData ) {

	if ( pData == nullptr ) return E_POINTER;

	uint32_t dataBlockSize = pData->BlockSize;

	switch ( pData->BlockSignature ) {
		case ExtraData_ConsoleDataBlockSignature:
			if ( dataBlockSize == ExtraData_ConsoleDataBlockSize ) this->m_extra.mp_ConsoleDataBlock = reinterpret_cast<ExtraData_ConsoleDataBlock*>( pData );
			else return E_FAIL;
			break;
		case ExtraData_ConsoleFEDataBlockSignature:
			if ( dataBlockSize == ExtraData_ConsoleFEDataBlockSize ) this->m_extra.mp_ConsoleFEDataBlock = reinterpret_cast<ExtraData_ConsoleFEDataBlock*>( pData );
			else return E_FAIL;
			break;
		case ExtraData_DarwinDataBlockSignature:
			if ( dataBlockSize == ExtraData_DarwinDataBlockSize ) this->m_extra.mp_DarwinDataBlock = reinterpret_cast<ExtraData_DarwinDataBlock*>( pData );
			else return E_FAIL;
			break;
		case ExtraData_EnvironmentVariableDataBlockSignature:
			if ( dataBlockSize == ExtraData_EnvironmentVariableDataBlockSize ) this->m_extra.mp_EnvironmentVariableDataBlock = reinterpret_cast<ExtraData_EnvironmentVariableDataBlock*>( pData );
			else return E_FAIL;
			break;
		case ExtraData_IconEnvironmentDataBlockSignature:
			if ( dataBlockSize == ExtraData_IconEnvironmentDataBlockSize ) this->m_extra.mp_IconEnvironmentDataBlock = reinterpret_cast<ExtraData_IconEnvironmentDataBlock*>( pData );
			else return E_FAIL;
			break;
		case ExtraData_KnownFolderDataBlockSignature:
			if ( dataBlockSize == ExtraData_KnownFolderDataBlockSize ) this->m_extra.mp_KnownFolderDataBlock = reinterpret_cast<ExtraData_KnownFolderDataBlock*>( pData );
			else return E_FAIL;
			break;
		case ExtraData_PropertyStoreDataBlockSignature:
			if ( dataBlockSize >= ExtraData_PropertyStoreDataBlockMinSize ) {
				this->m_extra.m_PropertyStoreDataBlock.pHeader = reinterpret_cast<ExtraData_PropertyStoreDataBlockHeader*>( pData );
				this->m_extra.m_PropertyStoreDataBlock.pData = reinterpret_cast<uint8_t*>( pData ) + sizeof( ExtraData_PropertyStoreDataBlockHeader );
				this->m_extra.m_PropertyStoreDataBlock.dataSize = this->m_extra.m_PropertyStoreDataBlock.pHeader->BlockSize - sizeof( ExtraData_PropertyStoreDataBlockHeader );
			}
			else return E_FAIL;
			break;
		case ExtraData_ShimDataBlockSignature:
			if ( dataBlockSize == 0 ) this->m_extra.mp_ShimDataBlock = reinterpret_cast<ExtraData_ShimDataBlock*>( pData );
			else return E_FAIL;
			break;
		case ExtraData_SpecialFolderDataBlockSignature:
			if ( dataBlockSize == ExtraData_SpecialFolderDataBlockSize) this->m_extra.mp_SpecialFolderDataBlock = reinterpret_cast<ExtraData_SpecialFolderDataBlock*>( pData );
			else return E_FAIL;
			break;
		case ExtraData_TrackerDataBlockSignature:
			if ( dataBlockSize == ExtraData_TrackerDataBlockSize ) this->m_extra.mp_TrackerDataBlock = reinterpret_cast<ExtraData_TrackerDataBlock*>( pData );
			else return E_FAIL;
			break;
		case ExtraData_VistaAndAboveIDListDataBlockSignature:
			if ( dataBlockSize >= ExtraData_VistaAndAboveIDListDataBlockMinSize ) {
				this->m_extra.m_VistaAndAboveIDListDataBlock.pHeader = reinterpret_cast<ExtraData_VistaAndAboveIDListDataBlockHeader*>( pData );
				this->m_extra.m_VistaAndAboveIDListDataBlock.IDListSize = this->m_extra.m_VistaAndAboveIDListDataBlock.pHeader->BlockSize - sizeof( ExtraData_VistaAndAboveIDListDataBlockHeader );
				void* pIDList = reinterpret_cast<uint8_t*>( pData ) + sizeof( ExtraData_VistaAndAboveIDListDataBlockHeader );
				this->m_extra.m_VistaAndAboveIDListDataBlock.pIDList = reinterpret_cast<ITEMIDLIST*>( pIDList );
			}
			else return E_FAIL;
		break;
		default:
			return E_NOTIMPL;
	}

	m_hasExtraData = true;
	return S_OK;
}
