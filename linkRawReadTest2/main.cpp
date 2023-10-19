#include <stdio.h>
#include <locale>
#include <io.h>
#include <fcntl.h>

#include <ShlObj.h>
#include "CLinkFileParser.hpp"
#include "CCommandLineParser.hpp"
#include "PropertyStoreParseSupport.hpp"
#include "KnownFoldersSupport.hpp"

void ProcessMain( const wchar_t* pTargetFileName );

void PrintCLSID( const CLSID clsid );
void PrintFILETIME( const FILETIME  filetime );
void PrintlnCLSID( const CLSID clsid );
void PrintlnFILETIME( const FILETIME  filetime );

void ShowExtraData( hirosof::LinkFile::CLinkFileParser* pLink );


template <typename T, typename U> uint64_t CalcOffsetBytes( T* pBase, U* pTarget ) {
	uintptr_t b = reinterpret_cast<uintptr_t>( pBase );
	uintptr_t t = reinterpret_cast<uintptr_t>( pTarget );
	return ( b > t ) ? 0 : t - b;
}



enum struct DumpMode {
	None = 0,
	Hex,
	Array,
	Both
};


enum struct DumpWidthMode {
	Normal = 0,
	Extended,
	Custom
};


SIGDN  sigdn_values[] = {
	SIGDN_NORMALDISPLAY,
	SIGDN_PARENTRELATIVEPARSING,
	SIGDN_DESKTOPABSOLUTEPARSING,
	SIGDN_PARENTRELATIVEEDITING,
	SIGDN_DESKTOPABSOLUTEEDITING,
	SIGDN_FILESYSPATH,
	SIGDN_URL,
	SIGDN_PARENTRELATIVEFORADDRESSBAR,
	SIGDN_PARENTRELATIVE,
	SIGDN_PARENTRELATIVEFORUI
};

wchar_t  sigdn_values_shownames[][35] = {
	L"SIGDN_NORMALDISPLAY",
	L"SIGDN_PARENTRELATIVEPARSING",
	L"SIGDN_DESKTOPABSOLUTEPARSING",
	L"SIGDN_PARENTRELATIVEEDITING",
	L"SIGDN_DESKTOPABSOLUTEEDITING",
	L"SIGDN_FILESYSPATH",
	L"SIGDN_URL",
	L"SIGDN_PARENTRELATIVEFORADDRESSBAR",
	L"SIGDN_PARENTRELATIVE",
	L"SIGDN_PARENTRELATIVEFORUI"
};

DumpMode dumpMode = DumpMode::None;
DumpWidthMode  dumpWidthMode = DumpWidthMode::Extended;
int dumpWidthCustomValue = 0;


bool IsIntegerString( const std::wstring s );


void ShowItemIDList(const ITEMIDLIST *pIDList   , uint32_t IDListSize);

int main( void ) {

	//日本語を正常に表示させるための設定
	setlocale( LC_ALL, "Japanese" );

	CCommandLineParserExW  cmdline;

	if ( cmdline.parse( GetCommandLineW( ) ) <= 1 ) {
		wprintf( L"コマンドライン引数を指定してください。\n" );
		return 0;
	 }

	if ( cmdline.countOfUnnamedOption( ) < 2 ) {
		wprintf( L"コマンドライン引数で処理対象を指定してください。\n" );
		return 0;
	}

	int stdout_charset = _O_WTEXT;
	if ( cmdline.hasNamedOption( L"cs" ) || cmdline.hasNamedOption( L"charset" ) ) {
		std::wstring  cs_str = cmdline.getNamedOptionValue( L"cs" );
		if(cs_str.length() == 0 )cs_str = cmdline.getNamedOptionValue( L"charset" );
		if ( ( _wcsicmp( cs_str.c_str( ), L"utf8" ) == 0 ) || ( _wcsicmp( cs_str.c_str( ), L"utf-8" ) == 0 ) ) {
			stdout_charset = _O_U8TEXT;
		} else if ( ( _wcsicmp( cs_str.c_str( ), L"utf16" ) == 0 ) || ( _wcsicmp( cs_str.c_str( ), L"utf-16" ) == 0 ) ) {
			stdout_charset = _O_U16TEXT;
		} else if ( _wcsicmp( cs_str.c_str( ), L"wide" ) == 0 ) {
			stdout_charset = _O_WTEXT;
		} else if ( _wcsicmp( cs_str.c_str( ), L"ansi" ) == 0 ) {
			stdout_charset = _O_TEXT;
		} else if ( _wcsicmp( cs_str.c_str( ), L"sjis" ) == 0 ) {
			stdout_charset = _O_TEXT;
		} else if ( ( _wcsicmp( cs_str.c_str( ), L"shiftjis" ) == 0 ) || ( _wcsicmp( cs_str.c_str( ), L"shift-jis" ) == 0 ) ) {
			stdout_charset = _O_TEXT;
		}
	}

	if ( cmdline.hasNamedOption( L"dump" ) ) {
		std::wstring  optionvalue = cmdline.getNamedOptionValue( L"dump" );
		if (  _wcsicmp( optionvalue.c_str( ), L"none" ) == 0 ) {
			dumpMode = DumpMode::None;
		} else if ( _wcsicmp( optionvalue.c_str( ), L"hex" ) == 0 ) {
			dumpMode = DumpMode::Hex;
		} else if ( _wcsicmp( optionvalue.c_str( ), L"array" ) == 0 ) {
			dumpMode = DumpMode::Array;
		} else if ( _wcsicmp( optionvalue.c_str( ), L"both" ) == 0 ) {
			dumpMode = DumpMode::Both;
		}
	}


	if ( cmdline.hasNamedOption( L"dumpwidth" ) ) {
		std::wstring  optionvalue = cmdline.getNamedOptionValue( L"dumpwidth" );
		if (  _wcsicmp( optionvalue.c_str( ), L"normal" ) == 0 ) {
			dumpWidthMode = DumpWidthMode::Normal;
		}else if ( ( _wcsicmp( optionvalue.c_str( ), L"ext" ) == 0 ) || ( _wcsicmp( optionvalue.c_str( ), L"extended" ) == 0 ) ) {
			dumpWidthMode = DumpWidthMode::Extended;
		}else if ( _wcsicmp( optionvalue.c_str( ), L"wide" ) == 0 ) {
			dumpWidthMode = DumpWidthMode::Extended;
		} else if( IsIntegerString(optionvalue ) ) {
			dumpWidthCustomValue = cmdline.getNamedOptionTypeValue<int>( L"dumpwidth", 0, 0 );
			if ( dumpWidthCustomValue < 0 ) dumpWidthMode = DumpWidthMode::Extended;
			else if ( dumpWidthCustomValue == 0 ) dumpWidthMode = DumpWidthMode::Normal;
			else  dumpWidthMode = DumpWidthMode::Custom;
		}
	}

	(void)_setmode( _fileno( stdout ), stdout_charset );

	if ( FAILED( CoInitializeEx( NULL, COINIT_APARTMENTTHREADED ) ) ) {

		wprintf( L"CoInitializeEx Failed.\n" );
		return 0;
	}
		
	auto targets_name = cmdline.getUnnamedOptionValues( );
	auto it_target = targets_name.cbegin( );

	//最初の項目は自分自身なのでスキップする
	it_target++;

	while ( it_target != targets_name.cend() ) {
		ProcessMain( it_target->c_str() );
		wprintf( L"\n\n" );
		it_target++;
	}



	CoUninitialize( );

	return 0;
}


bool IsIntegerString( const std::wstring s ) {

	auto it = s.cbegin( );

	if ( ( std::isdigit( *it ) == 0 ) && ( *it != L'-' ) ) return false;

	it++;

	while ( it != s.cend( ) ) {
		if ( std::isdigit( *it ) == 0 ) return false;
		it++;
	}

	return true;
}

void ProcessMain( const wchar_t *pTargetFileName ) {

	hirosof::LinkFile::CLinkFileParser link;

	HRESULT hr = link.Load(pTargetFileName);
	wprintf(L"# %s\n\n", pTargetFileName );

	if ( hr != S_OK ) {
		wprintf( L"file load failed. (HRESULT = 0x%08X)\n"  , hr);
		return;
	}

	hr = link.Parse( );

	if ( hr == S_OK ) {
		
		auto  pHeader = link.getHeaderPtr( );
	
		wprintf(L"## ShellLinkHeader (Offset : 0x%08I64x)\n\n" , CalcOffsetBytes(pHeader , pHeader) );

		wprintf(L"```\n" );
		wprintf(L"HeaderSize : 0x%08x\n", pHeader->HeaderSize );
		wprintf(L"LinkCLSID : " );
		PrintlnCLSID( pHeader->LinkCLSID );

		wprintf(L"LinkFlags : %u (0x%08x)\n" , pHeader->LinkFlags.raw, pHeader->LinkFlags.raw );
		
		for ( size_t i = 0; i < 27; i++ ) {
			uint32_t flag = ( 1 << ( i  ) );
			if ( pHeader->LinkFlags.raw & flag) {
				wprintf(L"\t%S (0x%08x) \n", hirosof::LinkFile::LinkFlagsBitFieldItemNameStrings[i] , flag );
			}
			
		}

		wprintf(L"\n" );

		wprintf(L"FileAttributes : %u (0x%08x)\n", pHeader->FileAttributes.raw, pHeader->FileAttributes.raw );
		for ( size_t i = 0; i < 15; i++ ) {
			uint32_t flag = ( 1 << ( i  ) );
			if ( pHeader->FileAttributes.raw & flag) {
				wprintf(L"\t%S (0x%08x) \n", hirosof::LinkFile::LinkFileAttributesFlagsBitFieldItemNameStrings[i] , flag );
			}
		}

		wprintf(L"\n" );
		wprintf( L"CreationTime : " );
		if ( *reinterpret_cast<const uint64_t*>(&pHeader->CreationTime) != 0 ) {
			PrintlnFILETIME( pHeader->CreationTime );
		} else {
			wprintf( L"None\n" );
		}

		wprintf( L"AccessTime : " );
		if ( *reinterpret_cast<const uint64_t*>( &pHeader->AccessTime ) != 0 ) {
			PrintlnFILETIME( pHeader->AccessTime );
		} else {
			wprintf( L"None\n" );
		}

		wprintf( L"WriteTime : " );
		if ( *reinterpret_cast<const uint64_t*>( &pHeader->WriteTime ) != 0 ) {
			PrintlnFILETIME( pHeader->WriteTime );
		} else {
			wprintf( L"None\n" );
		}

		wprintf(L"FileSize : %u\n", pHeader->FileSize );
		wprintf(L"\n" );
		wprintf(L"IconIndex : %d\n", pHeader->IconIndex );
		wprintf(L"ShowCommand : " );

		switch ( pHeader->ShowCommand ) {
			case SW_SHOWNORMAL:
				wprintf( L"SW_SHOWNORMAL" );
				break;
			case SW_SHOWMAXIMIZED:
				wprintf( L"SW_SHOWMAXIMIZED" );
				break;
			case SW_SHOWMINNOACTIVE:
				wprintf( L"SW_SHOWMINNOACTIVE" );
				break;
			default:
				wprintf( L"treated as SW_SHOWNORMAL" );
				break;
		}

		wprintf(L" (0x%08x)\n", pHeader->ShowCommand );

		wprintf( L"HotKey : " );

		if ( pHeader->HotKey == 0 ) {
			wprintf( L"None" );
		} else {

			uint8_t specialkey = ( pHeader->HotKey & 0xFF00 ) >> 8;
			if ( specialkey & HOTKEYF_EXT ) wprintf( L"[Extended Key] " );
			if ( specialkey & HOTKEYF_CONTROL ) wprintf( L"Ctrl + " );
			if ( specialkey & HOTKEYF_SHIFT ) wprintf( L"Shift + " );
			if ( specialkey & HOTKEYF_ALT ) wprintf( L"Alt + " );

			uint8_t key = ( pHeader->HotKey & 0xFF );
			switch ( key ) {
				case VK_LBUTTON: wprintf( L"VK_LBUTTON" ); break;
				case VK_RBUTTON: wprintf( L"VK_RBUTTON" ); break;
				case VK_CANCEL: wprintf( L"VK_CANCEL" ); break;
				case VK_MBUTTON: wprintf( L"VK_MBUTTON" ); break;
				case VK_XBUTTON1: wprintf( L"VK_XBUTTON1" ); break;
				case VK_XBUTTON2: wprintf( L"VK_XBUTTON2" ); break;
				case VK_BACK: wprintf( L"VK_BACK" ); break;
				case VK_TAB: wprintf( L"VK_TAB" ); break;
				case VK_CLEAR: wprintf( L"VK_CLEAR" ); break;
				case VK_RETURN: wprintf( L"VK_RETURN" ); break;
				case VK_SHIFT: wprintf( L"VK_SHIFT" ); break;
				case VK_CONTROL: wprintf( L"VK_CONTROL" ); break;
				case VK_MENU: wprintf( L"VK_MENU" ); break;
				case VK_PAUSE: wprintf( L"VK_PAUSE" ); break;
				case VK_CAPITAL: wprintf( L"VK_CAPITAL" ); break;
				case VK_KANA: wprintf( L"VK_KANA" ); break;
				//case VK_HANGUEL: wprintf( L"VK_HANGUEL" ); break;
				//case VK_HANGUL: wprintf( L"VK_HANGUL" ); break;
				case VK_IME_ON: wprintf( L"VK_IME_ON" ); break;
				case VK_JUNJA: wprintf( L"VK_JUNJA" ); break;
				case VK_FINAL: wprintf( L"VK_FINAL" ); break;
				//case VK_HANJA: wprintf( L"VK_HANJA" ); break;
				case VK_KANJI: wprintf( L"VK_KANJI" ); break;
				case VK_IME_OFF: wprintf( L"VK_IME_OFF" ); break;
				case VK_ESCAPE: wprintf( L"VK_ESCAPE" ); break;
				case VK_CONVERT: wprintf( L"VK_CONVERT" ); break;
				case VK_NONCONVERT: wprintf( L"VK_NONCONVERT" ); break;
				case VK_ACCEPT: wprintf( L"VK_ACCEPT" ); break;
				case VK_MODECHANGE: wprintf( L"VK_MODECHANGE" ); break;
				case VK_SPACE: wprintf( L"VK_SPACE" ); break;
				case VK_PRIOR: wprintf( L"VK_PRIOR" ); break;
				case VK_NEXT: wprintf( L"VK_NEXT" ); break;
				case VK_END: wprintf( L"VK_END" ); break;
				case VK_HOME: wprintf( L"VK_HOME" ); break;
				case VK_LEFT: wprintf( L"VK_LEFT" ); break;
				case VK_UP: wprintf( L"VK_UP" ); break;
				case VK_RIGHT: wprintf( L"VK_RIGHT" ); break;
				case VK_DOWN: wprintf( L"VK_DOWN" ); break;
				case VK_SELECT: wprintf( L"VK_SELECT" ); break;
				case VK_PRINT: wprintf( L"VK_PRINT" ); break;
				case VK_EXECUTE: wprintf( L"VK_EXECUTE" ); break;
				case VK_SNAPSHOT: wprintf( L"VK_SNAPSHOT" ); break;
				case VK_INSERT: wprintf( L"VK_INSERT" ); break;
				case VK_DELETE: wprintf( L"VK_DELETE" ); break;
				case VK_HELP: wprintf( L"VK_HELP" ); break;
				case VK_LWIN: wprintf( L"VK_LWIN" ); break;
				case VK_RWIN: wprintf( L"VK_RWIN" ); break;
				case VK_APPS: wprintf( L"VK_APPS" ); break;
				case VK_SLEEP: wprintf( L"VK_SLEEP" ); break;
				case VK_NUMPAD0: wprintf( L"VK_NUMPAD0" ); break;
				case VK_NUMPAD1: wprintf( L"VK_NUMPAD1" ); break;
				case VK_NUMPAD2: wprintf( L"VK_NUMPAD2" ); break;
				case VK_NUMPAD3: wprintf( L"VK_NUMPAD3" ); break;
				case VK_NUMPAD4: wprintf( L"VK_NUMPAD4" ); break;
				case VK_NUMPAD5: wprintf( L"VK_NUMPAD5" ); break;
				case VK_NUMPAD6: wprintf( L"VK_NUMPAD6" ); break;
				case VK_NUMPAD7: wprintf( L"VK_NUMPAD7" ); break;
				case VK_NUMPAD8: wprintf( L"VK_NUMPAD8" ); break;
				case VK_NUMPAD9: wprintf( L"VK_NUMPAD9" ); break;
				case VK_MULTIPLY: wprintf( L"VK_MULTIPLY" ); break;
				case VK_ADD: wprintf( L"VK_ADD" ); break;
				case VK_SEPARATOR: wprintf( L"VK_SEPARATOR" ); break;
				case VK_SUBTRACT: wprintf( L"VK_SUBTRACT" ); break;
				case VK_DECIMAL: wprintf( L"VK_DECIMAL" ); break;
				case VK_DIVIDE: wprintf( L"VK_DIVIDE" ); break;
				case VK_F1: wprintf( L"VK_F1" ); break;
				case VK_F2: wprintf( L"VK_F2" ); break;
				case VK_F3: wprintf( L"VK_F3" ); break;
				case VK_F4: wprintf( L"VK_F4" ); break;
				case VK_F5: wprintf( L"VK_F5" ); break;
				case VK_F6: wprintf( L"VK_F6" ); break;
				case VK_F7: wprintf( L"VK_F7" ); break;
				case VK_F8: wprintf( L"VK_F8" ); break;
				case VK_F9: wprintf( L"VK_F9" ); break;
				case VK_F10: wprintf( L"VK_F10" ); break;
				case VK_F11: wprintf( L"VK_F11" ); break;
				case VK_F12: wprintf( L"VK_F12" ); break;
				case VK_F13: wprintf( L"VK_F13" ); break;
				case VK_F14: wprintf( L"VK_F14" ); break;
				case VK_F15: wprintf( L"VK_F15" ); break;
				case VK_F16: wprintf( L"VK_F16" ); break;
				case VK_F17: wprintf( L"VK_F17" ); break;
				case VK_F18: wprintf( L"VK_F18" ); break;
				case VK_F19: wprintf( L"VK_F19" ); break;
				case VK_F20: wprintf( L"VK_F20" ); break;
				case VK_F21: wprintf( L"VK_F21" ); break;
				case VK_F22: wprintf( L"VK_F22" ); break;
				case VK_F23: wprintf( L"VK_F23" ); break;
				case VK_F24: wprintf( L"VK_F24" ); break;
				case VK_NUMLOCK: wprintf( L"VK_NUMLOCK" ); break;
				case VK_SCROLL: wprintf( L"VK_SCROLL" ); break;
				case VK_LSHIFT: wprintf( L"VK_LSHIFT" ); break;
				case VK_RSHIFT: wprintf( L"VK_RSHIFT" ); break;
				case VK_LCONTROL: wprintf( L"VK_LCONTROL" ); break;
				case VK_RCONTROL: wprintf( L"VK_RCONTROL" ); break;
				case VK_LMENU: wprintf( L"VK_LMENU" ); break;
				case VK_RMENU: wprintf( L"VK_RMENU" ); break;
				case VK_BROWSER_BACK: wprintf( L"VK_BROWSER_BACK" ); break;
				case VK_BROWSER_FORWARD: wprintf( L"VK_BROWSER_FORWARD" ); break;
				case VK_BROWSER_REFRESH: wprintf( L"VK_BROWSER_REFRESH" ); break;
				case VK_BROWSER_STOP: wprintf( L"VK_BROWSER_STOP" ); break;
				case VK_BROWSER_SEARCH: wprintf( L"VK_BROWSER_SEARCH" ); break;
				case VK_BROWSER_FAVORITES: wprintf( L"VK_BROWSER_FAVORITES" ); break;
				case VK_BROWSER_HOME: wprintf( L"VK_BROWSER_HOME" ); break;
				case VK_VOLUME_MUTE: wprintf( L"VK_VOLUME_MUTE" ); break;
				case VK_VOLUME_DOWN: wprintf( L"VK_VOLUME_DOWN" ); break;
				case VK_VOLUME_UP: wprintf( L"VK_VOLUME_UP" ); break;
				case VK_MEDIA_NEXT_TRACK: wprintf( L"VK_MEDIA_NEXT_TRACK" ); break;
				case VK_MEDIA_PREV_TRACK: wprintf( L"VK_MEDIA_PREV_TRACK" ); break;
				case VK_MEDIA_STOP: wprintf( L"VK_MEDIA_STOP" ); break;
				case VK_MEDIA_PLAY_PAUSE: wprintf( L"VK_MEDIA_PLAY_PAUSE" ); break;
				case VK_LAUNCH_MAIL: wprintf( L"VK_LAUNCH_MAIL" ); break;
				case VK_LAUNCH_MEDIA_SELECT: wprintf( L"VK_LAUNCH_MEDIA_SELECT" ); break;
				case VK_LAUNCH_APP1: wprintf( L"VK_LAUNCH_APP1" ); break;
				case VK_LAUNCH_APP2: wprintf( L"VK_LAUNCH_APP2" ); break;
				case VK_OEM_1: wprintf( L"VK_OEM_1" ); break;
				case VK_OEM_PLUS: wprintf( L"VK_OEM_PLUS" ); break;
				case VK_OEM_COMMA: wprintf( L"VK_OEM_COMMA" ); break;
				case VK_OEM_MINUS: wprintf( L"VK_OEM_MINUS" ); break;
				case VK_OEM_PERIOD: wprintf( L"VK_OEM_PERIOD" ); break;
				case VK_OEM_2: wprintf( L"VK_OEM_2" ); break;
				case VK_OEM_3: wprintf( L"VK_OEM_3" ); break;
				case VK_OEM_4: wprintf( L"VK_OEM_4" ); break;
				case VK_OEM_5: wprintf( L"VK_OEM_5" ); break;
				case VK_OEM_6: wprintf( L"VK_OEM_6" ); break;
				case VK_OEM_7: wprintf( L"VK_OEM_7" ); break;
				case VK_OEM_8: wprintf( L"VK_OEM_8" ); break;
				case VK_OEM_102: wprintf( L"VK_OEM_102" ); break;
				case VK_PROCESSKEY: wprintf( L"VK_PROCESSKEY" ); break;
				case VK_PACKET: wprintf( L"VK_PACKET" ); break;
				case VK_ATTN: wprintf( L"VK_ATTN" ); break;
				case VK_CRSEL: wprintf( L"VK_CRSEL" ); break;
				case VK_EXSEL: wprintf( L"VK_EXSEL" ); break;
				case VK_EREOF: wprintf( L"VK_EREOF" ); break;
				case VK_PLAY: wprintf( L"VK_PLAY" ); break;
				case VK_ZOOM: wprintf( L"VK_ZOOM" ); break;
				case VK_NONAME: wprintf( L"VK_NONAME" ); break;
				case VK_PA1: wprintf( L"VK_PA1" ); break;
				case VK_OEM_CLEAR: wprintf( L"VK_OEM_CLEAR" ); break;
				default:
					if ( ( key >= '0' ) && ( key <= '9' ) ) {
						// 数字キー
						wprintf( L"%C", key );
					} else if ( ( key >= 'A' ) && ( key <= 'Z' ) ) {
						// アルファベットキー
						wprintf( L"%C", key );
					} else {
						wprintf( L"Unknown" );
					}
					break;
			}
		}

		wprintf(L" (0x%04x)\n", pHeader->HotKey );

		wprintf(L"```\n" );

		if ( pHeader->LinkFlags.item.HasLinkTargetIDList ) {
			wprintf( L"\n## LinkTargetIDList (Offset : 0x%08I64x)\n\n```\n",
				CalcOffsetBytes( pHeader, link.GetTargetItemIDList( ) ) - sizeof( hirosof::LinkFile::LinkTargetIDList::IDListSize ) );

			using hirosof::LinkFile::LinkTargetIDList;

			const LinkTargetIDList* pLinkTargetIDList = link.GetLinkTargetIDListPtr( );


			if ( pLinkTargetIDList ) {
				ShowItemIDList( pLinkTargetIDList->pIDList, pLinkTargetIDList->IDListSize );
			}

			wprintf(L"```\n" );
		}

		if ( pHeader->LinkFlags.item.HasLinkInfo ) {

			auto pLinkInfo = link.GetLinkInfo( );

			if ( pLinkInfo ) {
				wprintf( L"\n## LinkInfo (Offset : 0x%08I64x) \n\n```\n", CalcOffsetBytes( pHeader, pLinkInfo->pLinkInfo ));

				wprintf(L"LinkInfoSize : %u (0x%08x)\n", pLinkInfo->pLinkInfo->LinkInfoSize, pLinkInfo->pLinkInfo->LinkInfoSize );
				wprintf(L"LinkInfoHeaderSize : %u (0x%08x)\n",
					pLinkInfo->pLinkInfo->LinkInfoHeaderSize, pLinkInfo->pLinkInfo->LinkInfoHeaderSize );
				wprintf(L"LinkInfoFlags : %u (0x%08x)\n", pLinkInfo->pLinkInfo->LinkInfoFlags, pLinkInfo->pLinkInfo->LinkInfoFlags );
				if ( pLinkInfo->pLinkInfo->LinkInfoFlags & 1 ) wprintf(L"\tVolumeIDAndLocalBasePath\n" );
				if ( pLinkInfo->pLinkInfo->LinkInfoFlags & 2 ) wprintf(L"\tCommonNetworkRelativeLinkAndPathSuffix\n" );
				if ( pLinkInfo->pLinkInfo->LinkInfoFlags != 0 )wprintf(L"\n" );

				if ( pLinkInfo->pLocalBasePath ) {
					wprintf(L"LocalBasePath : \n\t%S", pLinkInfo->pLocalBasePath );
					wprintf(L"\n\n" );
				}
				if ( pLinkInfo->pLocalBasePathUnicode ) {
					wprintf(L"LocalBasePathUnicode : \n\t%s", pLinkInfo->pLocalBasePathUnicode );
					wprintf(L"\n\n" );
				}
				if ( pLinkInfo->pCommonPathSuffix ) {
					wprintf(L"CommonPathSuffix : \n\t%S", pLinkInfo->pCommonPathSuffix );
					wprintf(L"\n\n" );
				}
				if ( pLinkInfo->pCommonPathSuffixUnicode ) {
					wprintf(L"CommonPathSuffixUnicode : \n\t%s", pLinkInfo->pCommonPathSuffixUnicode );
					wprintf(L"\n\n" );
				}
				wprintf(L"```\n" );

				if ( pLinkInfo->volumeIDExt.pVolumeID ) {

					hirosof::LinkFile::VolumeID* pVolumeID = pLinkInfo->volumeIDExt.pVolumeID;
					wprintf( L"\n### VolumeID (Offset : 0x%08I64x) \n\n```\n", CalcOffsetBytes( pHeader, pVolumeID ) );

					wprintf( L"VolumeIDSize : %u (0x%08x)\n", pVolumeID->VolumeIDSize, pVolumeID->VolumeIDSize );
					wprintf( L"DriveType : " );
					switch ( pVolumeID->DriveType ) {
						case 	DRIVE_UNKNOWN:
							wprintf( L"DRIVE_UNKNOWN" );
							break;
						case DRIVE_NO_ROOT_DIR:
							wprintf( L"DRIVE_NO_ROOT_DIR" );
							break;
						case DRIVE_REMOVABLE:
							wprintf( L"DRIVE_REMOVABLE" );
							break;
						case DRIVE_FIXED:
							wprintf( L"DRIVE_FIXED" );
							break;
						case DRIVE_REMOTE:
							wprintf( L"DRIVE_REMOTE" );
							break;
						case DRIVE_CDROM:
							wprintf( L"DRIVE_CDROM" );
							break;
						case DRIVE_RAMDISK:
							wprintf( L"DRIVE_RAMDISK" );
							break;

					}
					wprintf( L" (0x%08x)\n", pVolumeID->DriveType );
					wprintf( L"DriveSerialNumber : %08X\n", pVolumeID->DriveSerialNumber );
					if ( pLinkInfo->volumeIDExt.pVolumeLabel ) 
						wprintf( L"VolumeLabel : %S\n", pLinkInfo->volumeIDExt.pVolumeLabel );
					if ( pLinkInfo->volumeIDExt.pVolumeLabelUnicode ) 
						wprintf( L"VolumeLabelUnicode : %s\n", pLinkInfo->volumeIDExt.pVolumeLabelUnicode );
					wprintf( L"```\n" );
				}

				if ( pLinkInfo->commonNetworkRelativeLinkExt.pCommonNetworkRelativeLink ) {
					hirosof::LinkFile::CommonNetworkRelativeLink* pCommonNetworkReletiveLink = pLinkInfo->commonNetworkRelativeLinkExt.pCommonNetworkRelativeLink;

					wprintf( L"\n### CommonNetworkRelativeLink (Offset : 0x%08I64x) \n\n```\n",
						CalcOffsetBytes( pHeader, pCommonNetworkReletiveLink ) );
					wprintf( L"CommonNetworkRelativeLinkSize : %u (0x%08x)\n",
						pCommonNetworkReletiveLink->CommonNetworkRelativeLinkSize, 
						pCommonNetworkReletiveLink->CommonNetworkRelativeLinkSize );
					wprintf( L"CommonNetworkRelativeLinkFlags : %u (0x%08x)\n", 
						pCommonNetworkReletiveLink->CommonNetworkRelativeLinkFlags,
						pCommonNetworkReletiveLink->CommonNetworkRelativeLinkFlags );

					if ( pCommonNetworkReletiveLink->CommonNetworkRelativeLinkFlags & 1 ) wprintf( L"\tValidDevice\n" );
					if ( pCommonNetworkReletiveLink->CommonNetworkRelativeLinkFlags & 2 ) wprintf( L"\tValidNetType\n" );
					if ( pCommonNetworkReletiveLink->CommonNetworkRelativeLinkFlags != 0 )wprintf( L"\n" );


					if ( pLinkInfo->commonNetworkRelativeLinkExt.pNetName )	
						wprintf( L"NetName : \n\t%S\n\n", pLinkInfo->commonNetworkRelativeLinkExt.pNetName );

					if ( pLinkInfo->commonNetworkRelativeLinkExt.pNetNameUnicode )	
						wprintf( L"NetNameUnicode : \n\t%s\n\n", pLinkInfo->commonNetworkRelativeLinkExt.pNetNameUnicode );

					if ( pLinkInfo->commonNetworkRelativeLinkExt.pDeviceName )
						wprintf( L"DeviceName : \n\t%S\n\n", pLinkInfo->commonNetworkRelativeLinkExt.pDeviceName );

					if ( pLinkInfo->commonNetworkRelativeLinkExt.pDeviceNameUnicode )
						wprintf( L"DeviceNameUnicode : \n\t%s\n\n", pLinkInfo->commonNetworkRelativeLinkExt.pDeviceNameUnicode );

					if ( pCommonNetworkReletiveLink->CommonNetworkRelativeLinkFlags & 2 ) {
						wprintf( L"NetProviderType  : " );
						switch ( pCommonNetworkReletiveLink->NetworkProviderType ) {
							case WNNC_NET_MSNET: wprintf( L"WNNC_NET_MSNET" ); break;
							case WNNC_NET_SMB: wprintf( L"WNNC_NET_SMB" ); break;
							case WNNC_NET_NETWARE: wprintf( L"WNNC_NET_NETWARE" ); break;
							case WNNC_NET_VINES: wprintf( L"WNNC_NET_VINES" ); break;
							case WNNC_NET_10NET: wprintf( L"WNNC_NET_10NET" ); break;
							case WNNC_NET_LOCUS: wprintf( L"WNNC_NET_LOCUS" ); break;
							case WNNC_NET_SUN_PC_NFS: wprintf( L"WNNC_NET_SUN_PC_NFS" ); break;
							case WNNC_NET_LANSTEP: wprintf( L"WNNC_NET_LANSTEP" ); break;
							case WNNC_NET_9TILES: wprintf( L"WNNC_NET_9TILES" ); break;
							case WNNC_NET_LANTASTIC: wprintf( L"WNNC_NET_LANTASTIC" ); break;
							case WNNC_NET_AS400: wprintf( L"WNNC_NET_AS400" ); break;
							case WNNC_NET_FTP_NFS: wprintf( L"WNNC_NET_FTP_NFS" ); break;
							case WNNC_NET_PATHWORKS: wprintf( L"WNNC_NET_PATHWORKS" ); break;
							case WNNC_NET_LIFENET: wprintf( L"WNNC_NET_LIFENET" ); break;
							case WNNC_NET_POWERLAN: wprintf( L"WNNC_NET_POWERLAN" ); break;
							case WNNC_NET_BWNFS: wprintf( L"WNNC_NET_BWNFS" ); break;
							case WNNC_NET_COGENT: wprintf( L"WNNC_NET_COGENT" ); break;
							case WNNC_NET_FARALLON: wprintf( L"WNNC_NET_FARALLON" ); break;
							case WNNC_NET_APPLETALK: wprintf( L"WNNC_NET_APPLETALK" ); break;
							case WNNC_NET_INTERGRAPH: wprintf( L"WNNC_NET_INTERGRAPH" ); break;
							case WNNC_NET_SYMFONET: wprintf( L"WNNC_NET_SYMFONET" ); break;
							case WNNC_NET_CLEARCASE: wprintf( L"WNNC_NET_CLEARCASE" ); break;
							case WNNC_NET_FRONTIER: wprintf( L"WNNC_NET_FRONTIER" ); break;
							case WNNC_NET_BMC: wprintf( L"WNNC_NET_BMC" ); break;
							case WNNC_NET_DCE: wprintf( L"WNNC_NET_DCE" ); break;
							case WNNC_NET_AVID: wprintf( L"WNNC_NET_AVID" ); break;
							case WNNC_NET_DOCUSPACE: wprintf( L"WNNC_NET_DOCUSPACE" ); break;
							case WNNC_NET_MANGOSOFT: wprintf( L"WNNC_NET_MANGOSOFT" ); break;
							case WNNC_NET_SERNET: wprintf( L"WNNC_NET_SERNET" ); break;
							case WNNC_NET_RIVERFRONT1: wprintf( L"WNNC_NET_RIVERFRONT1" ); break;
							case WNNC_NET_RIVERFRONT2: wprintf( L"WNNC_NET_RIVERFRONT2" ); break;
							case WNNC_NET_DECORB: wprintf( L"WNNC_NET_DECORB" ); break;
							case WNNC_NET_PROTSTOR: wprintf( L"WNNC_NET_PROTSTOR" ); break;
							case WNNC_NET_FJ_REDIR: wprintf( L"WNNC_NET_FJ_REDIR" ); break;
							case WNNC_NET_DISTINCT: wprintf( L"WNNC_NET_DISTINCT" ); break;
							case WNNC_NET_TWINS: wprintf( L"WNNC_NET_TWINS" ); break;
							case WNNC_NET_RDR2SAMPLE: wprintf( L"WNNC_NET_RDR2SAMPLE" ); break;
							case WNNC_NET_CSC: wprintf( L"WNNC_NET_CSC" ); break;
							case WNNC_NET_3IN1: wprintf( L"WNNC_NET_3IN1" ); break;
							case WNNC_NET_EXTENDNET: wprintf( L"WNNC_NET_EXTENDNET" ); break;
							case WNNC_NET_STAC: wprintf( L"WNNC_NET_STAC" ); break;
							case WNNC_NET_FOXBAT: wprintf( L"WNNC_NET_FOXBAT" ); break;
							case WNNC_NET_YAHOO: wprintf( L"WNNC_NET_YAHOO" ); break;
							case WNNC_NET_EXIFS: wprintf( L"WNNC_NET_EXIFS" ); break;
							case WNNC_NET_DAV: wprintf( L"WNNC_NET_DAV" ); break;
							case WNNC_NET_KNOWARE: wprintf( L"WNNC_NET_KNOWARE" ); break;
							case WNNC_NET_OBJECT_DIRE: wprintf( L"WNNC_NET_OBJECT_DIRE" ); break;
							case WNNC_NET_MASFAX: wprintf( L"WNNC_NET_MASFAX" ); break;
							case WNNC_NET_HOB_NFS: wprintf( L"WNNC_NET_HOB_NFS" ); break;
							case WNNC_NET_SHIVA: wprintf( L"WNNC_NET_SHIVA" ); break;
							case WNNC_NET_IBMAL: wprintf( L"WNNC_NET_IBMAL" ); break;
							case WNNC_NET_LOCK: wprintf( L"WNNC_NET_LOCK" ); break;
							case WNNC_NET_TERMSRV: wprintf( L"WNNC_NET_TERMSRV" ); break;
							case WNNC_NET_SRT: wprintf( L"WNNC_NET_SRT" ); break;
							case WNNC_NET_QUINCY: wprintf( L"WNNC_NET_QUINCY" ); break;
							case WNNC_NET_OPENAFS: wprintf( L"WNNC_NET_OPENAFS" ); break;
							case WNNC_NET_AVID1: wprintf( L"WNNC_NET_AVID1" ); break;
							case WNNC_NET_DFS: wprintf( L"WNNC_NET_DFS" ); break;
							case WNNC_NET_KWNP: wprintf( L"WNNC_NET_KWNP" ); break;
							case WNNC_NET_ZENWORKS: wprintf( L"WNNC_NET_ZENWORKS" ); break;
							case WNNC_NET_DRIVEONWEB: wprintf( L"WNNC_NET_DRIVEONWEB" ); break;
							case WNNC_NET_VMWARE: wprintf( L"WNNC_NET_VMWARE" ); break;
							case WNNC_NET_RSFX: wprintf( L"WNNC_NET_RSFX" ); break;
							case WNNC_NET_MFILES: wprintf( L"WNNC_NET_MFILES" ); break;
							case WNNC_NET_MS_NFS: wprintf( L"WNNC_NET_MS_NFS" ); break;
							case WNNC_NET_GOOGLE: wprintf( L"WNNC_NET_GOOGLE" ); break;
							case WNNC_NET_NDFS: wprintf( L"WNNC_NET_NDFS" ); break;
							case WNNC_NET_DOCUSHARE: wprintf( L"WNNC_NET_DOCUSHARE" ); break;
							case WNNC_NET_AURISTOR_FS: wprintf( L"WNNC_NET_AURISTOR_FS" ); break;
							case WNNC_NET_SECUREAGENT: wprintf( L"WNNC_NET_SECUREAGENT" ); break;
							case WNNC_NET_9P: wprintf( L"WNNC_NET_9P" ); break;
						}
						wprintf( L" (0x%08x)\n", pCommonNetworkReletiveLink->NetworkProviderType );
					}
					wprintf( L"```\n" );
				}
			}
		}


		//  string
		if ( link.hasStringData( ) ) {

			wprintf( L"\n## StringData\n\n");

			const wchar_t names[][24] = {
				L"NAME_STRING",
				L"RELATIVE_PATH",
				L"WORKING_DIR",
				L"COMMAND_LINE_ARGUMENTS",
				L"ICON_LOCATION"
			};

			const hirosof::LinkFile::StringData sd[] = {
				link.getNameString( ),
				link.getRelativePath( ),
				link.getWorkingDir( ),
				link.getCommandLineArguments( ),
				link.getIconLocation( )
			};


			for ( size_t i = 0; i < ( sizeof( sd ) / sizeof( hirosof::LinkFile::StringData ) ); i++ ) {

				if ( sd[i].pRawData == nullptr ) continue;

				wprintf( L"\n### %s (Offset : 0x%08I64x) \n\n", names[i], CalcOffsetBytes( pHeader, sd[i].pRawData ) );

				wprintf( L"* CountCharacters : %u (0x%04x)\n", sd[i].CountCharacters, sd[i].CountCharacters );


				if ( sd[i].CountCharacters != 0 ) {
					wprintf( L"* String\n\n\t```\n" );

					if ( pHeader->LinkFlags.item.IsUnicode ) {
						wprintf( L"\t%s\n", sd[i].StringUnicode.c_str( ) );
					} else {
						wprintf( L"\t%S\n", sd[i].String.c_str( ) );
					}
					wprintf( L"\t```\n" );
				}
			}


			wprintf( L"\n" );


		}

		if ( link.hasExtraData( ) ) {

			auto general = link.getUnclassifiedAndUnparsedExtraDataBlocks( );
			
			wprintf( L"\n## ExtraData\n\n" );

			if (! general.empty( ) ) {


				uint32_t index = 0;
				wprintf( L"\n* DataBlock一覧\n\n\t```\n" );

				for ( auto item : general  ) {

					wprintf( L"\tDataBlock#%u (Offset : 0x%08I64x) :\n", index ,  CalcOffsetBytes( pHeader, item ) );

					wprintf( L"\t\tBlockSize : %u (0x%08x)\n", item->BlockSize, item->BlockSize );
					wprintf( L"\t\tBlockSignature : 0x%08x", item->BlockSignature );

					switch ( item->BlockSignature ) {
						using namespace hirosof::LinkFile;
						case ExtraData_ConsoleDataBlockSignature:wprintf( L" (ConsoleDataBlock)" ); break;
						case ExtraData_ConsoleFEDataBlockSignature:wprintf( L" (ConsoleFEDataBlock)" ); break;
						case ExtraData_DarwinDataBlockSignature:wprintf( L" (DarwinDataBlock)" ); break;
						case ExtraData_EnvironmentVariableDataBlockSignature:wprintf( L" (EnvironmentVariableDataBlock)" ); break;
						case ExtraData_IconEnvironmentDataBlockSignature:wprintf( L" (IconEnvironmentDataBlock)" ); break;
						case ExtraData_KnownFolderDataBlockSignature:wprintf( L" (KnownFolderDataBlock)" ); break;
						case ExtraData_PropertyStoreDataBlockSignature:wprintf( L" (PropertyStoreDataBlock)" ); break;
						case ExtraData_ShimDataBlockSignature:wprintf( L" (ShimDataBlock) [Unsupported]" ); break;
						case ExtraData_SpecialFolderDataBlockSignature:wprintf( L" (SpecialFolderDataBlock)" ); break;
						case ExtraData_TrackerDataBlockSignature:wprintf( L" (TrackerDataBlock)" ); break;
						case ExtraData_VistaAndAboveIDListDataBlockSignature:wprintf( L" (VistaAndAboveIDListDataBlock)" ); break;
					}

					wprintf( L"\n\n" );
					index++;
				}

				wprintf( L"\t```\n" );
				ShowExtraData( &link );
			}


		}
	}

}

void ShowItemIDList( const ITEMIDLIST* pIDList, uint32_t IDListSize ) {
	if ( pIDList == nullptr ) return;
	const uint8_t* pData = reinterpret_cast<const uint8_t*>( pIDList );

	HRESULT  hr;

	wchar_t  path[MAX_PATH] = L"";


	if( dumpMode == DumpMode::None )
		wprintf( L"IDList Size ： %u (0x%08x)\n\n", IDListSize, IDListSize );


#if defined(__SHOW_SHGETPATHFROMIDLIST_RESULT__) 
	if ( SHGetPathFromIDListW( pIDList, path ) ) {
		wprintf( L"IDList Parsed Result (by SHGetPathFromIDListW) : \n\t%s\n\n", path );
	}
#endif


	wchar_t* pDisplayName;
	bool  bFirstShow_ShellItemResult = true;


#if defined(__SHOW_SHELLITEM_GETDISPLAYNAME_RESULT__) 

	IShellItem* pItem;
	hr = SHCreateItemFromIDList( pIDList, IID_PPV_ARGS( &pItem ) );
	if ( SUCCEEDED( hr ) ) {

		for ( size_t i = 0; i < ( sizeof( sigdn_values ) / sizeof( SIGDN ) ); i++ ) {
			hr = pItem->GetDisplayName( sigdn_values[i], &pDisplayName );
			if ( SUCCEEDED( hr ) ) {

				if ( bFirstShow_ShellItemResult ) {
					wprintf( L"IDList Parsed Result (by IShellItem::GetDisplayName) : \n" );
					bFirstShow_ShellItemResult = false;
				}

				wprintf( L"\t%s : \n\t\t%s\n\n", sigdn_values_shownames[i], pDisplayName );
				CoTaskMemFree( pDisplayName );
			}
		}

		pItem->Release( );
	}

	bFirstShow_ShellItemResult = true;
#endif

	for ( size_t i = 0; i < ( sizeof( sigdn_values ) / sizeof( SIGDN ) ); i++ ) {
		hr = SHGetNameFromIDList( pIDList, sigdn_values[i], &pDisplayName );
		if ( SUCCEEDED( hr ) ) {

			if ( bFirstShow_ShellItemResult ) {
				wprintf( L"IDList Parsed Result (by SHGetNameFromIDList) : \n" );
				bFirstShow_ShellItemResult = false;
			}

			wprintf( L"\t%s : \n\t\t%s\n\n", sigdn_values_shownames[i], pDisplayName );
			CoTaskMemFree( pDisplayName );
		}
	}

	int numberOfLineElements = 16;

	switch ( dumpWidthMode ) {
		case DumpWidthMode::Normal:
			numberOfLineElements = 16;
			break;
		case DumpWidthMode::Extended:
			numberOfLineElements = 32;
			break;
		case DumpWidthMode::Custom:
			numberOfLineElements = dumpWidthCustomValue;
			break;
		default:
			numberOfLineElements = 16;
			break;
	}

	if ( ( dumpMode == DumpMode::Hex ) || ( dumpMode == DumpMode::Both ) ) {
		wprintf( L"IDList Binary Hex Dump (%u bytes): \n\t", IDListSize );
		for ( size_t i = 1; i <= IDListSize; i++ ) {
			wprintf( L"%02X ", *( pData + i - 1 ) );


			if ( ( i % numberOfLineElements ) == 0 ) wprintf( L"\n\t" );
		}

		wprintf( L"\n\n" );

	}

	if ( ( dumpMode == DumpMode::Array ) || ( dumpMode == DumpMode::Both ) ) {
		wprintf( L"IDList Binary Dump Array(%u bytes): \n\t", IDListSize );

		wprintf( L"const size_t NumberOfIDListDataElements = %u;\n\t", IDListSize );
		wprintf( L"const uint8_t IDListData[NumberOfIDListDataElements] = {\n\t\t" );

		switch ( dumpWidthMode ) {
			case DumpWidthMode::Normal:
				numberOfLineElements = 8;
				break;
			case DumpWidthMode::Extended:
				numberOfLineElements = 16;
				break;
			case DumpWidthMode::Custom:
				numberOfLineElements = dumpWidthCustomValue;
				break;
			default:
				numberOfLineElements = 8;
				break;
		}

		for ( size_t i = 1; i <= IDListSize; i++ ) {
			wprintf( L"0x%02X", *( pData + i - 1 ) );
			if ( i != IDListSize ) {
				wprintf( L", " );
				if ( ( i % numberOfLineElements ) == 0 ) wprintf( L"\n\t\t" );
			}
		}
		wprintf( L"\n\t};\n\n" );
	}

}

void ShowExtraData( hirosof::LinkFile::CLinkFileParser* pLink ) {

	if ( pLink == nullptr ) return;

	if ( pLink->hasExtraData( ) == false ) return;

	auto  pHeader = pLink->getHeaderPtr( );

	// ConsoleDataBlock
	if ( pLink->getExtraConsoleDataBlockPtr( ) != nullptr ) {
		const hirosof::LinkFile::ExtraData_ConsoleDataBlock* pBlock = pLink->getExtraConsoleDataBlockPtr( );
		wprintf( L"\n### ConsoleDataBlock (Offset : 0x%08I64x) \n\n```\n", CalcOffsetBytes( pHeader, pBlock ) );

		wprintf( L"BlockSize : %u (0x%08x)\n", pBlock->BlockSize, pBlock->BlockSize );
		wprintf( L"BlockSignature : 0x%08x\n\n", pBlock->BlockSignature );


		wprintf( L"FillAttributes : %u (0x%04x)\n", pBlock->FillAttributes, pBlock->FillAttributes );
		if ( pBlock->FillAttributes & FOREGROUND_BLUE ) wprintf( L"\tFOREGROUND_BLUE (0x%04x)\n", FOREGROUND_BLUE );
		if ( pBlock->FillAttributes & FOREGROUND_GREEN ) wprintf( L"\tFOREGROUND_GREEN (0x%04x)\n", FOREGROUND_GREEN );
		if ( pBlock->FillAttributes & FOREGROUND_RED ) wprintf( L"\tFOREGROUND_RED (0x%04x)\n", FOREGROUND_RED );
		if ( pBlock->FillAttributes & FOREGROUND_INTENSITY ) wprintf( L"\tFOREGROUND_INTENSITY (0x%04x)\n", FOREGROUND_INTENSITY );
		if ( pBlock->FillAttributes & BACKGROUND_BLUE ) wprintf( L"\tBACKGROUND_BLUE (0x%04x)\n", BACKGROUND_BLUE );
		if ( pBlock->FillAttributes & BACKGROUND_GREEN ) wprintf( L"\tBACKGROUND_GREEN (0x%04x)\n", BACKGROUND_GREEN );
		if ( pBlock->FillAttributes & BACKGROUND_RED ) wprintf( L"\tBACKGROUND_RED (0x%04x)\n", BACKGROUND_RED );
		if ( pBlock->FillAttributes & BACKGROUND_INTENSITY ) wprintf( L"\tBACKGROUND_INTENSITY (0x%04x)\n", BACKGROUND_INTENSITY );
		if ( pBlock->FillAttributes != 0 )	wprintf( L"\n" );

		wprintf( L"PopupFillAttributes : %u (0x%04x)\n", pBlock->PopupFillAttributes, pBlock->PopupFillAttributes );
		if ( pBlock->PopupFillAttributes & FOREGROUND_BLUE ) wprintf( L"\tFOREGROUND_BLUE (0x%04x)\n", FOREGROUND_BLUE );
		if ( pBlock->PopupFillAttributes & FOREGROUND_GREEN ) wprintf( L"\tFOREGROUND_GREEN (0x%04x)\n", FOREGROUND_GREEN );
		if ( pBlock->PopupFillAttributes & FOREGROUND_RED ) wprintf( L"\tFOREGROUND_RED (0x%04x)\n", FOREGROUND_RED );
		if ( pBlock->PopupFillAttributes & FOREGROUND_INTENSITY ) wprintf( L"\tFOREGROUND_INTENSITY (0x%04x)\n", FOREGROUND_INTENSITY );
		if ( pBlock->PopupFillAttributes & BACKGROUND_BLUE ) wprintf( L"\tBACKGROUND_BLUE (0x%04x)\n", BACKGROUND_BLUE );
		if ( pBlock->PopupFillAttributes & BACKGROUND_GREEN ) wprintf( L"\tBACKGROUND_GREEN (0x%04x)\n", BACKGROUND_GREEN );
		if ( pBlock->PopupFillAttributes & BACKGROUND_RED ) wprintf( L"\tBACKGROUND_RED (0x%04x)\n", BACKGROUND_RED );
		if ( pBlock->PopupFillAttributes & BACKGROUND_INTENSITY ) wprintf( L"\tBACKGROUND_INTENSITY (0x%04x)\n", BACKGROUND_INTENSITY );
		if ( pBlock->PopupFillAttributes != 0 )	wprintf( L"\n" );

		wprintf( L"ScreenBufferSizeX : %u (0x%04x)\n", pBlock->ScreenBufferSizeX, pBlock->ScreenBufferSizeX );
		wprintf( L"ScreenBufferSizeY : %u (0x%04x)\n", pBlock->ScreenBufferSizeY, pBlock->ScreenBufferSizeY );
		wprintf( L"\n" );

		wprintf( L"WindowSizeX : %u (0x%04x)\n", pBlock->WindowSizeX, pBlock->WindowSizeX );
		wprintf( L"WindowSizeY : %u (0x%04x)\n", pBlock->WindowSizeY, pBlock->WindowSizeY );
		wprintf( L"\n" );
		wprintf( L"WindowOriginX : %u (0x%04x)\n", pBlock->WindowOriginX, pBlock->WindowOriginX );
		wprintf( L"WindowOriginY : %u (0x%04x)\n", pBlock->WindowOriginY, pBlock->WindowOriginY );
		wprintf( L"\n" );



		wprintf( L"FontSize : %u (0x%08x)", pBlock->FontSize, pBlock->FontSize );
		if ( ( pBlock->FontSize & 0xFFFF ) == 0 ) wprintf( L" [Vector Font]" );
		wprintf( L"\n\tWidth : %u (0x%08x)\n", pBlock->FontSize & 0xFFFF, pBlock->FontSize & 0xFFFF );
		wprintf( L"\tHeight : %u (0x%08x)\n", ( pBlock->FontSize & 0xFFFF0000 ) >> 16, ( pBlock->FontSize & 0xFFFF0000 ) >> 16 );
		wprintf( L"\n" );
		wprintf( L"FontFamily : %u (0x%08x)\n", pBlock->FontFamily, pBlock->FontFamily );

		uint8_t ff_value = pBlock->FontFamily & 0xF0;
		uint8_t tmpf_value = pBlock->FontFamily & 0x0F;


		switch ( ff_value ) {
			case FF_DONTCARE: wprintf( L"\t%s (0x%04x)\n", L"FF_DONTCARE", ff_value ); break;
			case FF_ROMAN: wprintf( L"\t%s (0x%04x)\n", L"FF_ROMAN", ff_value ); break;
			case FF_SWISS: wprintf( L"\t%s (0x%04x)\n", L"FF_SWISS", ff_value ); break;
			case FF_MODERN: wprintf( L"\t%s (0x%04x)\n", L"FF_MODERN", ff_value ); break;
			case FF_SCRIPT: wprintf( L"\t%s (0x%04x)\n", L"FF_SCRIPT", ff_value ); break;
			case FF_DECORATIVE: wprintf( L"\t%s (0x%04x)\n", L"FF_DECORATIVE", ff_value ); break;
		}

		if ( tmpf_value == 0 ) {
			wprintf( L"\t%s (0x%04x)\n", L"TMPF_NONE", 0 );
		} else {
			if ( tmpf_value & TMPF_FIXED_PITCH ) wprintf( L"\t%s (0x%04x)\n", L"TMPF_FIXED_PITCH", TMPF_FIXED_PITCH );
			if ( tmpf_value & TMPF_VECTOR ) wprintf( L"\t%s (0x%04x)\n", L"TMPF_VECTOR", TMPF_VECTOR );
			if ( tmpf_value & TMPF_TRUETYPE ) wprintf( L"\t%s (0x%04x)\n", L"TMPF_TRUETYPE", TMPF_TRUETYPE );
			if ( tmpf_value & TMPF_DEVICE ) wprintf( L"\t%s (0x%04x)\n", L"TMPF_DEVICE", TMPF_DEVICE );
		}

		wprintf( L"\n" );

		wprintf( L"FontWeight : %u (0x%08x) [%s]\n",
			pBlock->FontWeight,
			pBlock->FontWeight,
			( pBlock->FontWeight >= 700 ) ? L"Bold" : L"Regular" );

		wprintf( L"Face Name : %s\n\n", pBlock->FaceName );

		wprintf( L"CursorSize : %u (0x%08x)", pBlock->CursorSize, pBlock->CursorSize );

		if ( pBlock->CursorSize <= 25 ) {
			wprintf( L" [Small]\n" );
		} else if ( pBlock->CursorSize <= 50 ) {
			wprintf( L" [Medium]\n" );

		} else if ( pBlock->CursorSize <= 100 ) {
			wprintf( L" [Large]\n" );

		} else {
			wprintf( L"\n" );
		}
		wprintf( L"FullScreen : %u (0x%08x) [%s]\n",
			pBlock->FullScreen,
			pBlock->FullScreen,
			( pBlock->FullScreen != 0) ? L"On" : L"Off" );
		
		wprintf( L"QuickEdit  : %u (0x%08x) [%s]\n",
			pBlock->QuickEdit,
			pBlock->QuickEdit,
			( pBlock->QuickEdit != 0) ? L"On" : L"Off" );

		wprintf( L"InsertMode : %u (0x%08x) [%s]\n",
			pBlock->InsertMode,
			pBlock->InsertMode,
			( pBlock->InsertMode != 0 ) ? L"Enable" : L"Disable" );

		wprintf( L"AutoPosition : %u (0x%08x) [%s]\n",
			pBlock->AutoPosition,
			pBlock->AutoPosition,
			( pBlock->AutoPosition != 0 ) ? L"Automatically" : L"Use WindowOriginX and WindowOriginY" );

		wprintf( L"HistoryBufferSize : %u (0x%08x)\n", pBlock->HistoryBufferSize, pBlock->HistoryBufferSize );
		wprintf( L"NumberOfHistoryBuffers : %u (0x%08x)\n", pBlock->NumberOfHistoryBuffers, pBlock->NumberOfHistoryBuffers );
		wprintf( L"HistoryNoDup  : %u (0x%08x) [%s]\n",
			pBlock->HistoryNoDup,
			pBlock->HistoryNoDup,
			( pBlock->HistoryNoDup != 0 ) ? L"Duplicates are allowed." : L"Duplicates are not allowed." );


		wprintf( L"\n" );


		wprintf( L"ColorTable  : \n" );
		

		for ( uint32_t color : pBlock->ColorTable ) {
			wprintf( L"\t0x%08X (%u)\n", color, color );
		}

		wprintf( L"```\n" );
	}

	// ConsoleFEDataBlock
	if ( pLink->getExtraConsoleFEDataBlockPtr( ) != nullptr ) {
		const hirosof::LinkFile::ExtraData_ConsoleFEDataBlock* pBlock = pLink->getExtraConsoleFEDataBlockPtr( );
		wprintf( L"\n### ConsoleFEDataBlock (Offset : 0x%08I64x) \n\n```\n", CalcOffsetBytes( pHeader, pBlock ) );

		wprintf( L"BlockSize : %u (0x%08x)\n", pBlock->BlockSize, pBlock->BlockSize );
		wprintf( L"BlockSignature : 0x%08x\n\n", pBlock->BlockSignature );

		wprintf( L"CodePage : %u (0x%08x)\n", pBlock->CodePage, pBlock->CodePage );

		wprintf( L"```\n" );
	}
	// 	DarwinDataBlock
	if ( pLink->getExtraDarwinDataBlockPtr( ) != nullptr ) {
		const hirosof::LinkFile::ExtraData_DarwinDataBlock* pBlock = pLink->getExtraDarwinDataBlockPtr( );
		wprintf( L"\n### DarwinDataBlock (Offset : 0x%08I64x) \n\n```\n", CalcOffsetBytes( pHeader, pBlock ) );
		wprintf( L"BlockSize : %u (0x%08x)\n", pBlock->BlockSize, pBlock->BlockSize );
		wprintf( L"BlockSignature : 0x%08x\n\n", pBlock->BlockSignature );
		wprintf( L"DarwinDataAnsi (Ignore Field) : \n\t%S\n\n", pBlock->DarwinDataAnsi );
		wprintf( L"DarwinDataUnicode : \n\t%s\n", pBlock->DarwinDataUnicode );
		wprintf( L"```\n" );
	}

	// 	EnvironmentVariableDataBlock
	if ( pLink->getExtraEnvironmentVariableDataBlockPtr( ) != nullptr ) {
		const hirosof::LinkFile::ExtraData_EnvironmentVariableDataBlock* pBlock = pLink->getExtraEnvironmentVariableDataBlockPtr( );
		wprintf( L"\n### EnvironmentVariableDataBlock (Offset : 0x%08I64x) \n\n```\n", CalcOffsetBytes( pHeader, pBlock ) );

		wprintf( L"BlockSize : %u (0x%08x)\n", pBlock->BlockSize, pBlock->BlockSize );
		wprintf( L"BlockSignature : 0x%08x\n\n", pBlock->BlockSignature );

		wprintf( L"TargetAnsi : \n\t%S\n\n", pBlock->TargetAnsi );
		wprintf( L"TargetUnicode : \n\t%s\n", pBlock->TargetUnicode );
		wprintf( L"```\n" );
	}


	//	IconEnvironmentDataBlock
	if ( pLink->getExtraIconEnvironmentDataBlockPtr( ) != nullptr ) {
		const hirosof::LinkFile::ExtraData_IconEnvironmentDataBlock* pBlock = pLink->getExtraIconEnvironmentDataBlockPtr( );
		wprintf( L"\n### IconEnvironmentDataBlock (Offset : 0x%08I64x) \n\n```\n", CalcOffsetBytes( pHeader, pBlock ) );

		wprintf( L"BlockSize : %u (0x%08x)\n", pBlock->BlockSize, pBlock->BlockSize );
		wprintf( L"BlockSignature : 0x%08x\n\n", pBlock->BlockSignature );

		wprintf( L"TargetAnsi : \n\t%S\n\n", pBlock->TargetAnsi );
		wprintf( L"TargetUnicode : \n\t%s\n", pBlock->TargetUnicode );
		wprintf( L"```\n" );
	}

	// 	KNOWN_FOLDER_PROPS: A KnownFolderDataBlock structure (section 2.5.6).
	if ( pLink->getExtraKnownFolderDataBlockPtr( ) != nullptr ) {
		const hirosof::LinkFile::ExtraData_KnownFolderDataBlock* pBlock = pLink->getExtraKnownFolderDataBlockPtr( );
		wprintf( L"\n### KnownFolderDataBlock (Offset : 0x%08I64x) \n\n```\n", CalcOffsetBytes( pHeader, pBlock ) );
		wprintf( L"BlockSize : %u (0x%08x)\n", pBlock->BlockSize, pBlock->BlockSize );
		wprintf( L"BlockSignature : 0x%08x\n\n", pBlock->BlockSignature );
		wprintf( L"KnownFolderID : " );
		PrintlnCLSID( pBlock->KnownFolderID );
		wprintf( L"Offset : %u (0x%08x)\n", pBlock->Offset, pBlock->Offset );
		wprintf( L"```\n" );

		const std::wstring name = hirosof::Supports::KnownFolders::getOriginalNameFromFolderID( pBlock->KnownFolderID );
		if ( name.empty( ) == false ) wprintf( L"\n* Appendix\n\t- KnownFolderID (MacroName) ： %s\n\n", name.c_str( ) );

	}

	// 	PropertyStoreDataBlock
	if ( pLink->getExtraPropertyStoreDataBlock( ).pHeader ) {
		using hirosof::Supports::PropertyStoreParse::ParsePropertyStore;
		using hirosof::Supports::PropertyStoreParse::PropertyStoreData;

		const hirosof::LinkFile::ExtraData_PropertyStoreDataBlock block = pLink->getExtraPropertyStoreDataBlock( );
		wprintf( L"\n### PropertyStoreDataBlock (Offset : 0x%08I64x) \n\n```\n", CalcOffsetBytes( pHeader, block.pHeader ) );

		wprintf( L"BlockSize : %u (0x%08x)\n", block.pHeader->BlockSize, block.pHeader->BlockSize );
		wprintf( L"BlockSignature : 0x%08x\n\n", block.pHeader->BlockSignature );

		wprintf( L"DataSize : %u (0x%08x)\n\n", block.dataSize, block.dataSize );
			

		hirosof::Supports::PropertyStoreParse::PropertyStoreData data_m;
		HRESULT hr = hirosof::Supports::PropertyStoreParse::ParsePropertyStore( &data_m, block.pData, block.dataSize );

		if ( hr == S_OK ) {
			for ( auto data : data_m.storageData ) {
				wprintf( L"storage offset : 0x%08I64x\n",
					CalcOffsetBytes( pHeader, data.pStorageTop )
					);
					wprintf( L"\tstorage size : %u\n",
					data.storageSize );
				wprintf( L"\tstorage type : %u\n",
					data.formatType );
				wprintf( L"\tstorage guid : " );
				PrintlnCLSID( data.formatID );
			}
			
		}
		wprintf( L"hr : 0x%08x\n\n", hr );

		wprintf( L"```\n" );

		wprintf( L"\n* ※ データ部分は未パース\n" );
	}

	// 	SHIM_PROPS: A ShimDataBlock structure (section 2.5.8).
	// サンプルデータの作成/用意 方法が不明の為非対応

	// SpecialFolderDataBlock
	if ( pLink->getExtraSpecialFolderDataBlockPtr( ) != nullptr ) {
		const hirosof::LinkFile::ExtraData_SpecialFolderDataBlock* pBlock = pLink->getExtraSpecialFolderDataBlockPtr( );
		wprintf( L"\n### SpecialFolderDataBlock (Offset : 0x%08I64x) \n\n```\n", CalcOffsetBytes( pHeader, pBlock ) );
		wprintf( L"BlockSize : %u (0x%08x)\n", pBlock->BlockSize, pBlock->BlockSize );
		wprintf( L"BlockSignature : 0x%08x\n\n", pBlock->BlockSignature );
		wprintf( L"SpecialFolderID : %u (0x%08x)\n", pBlock->SpecialFolderID, pBlock->SpecialFolderID );
		wprintf( L"Offset : %u (0x%08x)\n", pBlock->Offset, pBlock->Offset );
		wprintf( L"```\n" );
		
	}
	
	// 	TrackerDataBlock
	if ( pLink->getExtraTrackerDataBlockPtr( ) ) {
		const hirosof::LinkFile::ExtraData_TrackerDataBlock* pBlock = pLink->getExtraTrackerDataBlockPtr( );
		wprintf( L"\n### TrackerDataBlock (Offset : 0x%08I64x) \n\n```\n", CalcOffsetBytes( pHeader, pBlock ) );
		wprintf( L"BlockSize : %u (0x%08x)\n", pBlock->BlockSize, pBlock->BlockSize );
		wprintf( L"BlockSignature : 0x%08x\n", pBlock->BlockSignature );
		wprintf( L"Length : %u (0x%08x)\n\n", pBlock->Length, pBlock->Length );
		wprintf( L"MachineID : %S\n\n", pBlock->MachineID );

		wprintf( L"Droid : \n" );
		for ( auto id : pBlock->Droid ) {
			wprintf( L"\t" );
			PrintlnCLSID( id );
		}
		wprintf( L"\nDroidBirth : \n" );
		for ( auto id : pBlock->DroidBirth ) {
			wprintf( L"\t" );
			PrintlnCLSID( id );
		}
		wprintf( L"```\n" );

	}


	// VistaAndAboveIDListDataBlock
	if ( pLink->getExtraVistaAndAboveIDListDataBlock( ).pHeader ) {

		const hirosof::LinkFile::ExtraData_VistaAndAboveIDListDataBlock block = pLink->getExtraVistaAndAboveIDListDataBlock( );
		wprintf( L"\n### VistaAndAboveIDListDataBlock (Offset : 0x%08I64x) \n\n```\n", CalcOffsetBytes( pHeader, block.pHeader ) );

		wprintf( L"BlockSize : %u (0x%08x)\n", block.pHeader->BlockSize, block.pHeader->BlockSize );
		wprintf( L"BlockSignature : 0x%08x\n\n", block.pHeader->BlockSignature );
		ShowItemIDList( block.pIDList, block.IDListSize );
		wprintf( L"```\n" );
	}

}
void PrintCLSID( const CLSID clsid ) {
	LPOLESTR pstr;
	if ( StringFromCLSID( clsid, &pstr ) == S_OK ) {
		wprintf(L"%s", pstr );
		CoTaskMemFree( pstr );
	} else {
		wprintf(L"{%08X-%04X-%04X-", clsid.Data1, clsid.Data2, clsid.Data3 );
		for ( size_t i = 0; i < 8; i++ ) {
			wprintf(L"%02X", clsid.Data4[i] );
			if ( i == 1 )wprintf(L"-" );
		}
		wprintf(L"}" );		
	}
}
void PrintFILETIME( const FILETIME  filetime ) {
	FILETIME local_ft;
	SYSTEMTIME st,st_local;
	BOOL bTrans;
	if ( FileTimeToSystemTime( &filetime, &st ) ) {
		wprintf(L"%04d/%02d/%02d %02d:%02d:%02d (UTC)", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );

		bTrans = FileTimeToLocalFileTime( &filetime, &local_ft );
		if(bTrans )	bTrans = FileTimeToSystemTime( &local_ft, &st_local );

		if ( bTrans ) {
			wprintf(L" = %04d/%02d/%02d %02d:%02d:%02d (Local)", st_local.wYear, st_local.wMonth, st_local.wDay, 
				st_local.wHour, st_local.wMinute, st_local.wSecond );
		}
	} else {
		ULARGE_INTEGER uli;
		uli.HighPart = filetime.dwHighDateTime;
		uli.LowPart = filetime.dwLowDateTime;
		wprintf(L"0x%I64x = %I64u (Raw UTC)", uli.QuadPart , uli.QuadPart );
	}
}


void PrintlnCLSID( const CLSID clsid ) {
	PrintCLSID( clsid );
	wprintf(L"\n" );
}
void PrintlnFILETIME( const FILETIME  filetime ) {
	PrintFILETIME( filetime );
	wprintf(L"\n" );
}
