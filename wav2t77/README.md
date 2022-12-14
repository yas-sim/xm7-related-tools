# 当時ホームページに載せていた内容をマークダウン用に修正
---
★WAV→T77ファイルコンバーター


== 説明 ==

　XM7でサポートしているカセットイメージファイル(T77ファイル)はエミュレータ上で作成することは容易ですが、カセットに記録(録音)されているデータをT77ファイルに変換する方法が無いため古いカセットのゲームなどを持っている方は使えなくて困っていたと思います。
このプログラムは、Windowsマシンで録音したカセット音声(WAVファイル)からT77形式のファイルに変換するプログラムです
録音レベルなどは(一応)自動で検出して、適切なしきい値を使ってコンバートします。

　[w2t7_03.zip](bin/w2t7_03.zip) v0.3ダウンロード (26KB)


== 使い方 ==

またまたコマンドラインプログラムです。まぁ、FM-7を使っていた方ならこのくらいど～ってことないですよね(^^;  引数として入力ファイル名と出力ファイル名を指定します

書式：　`wav2t77 入力ファイル 出力ファイル [オプション]`

入力ファイルは拡張子がwavになっている(wavでなくてもかまいませんが)音声データを指定します。この音声データは必ずPCM形式で作成されている必要があります。μ-LawやA-Lawをはじめとする圧縮形式wavではだめです。サンプリングツールの保存時に保存形式が指定できない場合はたいていPCM形式で保存されていると思います。

出力ファイルは、変換されたT77形式データのファイル名を指定します。

オプションは次のものがあります。

|||
|-|-|
|-v|Verboseモード (音声ファイルのフォーマット情報など、いろいろメッセージを出力する)|
|-p[mode]|位相を指定する。昔のデレコのPhaseとか位相とかかかれてたスイッチと同じです。[mode]は以下の２つを指定できます<br>r = 逆相指定<br>n = 正相指定(デフォルト)|
|-th[hex]|コンパレータの上側のしきい値をhexで指定する|
|-tl[hex]|コンパレータの下側のしきい値をhexで指定する|

自動解析した値でコンバートした結果"Device I/O Error"などのエラーが出てロードできないときはこららのオプションを指定してみてください。まずは-prオプションを試してみるのがよいでしょう。

しきい値は-vオプションをつけて変換すると自動解析した値が表示されるので、その値を参考に変更するのがよいでしょう。wavファイルは0x8000を中心として+,-に振れますので、センターは0x8000として指定してください。-th8200 -tl7e00くらいが無難な設定のようです。-th, -tlオプションは両方を同時に指定しないと意味がありません。

用例：
```sh
C:\> wav2t77 sample.wav sample.t77
```
sample.wavという音声ファイルからsample.t77というファイルに変換する。

```sh
C:\> wav2t77 sample.wav sample.t77 -v
```
変換するときの情報(検出した録音レベルや、音声ファイルのフォーマット)を表示する

```sh
C:\> wav2t77 sample.wav sample.t77 -th8200 -tl7e00
```
録音レベルの自動検出を行わず、指定されたしきい値を使ってwavからt77ファイルに変換する

```sh
C:\> wav2t77 sample.wav sample.t77 -th8200 -tl7e00 -pr
```
録音レベルの自動検出を行わず、指定されたしきい値を使ってwavからt77ファイルに変換する(逆相で変換)


WAVファイルは以下のものに対応している予定です(^^;　

|サンプルレート|22K, 44K, 48Kなど何でもOK!|
|-|-|
|量子化ビット数|8 or 16|
|チャネル数|1 or 2|

== 実行例 ==　
```sh
C:/>wav2t77 m.wav m.t77 -pr -v
WAV to T77 converter v0.3
Programmed by an XM7 supporter

PASS1: Analyzing record level...
Record Level High 0x824f Low 0x7dfe
Sample freq 44100[Hz] Channel 1[ch] Quantize bit 8[bit] Phase Reverse
PASS2: Converting...
Done.
```

== その他ノウハウなど　==

ロード途中でエラーが出る場合はまず最初に-prオプションを試してみてください。これだけであっさりロードできてしまうときがあります。

サンプルレートは高ければ高いほど変換精度が向上します。録音レベルはファイルをスキャンして自動で設定しますので飽和しない程度に録音されていれば変換できると思います。22Kサンプルのデータは読めるように変換できましたが、11Kサンプルのデータではサンプル数が少なすぎるためかうまく変換できませんでした。ステレオ/モノラルや、8ビット/16ビットなどはほとんど変換精度に影響しないため、44K/Mono/8bitでサンプルするのがベストだと思います。

参考として、変換テストに使用したwavファイルと、このプログラムを使って生成したT77ファイルを含むサンプルデータも載せておきます。3行ばかりのBASICプログラムをBINモードでセーブしてあります。

[basic_i.zip](bin/basic_i.zip) ダウンロード (38KB)   変換テストに使用したBASICテキストのサンプルデータ

プログラムは、音声データをLPFに通し、それをヒステリシスコンパレータでコンパレートした結果として変換データを作成しています。

録音レベルの検出は、音声データの一定区間の標準偏差をとり、偏差が一定値以下に収まった場合をHとLのレベルとみなし、判断しています。

　

== 改版履歴 ==

// v0.0 2000/02/29 初期公開バージョン
// v0.1 2000/02/29 -th, -tlオプション追加
// v0.2 2000/03/01 録音レベルの検出方法に標準偏差を使うように変更
// v0.3 2000/03/03 LPFの方式を積分方式に変更(非公開)
// v0.3 2000/05/21 位相を指定できるようにした