// 以下の ifdef ブロックは DLL から簡単にエクスポートさせるマクロを作成する標準的な方法です。 
// この DLL 内のすべてのファイルはコマンドラインで定義された FMFSLIB_EXPORTS シンボル
// でコンパイルされます。このシンボルはこの DLL が使用するどのプロジェクト上でも未定義でなけ
// ればなりません。この方法ではソースファイルにこのファイルを含むすべてのプロジェクトが DLL 
// からインポートされたものとして FMFSLIB_API 関数を参照し、そのためこの DLL はこのマク 
// ロで定義されたシンボルをエクスポートされたものとして参照します。
#ifdef FMFSLIB_EXPORTS
#define FMFSLIB_API __declspec(dllexport)
#else
#define FMFSLIB_API __declspec(dllimport)
#endif

// このクラスは FMFSLIB.dll からエクスポートされます
//class FMFSLIB_API CFMFSLIB {
//public:
//	CFMFSLIB(void);
//	// TODO: この位置にメソッドを追加してください。
//};

//extern FMFSLIB_API int nFMFSLIB;

//FMFSLIB_API int fnFMFSLIB(void);
