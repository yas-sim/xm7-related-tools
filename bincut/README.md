# 当時ホームページに載せていた内容をマークダウン用に修正
---
●バイナリファイルの必要な部分だけを切り出すツール BINCUT

==　説明 ==

ベタバイナリ(でなくても構いませんが)ファイルの任意の範囲を切り出すプログラムです。ROMファイル作成時など、不要な部分を除き、必要な部分だけを取り出したりするときに便利です。

[bincut v0.1ダウンロード](bin/bincut_01.zip) (17KB)

== 使用方法 ==

書式： `bincut infile outfile start_adr end_adr`

|||
|-|-|
|infile|入力ファイルです|
|outfile|切り出した結果を格納するファイル名です|
|start_adr|切り出しを開始する位置です。ファイルの先頭の1バイトは0になります。16進で指定します|
|end_adr|切り出しを終了する位置です。16進で指定します|

保存されるサイズは、end_adr-start_adr+1になります。

infileには何も操作を行いません。

　

例： rawdata.binファイルの0x8000バイト目から0ｘfcffバイト目をromdata.romファイルに書き出します。
```sh
C:\> bincut rawdata.bin romdata.rom 8000 fcff
```

== 注意事項など ==

　

== 改版履歴 ==

v0.1  00/01/23  初回公開

　