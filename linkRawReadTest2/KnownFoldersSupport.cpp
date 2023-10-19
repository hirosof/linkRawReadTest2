#include "KnownFoldersSupport.hpp"

const std::wstring hirosof::Supports::KnownFolders::getOriginalNameFromFolderID( GUID id ) {


    for ( const KnownFolderItem& item : knownFolderItems ) {

        if ( IsEqualGUID( item.folderID, id ) ) {
            std::wstring s;
            s.append( item.originalName );
            return s;
        }
    }

    return std::wstring( );
}
