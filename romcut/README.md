# 当時ホームページに載せていた内容をマークダウン用に修正
---
●ROMCUT

== 説明 ==

このツールは、XM7で必要なROMファイル(FBASIC.ROM, SUBSYS_C.ROM, BOOT_BAS.ROM, BOOT_DOS.ROM)を、ROMライタから読み出したベタROMイメージファイル（FBASIC.BIN, SUBMON.BIN, BOOTROM.BIN）から作成するツールです。ROMイメージファイルの作成方法は、「KnowHow」の「ROMファイルの作成」を参照してください。

[romcut](bin/romcut_01.zip) v0.1 ダウンロード (15KB)

== 使用方法 ==

romcutと同じディレクトリにFBASIC.BIN, SUBMON.BIN, BOOTROM.BINの３つのファイルを用意してコマンド名をタイプするだけです。入力ファイル名は固定になっています。

書式：　`romcut`

実行例：
```sh
C:> romcut 
XM7 ROM CUTTER v0.1
Programmed by an XM7 supporter

This program requires the following files:
1. FBASIC.BIN
2. SUBMON.BIN
3. BOOTROM.BIN
You have to extract above files from your FM-7 ROM before you run this program.

$0000 to $7bff of 'FBASIC.BIN' is cut for 'FBASIC30.ROM'
$5800 to $7fff of 'SUBMON.BIN' is cut for 'SUBSYS_C.ROM'
$0000 to $01ff of 'BOOTROM.BIN' is cut for 'BOOT_BAS.ROM'
$0400 to $05ff of 'BOOTROM.BIN' is cut for 'BOOT_DOS.ROM'
```

== 注意事項 ==

特になし。

== 変更履歴 ==

v0.1	2000/03/19	メッセージの英語を修正。機能の変更はなし。
v0.0	?	初回リリース
