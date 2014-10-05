
#include "main.h"
#include "utils.h"

int _fltused = 0;
MEMORY_STATS g_MemStats = { 0 };


//++ UtilsInitialize
VOID UtilsInitialize()
{
	TRACE( _T( "  UtilsInitialize()\n" ) );
}

//++ UtilsDestroy
VOID UtilsDestroy()
{
	TRACE( _T( "  MyAlloc: %u calls, %I64u bytes\n" ), g_MemStats.AllocCalls, g_MemStats.AllocBytes );
	TRACE( _T( "  MyFree:  %u calls, %I64u bytes\n" ), g_MemStats.FreeCalls, g_MemStats.FreeBytes );
	TRACE( _T( "  UtilsDestroy()\n" ) );
}

//++ TraceImpl
#if DBG || _DEBUG
VOID TraceImpl( __in LPCTSTR pszFormat, ... )
{
	DWORD err = ERROR_SUCCESS;
	if ( pszFormat && *pszFormat ) {

		TCHAR szStr[1024];
		int iLen1, iLen2;
		va_list args;

		iLen1 = wnsprintf( szStr, (int)ARRAYSIZE( szStr ), _T( "[xfer.th%04x] " ), GetCurrentThreadId() );

		va_start( args, pszFormat );
		iLen2 = wvnsprintf( szStr + iLen1, (int)ARRAYSIZE( szStr ) - iLen1, pszFormat, args );
		if ( iLen2 > 0 ) {
			if ( iLen1 + iLen2 < ARRAYSIZE( szStr ) )
				szStr[iLen1 + iLen2] = 0;	/// The string is not guaranteed to be null terminated
		} else {
			szStr[ARRAYSIZE( szStr ) - 1] = 0;
		}
		OutputDebugString( szStr );
		va_end( args );
	}
}
#endif

//++ GetLocalFileTime
BOOL GetLocalFileTime( _Out_ LPFILETIME lpFT )
{
	if ( lpFT ) {
		SYSTEMTIME st;
		GetLocalTime( &st );
		return SystemTimeToFileTime( &st, lpFT );
	}
	return FALSE;
}

//++ AllocErrorStr
VOID AllocErrorStr( _In_ DWORD dwErrCode, _Out_ TCHAR **ppszErrText )
{
	if ( ppszErrText ) {

		DWORD dwLen;
		TCHAR szTextError[512];
		HMODULE hModule = NULL;
		DWORD dwExtraFlags = 0;

		if ( dwErrCode >= INTERNET_ERROR_BASE && dwErrCode <= INTERNET_ERROR_LAST ) {
			hModule = GetModuleHandle( _T( "wininet.dll" ) );
			dwExtraFlags = FORMAT_MESSAGE_FROM_HMODULE;
		} else {
			dwExtraFlags = FORMAT_MESSAGE_FROM_SYSTEM;
		}

		szTextError[0] = 0;
		dwLen = FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS | dwExtraFlags, hModule, dwErrCode, 0, szTextError, ARRAYSIZE( szTextError ), NULL );
		if ( dwLen > 0 ) {
			StrTrim( szTextError, _T( ". \r\n" ) );
			MyStrDup( *ppszErrText, szTextError );
		}
	}
}


/// ULONLONG <-> double conversion groups
///	ULLONG_MAX == 18446744073709551615
ULONGLONG ULONGLONG_groups[19] = { 1000000000000000000, 100000000000000000, 10000000000000000, 1000000000000000, 100000000000000, 10000000000000, 1000000000000, 100000000000, 10000000000, 1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1 };
double double_groups[19] = { 1000000000000000000.0F, 100000000000000000.0F, 10000000000000000.0F, 1000000000000000.0F, 100000000000000.0F, 10000000000000.0F, 1000000000000.0F, 100000000000.0F, 10000000000.0F, 1000000000.0F, 100000000.0F, 10000000.0F, 1000000.0F, 100000.0F, 10000.0F, 1000.0F, 100.0F, 10.0F, 1.0F };


//++ MyUlonglongToDouble
double MyUlonglongToDouble( __in ULONGLONG ull )
{
	double d = 0.0F;
	int i;
	for (i = 0; i < 19; i++) {
		while (ull >= ULONGLONG_groups[i]) {
			d += double_groups[i];
			ull -= ULONGLONG_groups[i];
		}
	}
	return d;
}


//++ MyDoubleToUlonglong
ULONGLONG MyDoubleToUlonglong( __in double d )
{
	ULONGLONG ull = 0;
	int i;
	for (i = 0; i < 19; i++) {
		while (d >= double_groups[i]) {
			ull += ULONGLONG_groups[i];
			d -= double_groups[i];
		}
	}
	return ull;
}

//++ MyDiv64
ULONGLONG MyDiv64( __in ULONGLONG iNumerator, __in ULONGLONG iDenominator )
{
/*	ULONGLONG iQuotient = 0;
	if (iDenominator != 0) {
		while (iNumerator >= iDenominator) {
			iNumerator -= iDenominator;
			iQuotient++;
		}
	}
	return iQuotient;*/

	if (iDenominator == 0)
		iDenominator = 1;
	return MyDoubleToUlonglong( MyUlonglongToDouble( iNumerator ) / MyUlonglongToDouble( iDenominator ) );
}


//++ MyMulDiv64
ULONGLONG MyMulDiv64( __in ULONGLONG iNumber, __in ULONGLONG iNumerator, __in ULONGLONG iDenominator )
{
	/// iNumber*(iNumerator/iDenominator)
	return MyDoubleToUlonglong( MyUlonglongToDouble( iNumber ) * (MyUlonglongToDouble( iNumerator ) / MyUlonglongToDouble( iDenominator )) );
}


//++ MyTimeDiff
ULONG MyTimeDiff( __in PFILETIME pEndTime, __in PFILETIME pStartTime )
{
	/// NOTE: Large integer operations are not available because we're not linking to CRT
	if (pStartTime && pEndTime) {
		ULONGLONG iDiff = MyDiv64( ((PULARGE_INTEGER)pEndTime)->QuadPart - ((PULARGE_INTEGER)pStartTime)->QuadPart, 10000 );
		if (iDiff < UINT_MAX)
			return (ULONG)iDiff;
		else
			return UINT_MAX;			/// ~49 days
	}
	return 0;
}
