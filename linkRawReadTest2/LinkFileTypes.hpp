#pragma once

#include <Windows.h>
#include <stdint.h>
#include <memory>
#include <string>

namespace hirosof {
	namespace  LinkFile {

		template<typename baseType, typename itemType>  union ULinkFileGeneralItemType {
			baseType raw;
			itemType item;
		};


		template<typename itemType>  using ULinkGeneralPointerItemType = ULinkFileGeneralItemType<uint8_t*, itemType*>;


		template<typename itemType>  using ULinkFileBitFieldUint32 = ULinkFileGeneralItemType<uint32_t, itemType>;


		//
		// ShellLinkHeader
		//

		struct  LinkFlagsBitField {
			bool HasLinkTargetIDList : 1;
			bool HasLinkInfo : 1;
			bool HasName : 1;
			bool HasRelativePath : 1;
			bool HasWorkingDir : 1;
			bool HasArguments : 1;
			bool HasIconLocation : 1;
			bool IsUnicode : 1;
			bool ForceNoLinkInfo : 1;
			bool HasExpString : 1;
			bool RunInSeparateProcess : 1;
			bool Unused1 : 1;
			bool HasDarwinID : 1;
			bool RunAsUser : 1;
			bool HasExpIcon : 1;
			bool NoPidlAlias : 1;
			bool Unused2 : 1;
			bool RunWithShimLayer : 1;
			bool ForceNoLinkTrack : 1;
			bool EnableTargetMetadata : 1;
			bool DisableLinkPathTracking : 1;
			bool DisableKnownFolderTracking : 1;
			bool DisableKnownFolderAlias : 1;
			bool AllowLinkToLink : 1;
			bool UnaliasOnSave : 1;
			bool PreferEnvironmentPath : 1;
			bool AKeepLocalIDListForUNCTarget : 1;
		};


		enum struct ELinkFlagsBitFieldItems {
			HasLinkTargetIDList = 0,
			HasLinkInfo,
			HasName,
			HasRelativePath,
			HasWorkingDir,
			HasArguments,
			HasIconLocation,
			IsUnicode,
			ForceNoLinkInfo,
			HasExpString,
			RunInSeparateProcess,
			Unused1,
			HasDarwinID,
			RunAsUser,
			HasExpIcon,
			NoPidlAlias,
			Unused2,
			RunWithShimLayer,
			ForceNoLinkTrack,
			EnableTargetMetadata,
			DisableLinkPathTracking,
			DisableKnownFolderTracking,
			DisableKnownFolderAlias,
			AllowLinkToLink,
			UnaliasOnSave,
			PreferEnvironmentPath,
			KeepLocalIDListForUNCTarget
		};

		const char  LinkFlagsBitFieldItemNameStrings[][32] = {
			"HasLinkTargetIDList",
			"HasLinkInfo",
			"HasName",
			"HasRelativePath",
			"HasWorkingDir",
			"HasArguments",
			"HasIconLocation",
			"IsUnicode",
			"ForceNoLinkInfo",
			"HasExpString",
			"RunInSeparateProcess",
			"Unused1",
			"HasDarwinID",
			"RunAsUser",
			"HasExpIcon",
			"NoPidlAlias",
			"Unused2",
			"RunWithShimLayer",
			"ForceNoLinkTrack",
			"EnableTargetMetadata",
			"DisableLinkPathTracking",
			"DisableKnownFolderTracking",
			"DisableKnownFolderAlias",
			"AllowLinkToLink",
			"UnaliasOnSave",
			"PreferEnvironmentPath",
			"KeepLocalIDListForUNCTarget"
		};



		struct  LinkFileAttributesFlagsBitField {
			bool READONLY : 1;
			bool HIDDEN : 1;
			bool SYSTEM : 1;
			bool Reserved1 : 1;
			bool DIRECTORY : 1;
			bool ARCHIVE : 1;
			bool Reserved2 : 1;
			bool NORMAL : 1;
			bool TEMPORARY : 1;
			bool SPARSE_FILE : 1;
			bool REPARSE_POINT : 1;
			bool COMPRESSED : 1;
			bool OFFLINE : 1;
			bool NOT_CONTENT_INDEXED : 1;
			bool ENCRYPTED : 1;
		};

		enum struct LinkFileAttributesFlagsBitFieldItems {
			READONLY = 0,
			HIDDEN,
			SYSTEM,
			Reserved1,
			DIRECTORY,
			ARCHIVE,
			Reserved2,
			NORMAL,
			TEMPORARY,
			SPARSE_FILE,
			REPARSE_POINT,
			COMPRESSED,
			OFFLINE,
			NOT_CONTENT_INDEXED,
			ENCRYPTED
		};

		const char  LinkFileAttributesFlagsBitFieldItemNameStrings[][35] = {
			"FILE_ATTRIBUTE_READONLY",
			"FILE_ATTRIBUTE_HIDDEN",
			"FILE_ATTRIBUTE_SYSTEM",
			"Reserved1",
			"FILE_ATTRIBUTE_DIRECTORY",
			"FILE_ATTRIBUTE_ARCHIVE",
			"Reserved2",
			"FILE_ATTRIBUTE_NORMAL",
			"FILE_ATTRIBUTE_TEMPORARY",
			"FILE_ATTRIBUTE_SPARSE_FILE",
			"FILE_ATTRIBUTE_REPARSE_POINT",
			"FILE_ATTRIBUTE_COMPRESSED",
			"FILE_ATTRIBUTE_OFFLINE",
			"FILE_ATTRIBUTE_NOT_CONTENT_INDEXED",
			"FILE_ATTRIBUTE_ENCRYPTED"
		};

		struct ShellLinkHeader {
			uint32_t  HeaderSize;
			CLSID LinkCLSID;
			ULinkFileBitFieldUint32<LinkFlagsBitField> LinkFlags;
			ULinkFileBitFieldUint32<LinkFileAttributesFlagsBitField> FileAttributes;
			FILETIME CreationTime;
			FILETIME AccessTime;
			FILETIME WriteTime;
			uint32_t FileSize;
			int32_t IconIndex;
			uint32_t ShowCommand;
			uint16_t HotKey;
			uint16_t Reserved1;
			uint32_t Reserved2;
			uint32_t Reserved3;
		};

		//
		// LinkTargetIDList
		//

		struct LinkTargetIDList {
			uint16_t IDListSize;
			ITEMIDLIST *pIDList;
		};


		//
		// LinkInfo
		//

		struct LinkInfo {
			uint32_t  LinkInfoSize;
			uint32_t  LinkInfoHeaderSize;
			uint32_t LinkInfoFlags;
			uint32_t VolumeIDOffset;
			uint32_t LocalBasePathOffset;
			uint32_t CommonNetworkRelativeLinkOffset;
			uint32_t CommonPathSuffixOffset;
			struct {
				uint32_t LocalBasePathOffsetUnicode;
				uint32_t CommonPathSuffixOffsetUnicode;
			}optional;
		};

		struct VolumeID {
			uint32_t VolumeIDSize;
			uint32_t DriveType;
			uint32_t DriveSerialNumber;
			uint32_t VolumeLabelOffset;
			struct {
				uint32_t VolumeLabelOffsetUnicode;
			}optional;
		};

		struct CommonNetworkRelativeLink {
			uint32_t CommonNetworkRelativeLinkSize;
			uint32_t CommonNetworkRelativeLinkFlags;
			uint32_t NetNameOffset;
			uint32_t DeviceNameOffset;
			uint32_t NetworkProviderType;
			struct {
				uint32_t NetNameOffsetUnicode;
				uint32_t DeviceNameOffsetUnicode;
			}optional;
		};

		struct VolumeIDExtra {
			VolumeID* pVolumeID;
			char* pVolumeLabel;
			wchar_t* pVolumeLabelUnicode;
		};


		struct CommonNetworkRelativeLinkExtra {
			CommonNetworkRelativeLink* pCommonNetworkRelativeLink;
			char* pNetName;
			char* pDeviceName;
			wchar_t* pNetNameUnicode;
			wchar_t* pDeviceNameUnicode;
		};

		struct LinkInfoExtra {
			LinkInfo* pLinkInfo;
			VolumeIDExtra volumeIDExt;
			CommonNetworkRelativeLinkExtra commonNetworkRelativeLinkExt;
			char* pLocalBasePath;
			char* pCommonPathSuffix;
			wchar_t* pLocalBasePathUnicode;
			wchar_t* pCommonPathSuffixUnicode;
		};


		//
		//StringData
		//



		struct StringData {
			void* pRawData;
			uint16_t CountCharacters;
			bool bUnicode;
			std::string String;
			std::wstring StringUnicode;
		};


		//
		// ExtraData
		//

		const uint32_t ExtraData_ConsoleDataBlockSignature = 0xA0000002;
		const uint32_t ExtraData_ConsoleFEDataBlockSignature = 0xA0000004;
		const uint32_t ExtraData_DarwinDataBlockSignature = 0xA0000006;
		const uint32_t ExtraData_EnvironmentVariableDataBlockSignature = 0xA0000001;
		const uint32_t ExtraData_IconEnvironmentDataBlockSignature = 0xA0000007;
		const uint32_t ExtraData_KnownFolderDataBlockSignature = 0xA000000B;
		const uint32_t ExtraData_PropertyStoreDataBlockSignature = 0xA0000009;
		const uint32_t ExtraData_ShimDataBlockSignature = 0xA0000008;
		const uint32_t ExtraData_SpecialFolderDataBlockSignature = 0xA0000005;
		const uint32_t ExtraData_TrackerDataBlockSignature = 0xA0000003;
		const uint32_t ExtraData_VistaAndAboveIDListDataBlockSignature = 0xA000000C;

		struct ExtraData_General {
			uint32_t BlockSize;
			uint32_t BlockSignature;
			uint8_t  Body[1];
		};


		const uint32_t ExtraData_ConsoleDataBlockSize = 0x000000CC;
		struct ExtraData_ConsoleDataBlock{
			uint32_t BlockSize;
			uint32_t BlockSignature;
			uint16_t FillAttributes;
			uint16_t PopupFillAttributes;
			uint16_t ScreenBufferSizeX;
			uint16_t ScreenBufferSizeY;
			uint16_t WindowSizeX;
			uint16_t WindowSizeY;
			uint16_t WindowOriginX;
			uint16_t WindowOriginY;
			uint32_t Unused1;
			uint32_t Unused2;
			uint32_t FontSize;
			uint32_t FontFamily;
			uint32_t FontWeight;
			wchar_t FaceName[32];
			uint32_t CursorSize;
			uint32_t FullScreen;
			uint32_t QuickEdit;
			uint32_t InsertMode;
			uint32_t AutoPosition;
			uint32_t HistoryBufferSize;
			uint32_t NumberOfHistoryBuffers;
			uint32_t HistoryNoDup;
			uint32_t ColorTable[16];
		};

		const uint32_t ExtraData_ConsoleFEDataBlockSize = 0x0000000C;
		struct ExtraData_ConsoleFEDataBlock{
			uint32_t BlockSize;
			uint32_t BlockSignature;
			uint32_t CodePage;
		};

		const uint32_t ExtraData_DarwinDataBlockSize = 0x00000314;
		struct ExtraData_DarwinDataBlock{
			uint32_t BlockSize;
			uint32_t BlockSignature;
			char DarwinDataAnsi[260];
			wchar_t DarwinDataUnicode[260];
		};
		

		const uint32_t ExtraData_EnvironmentVariableDataBlockSize = 0x00000314;
		struct ExtraData_EnvironmentVariableDataBlock{
			uint32_t BlockSize;
			uint32_t BlockSignature;
			char TargetAnsi[260];
			wchar_t TargetUnicode[260];
		};
		
		const uint32_t ExtraData_IconEnvironmentDataBlockSize = 0x00000314;
		struct ExtraData_IconEnvironmentDataBlock{
			uint32_t BlockSize;
			uint32_t BlockSignature;
			char TargetAnsi[260];
			wchar_t TargetUnicode[260];
		};

		const uint32_t ExtraData_KnownFolderDataBlockSize = 0x0000001C;
		struct ExtraData_KnownFolderDataBlock{
			uint32_t BlockSize;
			uint32_t BlockSignature;
			GUID KnownFolderID;
			uint32_t Offset;
		};


		const uint32_t ExtraData_PropertyStoreDataBlockMinSize = 0x0000000C;

		struct ExtraData_PropertyStoreDataBlockHeader{
			uint32_t BlockSize;
			uint32_t BlockSignature;
		};

		struct ExtraData_PropertyStoreDataBlock {
			ExtraData_PropertyStoreDataBlockHeader* pHeader;
			uint8_t* pData;
			uint32_t dataSize;
		};
		

		const uint32_t ExtraData_ShimDataBlockSize = 0;
		struct ExtraData_ShimDataBlock{
			uint32_t BlockSize;
			uint32_t BlockSignature;

		};
		
		const uint32_t ExtraData_SpecialFolderDataBlockSize = 0x00000010;
		struct ExtraData_SpecialFolderDataBlock{ 
			uint32_t BlockSize;
			uint32_t BlockSignature;
			uint32_t SpecialFolderID;
			uint32_t Offset;
		
		};


		const uint32_t ExtraData_TrackerDataBlockSize = 0x00000060;
		struct ExtraData_TrackerDataBlock{
			uint32_t BlockSize;
			uint32_t BlockSignature;
			uint32_t Length;
			uint32_t Version;
			char MachineID[16];
			GUID Droid[2];
			GUID DroidBirth[2];
		};


		const uint32_t ExtraData_VistaAndAboveIDListDataBlockMinSize = 0x0000000A;

		struct ExtraData_VistaAndAboveIDListDataBlockHeader {
			uint32_t BlockSize;
			uint32_t BlockSignature;
		};

		struct ExtraData_VistaAndAboveIDListDataBlock {
			ExtraData_VistaAndAboveIDListDataBlockHeader* pHeader;
			uint32_t IDListSize;
			ITEMIDLIST* pIDList;
		};

	}
}