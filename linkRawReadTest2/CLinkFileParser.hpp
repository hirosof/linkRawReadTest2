#pragma once

#include <Windows.h>
#include <stdint.h>
#include <memory>
#include <atlbase.h>
#include <atlcore.h>
#include <vector>

#include "LinkFileTypes.hpp"


namespace hirosof {
	namespace  LinkFile {

		class CLinkFileParser {
		public:
			CLinkFileParser( );
			~CLinkFileParser( );

			void Reset( void );

			HRESULT Load( const wchar_t* pFilePath );
			HRESULT Load( const char* pFilePath );
			HRESULT Load( IStream* pStream );
			HRESULT Parse( void );


			const  ShellLinkHeader* getHeaderPtr( void ) const;

			bool IsParsed( void )const;

			bool GetLinkTargetIDList( LinkTargetIDList* pIDList ) const;
			const LinkTargetIDList* GetLinkTargetIDListPtr( void ) const;
			const ITEMIDLIST* GetTargetItemIDList( void )const;
			const LinkInfoExtra* GetLinkInfo( void ) const;


			const StringData getNameString( void ) const;
			const StringData getRelativePath( void ) const;
			const StringData getWorkingDir( void ) const;
			const StringData getCommandLineArguments( void ) const;
			const StringData getIconLocation( void ) const;
			bool hasStringData( void )const;
			bool hasExtraData( void )const;

			const ExtraData_ConsoleDataBlock* getExtraConsoleDataBlockPtr( void ) const;
			const ExtraData_ConsoleFEDataBlock* getExtraConsoleFEDataBlockPtr( void ) const;
			const ExtraData_DarwinDataBlock* getExtraDarwinDataBlockPtr( void ) const;
			const ExtraData_EnvironmentVariableDataBlock* getExtraEnvironmentVariableDataBlockPtr( void ) const;
			const ExtraData_IconEnvironmentDataBlock* getExtraIconEnvironmentDataBlockPtr( void ) const;
			const ExtraData_KnownFolderDataBlock* getExtraKnownFolderDataBlockPtr( void ) const;
			const ExtraData_PropertyStoreDataBlock getExtraPropertyStoreDataBlock( void ) const;
			const ExtraData_ShimDataBlock* getExtraShimDataBlockPtr( void ) const;
			const ExtraData_SpecialFolderDataBlock* getExtraSpecialFolderDataBlockPtr( void ) const;
			const ExtraData_TrackerDataBlock* getExtraTrackerDataBlockPtr( void ) const;
			const ExtraData_VistaAndAboveIDListDataBlock getExtraVistaAndAboveIDListDataBlock( void ) const;

			const std::vector<ExtraData_General*> getUnclassifiedAndUnparsedExtraDataBlocks( void )const;

		private:
			CHeapPtr<uint8_t> m_rawData;
			bool m_Parsed;
			uint64_t m_rawDataSize;

			LinkTargetIDList m_IDList;
			LinkInfoExtra m_LinkInfo;

			StringData m_sd_Name_String;
			StringData m_sd_Relative_Path;
			StringData m_sd_Working_DIR;
			StringData m_sd_Command_Line_Arguments;
			StringData m_sd_Icon_Location;

			struct {
				ExtraData_ConsoleDataBlock* mp_ConsoleDataBlock;
				ExtraData_ConsoleFEDataBlock* mp_ConsoleFEDataBlock;
				ExtraData_DarwinDataBlock* mp_DarwinDataBlock;
				ExtraData_EnvironmentVariableDataBlock* mp_EnvironmentVariableDataBlock;
				ExtraData_IconEnvironmentDataBlock* mp_IconEnvironmentDataBlock;
				ExtraData_KnownFolderDataBlock* mp_KnownFolderDataBlock;
				ExtraData_PropertyStoreDataBlock m_PropertyStoreDataBlock;
				ExtraData_ShimDataBlock* mp_ShimDataBlock;
				ExtraData_SpecialFolderDataBlock* mp_SpecialFolderDataBlock;
				ExtraData_TrackerDataBlock* mp_TrackerDataBlock;
				ExtraData_VistaAndAboveIDListDataBlock m_VistaAndAboveIDListDataBlock;
			}m_extra;


			std::vector<ExtraData_General*> m_unclassified_and_unparsed_extra;



			bool m_hasStringData;
			bool m_hasExtraData;

			HRESULT  ParseLinkInfo( LinkInfoExtra* plie ) const;
			HRESULT  ParseVolumeID( VolumeIDExtra* pvide ) const;
			HRESULT  ParseCommonNetworkRelativeLink( CommonNetworkRelativeLinkExtra* pcnrle ) const;

			HRESULT  ClassifyExtraData( ExtraData_General* pData );

		};

	}
}