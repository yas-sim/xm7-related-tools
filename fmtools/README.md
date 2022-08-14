# 当時ホームページに載せていた内容をマークダウン用に修正
---
 【警告】 必ずお読みください 



FTOOLSに含まれているコマンド群は現在α版のものです。一切の動作を保証できませんので、ご使用になられる場合は「何が起こっても不思議ではない」ことを覚悟の上でご使用ください。万一ツールを使用したことによるデータの一部またはすべての消失、および、発生しうるすべての損害に対して作者は一切の責任を負わないものとします。
以上の事項を承諾いただけない場合、FTOOLSを使用することはできません。


[同意します（ダウンロード V0.0）](bin/ftools_00.zip)139KB




　

　
---
●FTOOLS  
●FM-FILE形式  
●FMDIR  
●FMREAD  
●FMDECODE  
●FMENCODE  
●FMWRITE  
●FMCOPY  
●D77DMP  
●注意事項など  
---
●FTOOLS

FTOOLSはXM7で正式に採用されているD77ディスクイメージフォーマットファイル用のユーティリティー群に与えられた名前です。FTOOLSはさまざまなコマンドを含んでいます。すべてのコマンドはCUIベースのとてもシンプルなものです。余談ですが、LinuxのMTOOLSをパクってネーミングしてます(^^;

FTOOLSを使用することで、DOS/V機のクロスアセンブラでアセンブルして作成したオブジェクトや、テキストエディタで入力したBASICプログラムを簡単にXM7上に持ってくることができます。一度DOS/V上で扱えるファイル形式に変換できますので、さまざまな応用例が考えられます(ホントか?)

FTOOLSが含んでいるコマンドは現在のところ以下のものがあります。

|||
|-|-|
|FMREAD|D77イメージファイルから指定のファイルを読み出す。<br>読み出したファイルはFM-FILEフォーマットとなる。|
|FMDECODE|FM-FILEフォーマットのファイルをデコードしてDOS/V標準ファイルに変換する(.TXT, .MOT, .BINなど)|
|FMENCODE|DOS/V標準ファイルをFM-FILEフォーマットに変換する<br>DOS/Vファイルの拡張子により変換後のファイル属性が決定される|
|FMWRITE|FM-FILEフォーマットファイルをD77イメージファイルに書き込む|
|FMDIR|D77フォーマットファイルに格納されているファイルのカタログを表示する|
|FMCOPY|D77フォーマット間でファイルのコピーを行う|
|D77DMP|D77ファーマットファイルのセクタダンプを表示する(補助ツール)<br>これらのコマンドを使用することで次のようなことが行えます。|

||||||||||
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|D77IMG|→|FM-FILE|→|DOS/V FILE|→|FM-FILE|→|D77IMG|
||↑||↑||↑||↑||	
||FMREAD||FMDECODE||FMENCODE||FMWRITE|	

　
||||||||||
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|D77IMG|→|D77IMG|||||||
||↑||||||||	
||FMCOPY||||||||	
　

||||||||||
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|D77IMG|→|FM-FILE|→|インターネット|→|FM-FILE|→|D77IMG|
||↑||↑||↑||↑|	
||FMREAD||MAILER||MAILER||FMWRITE|	
　

●FM-FILEとは

FM-FILEとはFTOOLSで使用するオリジナルフォーマットです(^^;   これは、

「16バイトのヘッダ」+「F-BASICディスク内のクラスタイメージRAWデータ」

にほぼ等しいものです。16バイトのヘッダの中にはD77ディスクイメージディスクに書かれていたときのファイル名や、属性、FM-FILEであることをあらわすIDなどが格納されています。

このファイルの中身を見ることにあまり意味はありません。FMREADコマンドはクラスタ内データをすべて読み出しますので、最終クラスタの未使用セクタの内容なども含まれます。このデータから、本当に必要なデータのみを抜き出すのがFMDECODEコマンドです。

D77イメージファイルから取り出したファイルの中身が必要な場合、FMDECODEコマンドを使用してヘッダを取り除いた状態に変換してから使用してください。

●FMDIR

機能： D77ファイルの中に格納されているF-BASICファイル一覧を表示する

書式： `FMDIR D77_IMG [options]`

オプション：

-C	クラスタチェイン表示モード。どのクラスタが使用されているのか分かるようにクラスタチェインを表示する
-W	横にたくさんファイル一覧を表示する
詳細：　基本的にはDISK-BASICのFILESコマンドの形式のとおりに表示を行います。今のバージョンではフリークラスタ数の表示機能はありません。

実行例:
```sh
C:\>FMDIR test.d77 -c
MONBOOT  0 B 0 7<2> 1
EXMON    2 B 0 8-9<5> 2
kanji    0 B 0 6<1> 1         ↓最終クラスタ
DUMPA    0 A 0 10-11-12-13-14-15<2> 6
KASAHARA 0 A 0 16-17<7> 2        ↑最終クラスタで使用されているセクタ数
               ↑先頭クラスタ
```

クラスタチェインの最後にある<>で囲まれた数字は、最終ラスタ内の何セクタが使用済みかを示します。

●FMREAD

機能： D77ファイルから、F-BASICファイルを読み出す

書式： `FMREAD D77_IMG FBASIC_FILE [options]`

オプション：

-V	Verboseモード。変換後の出力ファイル名などを表示する
詳細： 
読み出されたファイルはFM-FILE形式という独自形式ファイルとして作成されます。
このとき、出力ファイルのファイル名はF-BASICファイルのベースファイル名と属性から決定されます。
出力ファイル名は、
　「F-BASICファイル名」+「.」+「ファイルタイプ」+「アスキーフラグ」+「ランダムフラグ」
となります。たとえばマシン語のTESTというファイルの場合、TEST.2B0となります。


実行例：
```sh
C:\>FMREAD TEST.D77 DUMPA -V
Output file='DUMPA.0A0'
```

●FMDECODE

機能：FM-FILE形式ファイルを、DOS/V標準ファイル形式に変換する

書式： `FMDECODE FM-FILE [options]`

オプション： 

-V	Verboseモード。変換後の出力ファイル名などを表示する
詳細： FM-FILE形式ファイルを.TXTや.MOT(モトローラSフォーマット)等の、DOS/V標準(?)ファイルに変換します。F-BASICの時のファイル属性を利用して出力ファイル形式を決定します。FM-FILEに格納されているIDを使用してファイル属性を取得するので、入力ファイルの拡張子は出力ファイルの形式に関係ありません。変換される形式は以下のとおり。

|||||
|-|-|-|-|
|F-BASIC BINARY|0B0|xxxxxxxx.BIN|バイナリ|
|F-BASIC ASCII|0A0|xxxxxxxx.BAS|テキストファイル|
|F-BASIC DATA|1A0|xxxxxxxx.TXT|テキストファイル|
|マシン語|2B0|xxxxxxxx.MOT|テキストファイル:Motorola-S format|
|その他全部|XXX|xxxxxxxx.BIN|バイナリ|

注意事項： FMDECODEコマンドは、FM-FILE形式が保持しているRAWクラスタデータから必要な部分を切り出してDOS/V上で普通に使えるファイルに変換します。しかし、F-BASICはエントリ内にファイルサイズなどを持っていなく、FATやエントリからは使用セクタ数しか分かりません。クラスタ内フォーマットはファイル形式ごとに独自に決まっており、ファイル形式別にファイルサイズを知る手段なども変わります。今、手元にはF-BASICのファイルフォーマットについて記述されている書籍、雑誌がないため、FMDECODEコマンドはXM7でいろいろファイルを作成してそれを解析した結果に基づいて変換を行います。中には何を意味するのかわからない部分もあり、そのような部分は飛ばして変換したりもしています。変換結果、生成されるファイルは「たぶん」問題ないと思いますが、もし何かトラブルがありましたらレポートいただけるとありがたいです(しかし長い…)。

実行例１：
```sh
C:\>FMDECODE DUMPA.0A0 -V
Output file='DUMPA.bas'
FileName=DUMPA FileType=0 Ascii=A Random=S
```

実行例２：
```sh
C:\>FMDECODE DPOKEM.2B0 -V
Output file='DPOKEm.mot'
FileName=DPOKEm FileType=2 Ascii=B Random=S
StartAddr=6000 EndAddr=60a6 Entry=6000
```

実行例２で生成されたファイル(`DPOKEm.MOT`)
```
S1236000338C25C67F9E3334103702A7805A26F99F338607B702033510BF0204338823FF63
S12360200210BD8F397E8E8243484149CE45524153C54C4C4953D44C5052494ED4534F550C
S12360404EC4504C41D944504F4BC581F427037E92A09DD281DB2607EE8C499DD22005BD26
S12360609A021F1334409DD82734812C270A813B2711817C271820D79DD2BD99BC3540E728
S1236080C020E19DD2BD9A023540AFC120D69DD2BD98F13540A680A7C05A26F920C635400D
S10A60A0EF8C013900000040
S90360009C
```

●FMENCODE

機能： DOS/V標準ファイルをFM-FILE形式に変換する

書式： `FMENCODE DOSV_FILE [options]`

オプション：

-V	Verboseモード。出力ファイル名などを表示する
詳細： DOS/V標準ファイルをFM-FILE形式に変換する。出力ファイル名は入力ファイルのベースファイル名と拡張子から決定される。また、出力ファイルのファイル名、拡張子、FM-FILEのID情報は入力ファイルのベースファイル名と拡張子により決定される。

||||
|-|-|-|
|xxxxxxxx.TXT|1 A 0|F-BASIC DATA|
|xxxxxxxx.MOT|2 B 0|マシン語|
|xxxxxxxx.S|2 B 0|マシン語|
|xxxxxxxx.BAS|0 A 0|F-BASIC PROGRAM(ASCII)|

実行例：
```sh
C:\>FMENCODE DPOKEM.MOT -V
fn=DPOKEM.MOT ext=MOT 2 B 0
FM-FILE file ='DPOKEM.2B0'
```

●FMWRITE

機能： FM-FILE形式のファイルをD77イメージファイルに書き込みます

書式： `FMWRITE D77_IMG FM-FILE`

オプション： なし

詳細： 書き込まれるファイル名、属性などはFM-FILE形式のIDに保存されている情報を元に書き込みます。入力ファイルのファイル名、拡張子は書き込みファイル名、属性に一切関係ありません。もし、同名のファイルが存在する場合、書き込みに先立ってファイルの削除を行います。FTOOLSのコマンドはD77フォーマットがサポートするライトプロテクト機能に対応しているので、ライトプロテクトの掛かったD77イメージには書き込みを行うことはできません(ライトプロテクトされていてもエラー表示をしませんが書き込みません)。また、D77イメージ内のディスクIDを確認していますのでF-BASICフォーマット以外のディスクに書き込むこともできません。

使用例：
```sh
C:\>FMWRITE test.d77 DPOKEm.2B0
```

●FMCOPY

機能： D77イメージファイル間でのファイル転送を行います

書式： `FMCOPY D77_IMG_IN D77_IMG_OUT F-BASIC_FILE [options]`

オプション： 

-V	Verboseモード。コピーしたバイト数などを表示する
詳細： ファイルコピーを行います。ワイルドカードなどは使用できません。送り先のファイル名、属性は送り元のものと同一となります（変更できません）。

使用例：
```sh
C:\>FMCOPY YASU.D77 KASHI.D77 DUMPA -V
10704 bytes copied
```

●D77DMP

機能： D77イメージフォーマット内のセクタダンプを表示します

書式１： `D77DMP D77_IMG track sctor [options]`
書式２： `D77DMP D77_IMG -Ccluster [options]`
書式３： `D77DMP D77_IMG -Llba [options]`

オプション： 

-M	マニュアルモード。セクタダンプ後にキー入力待ちになり、キー操作で前後のセクタに移動することができる
詳細： もともとFTOOLSを開発、デバッグするために作った補助ユーティリティです。以外に便利なのでFTOOLSに混ぜてしまいました。書式１のときはtrackは0～79、sectorは1～16で指定。書式２のときはクラスタ番号で指定すると、クラスタの先頭セクタをダンプ。書式３の時はTRK0:SCT1を0とした連番(LBA)でセクタ番号を指定する。LBA32はTRK2:SCT0と同一です。
また、マニュアルモード時のキー操作は以下のものを受け付けます。

|キー|意味||
|-|-|-|
|P|Previous sector|前のセクタを表示する|
|N|Next sector|次のセクタを表示する|
|Q, ESC|Quit|終了する|

使用例１：
```sh
C:\>D77DMP TEST.D77 2 1
Dumping TRK=2 SCT=1
-OFFSET- +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF
00000000 00 FF FF FF FF C3 C0 C5 C3 C0 C2 C0 C1 09 C4 0B .....ﾃﾀﾅﾃﾀﾂﾀﾁ.ﾄ.
00000010 0C 0D 0E 0F C1 11 C6 13 14 15 16 17 18 19 1A 1B ....ﾁ.ﾆ.........
00000020 1C C0 C0 C0 C0 C0 C0 C0 FF FF FF FF FF FF FF FF .ﾀﾀﾀﾀﾀﾀﾀ........
00000030 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
00000040 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
00000050 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
00000060 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
00000070 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
00000080 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
00000090 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
000000A0 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
000000B0 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
000000C0 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
000000D0 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
000000E0 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
000000F0 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF ................
```

使用例２：
```sh
C:\>D77DMP TEST.D77 -C1
～表示例は省略～
```

使用例３：
```sh
C:\>D77DMP TEST.D77 -L32
～表示例は省略～
```

●注意事項など

◆D77ファイルのリードオンリー属性について
FTOOLSはファイルをR/Wモードでオープンする関係上、ディスクイメージファイルはリードオンリーの属性がついているとアクセスできません。リードオンリーの場合、FMDIRのような読み出ししか行わないコマンドも使用できません。

◆D77イメージファイルのライトプロテクト機能
FTOOLSのコマンドはすべてD77イメージのライトプロテクト機能をサポートしています。ライトプロテクトフラグの立ててあるファイルに対して書き込みを行いませんが、エラーの表示も行いません(ひどい…)。つまり、FMWRITEやFMCOPYコマンドは何事も無かったように終了しますが、FDイメージファイルには何も書き込まれません。

◆IDチェック機能について
FTOOLSのコマンドはFM-FILE形式と、D77ファイル形式についてはファイルタイプをチェックしています。間違ったファイルを指定した場合エラーとして処理されますので誤ったファイルを指定しても中身を壊したりすることはありません（無いように設計しています）。

◆F-BASICファイルシステムDLL
FTOOLSの一部のコマンド(FMREAD,FMWRITE,FMCOPY,FMDIR)はFTOOLSに同梱のFMFSLIB.DLLを使用します。コマンド(*.EXE)と同じディレクトリに置いておいてください。EXPLORERではデフォルトでDLLファイルが表示されませんので、FTOOLSを移動するときにFMFSLIB.DLLをコピーし忘れないように注意してください。FMFSLIB.DLLはF-BASICファイルフォーマットのD77ディスクを取り扱うためのファイルシステムを提供しています。

◆ワイルドカード、正規表現
すべてのコマンドはワイルドカード、正規表現など一切受け付けません(しょぼ～)。一生懸命タイプして下さい。