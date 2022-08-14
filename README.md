# XM7関連ツール置き場(集積場)  

これらのツールは主に2000年ごろに書いた、XM7用のツール類です。  
正直、ソースを公開するつもりで書いていませんでしたので、だいぶ適当で汚いコードですが、なくしてしまう前に公開しておきます。  
どちらかというと私のバックアップ的な目的でアップロードしており、いうなれば公開はついでです。  

コードの中身などについてもさっぱり思い出せないので、何か聞かれてもお答えできないと思っておいてください。  

一応、CMakeLists.txtを書いたので、ビルドはできるはずです。  
一部Win32 APIを使ってるツールはWindowsでしかビルドできません。  

もう少しコード眺めていたら、Win32 APIでファイル開いていたり、tcharでマルチバイト文字扱ったりしてる部分があって、Windows以外ではビルドはだいぶ難しそうです。  

```sh
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

- ツール説明（だいぶうろ覚え）
|ツール|説明|
|--|--|
|BootROM|FM-7実機がなくてもある程度ゲームができるようにする必要最低限のブートROMコード。一部のゲームしか動かない。|
|bin2mot|バイナリファイルをMotorolaS形式に変換|
|bincut|バイナリファイルの一部を切り出す|
|d77end/d77dec|D77ディスクイメージをテキスト形式に変換したり、そのファイルを元のディスクイメージに戻したりするツール。テキストエディタでD77の中身をいじれる。|
|d77uty|D77イメージのディスク順番とかを操作するツール|
|dmygen|たしか、ダミーファイルを作成するツール|
|fdump|ファイルダンプ|
|fmtools|F-BASICフォーマットのD77イメージを直接操作するツール。ディレクトリをみたりファイルを抜き出したり|
|fontp|XM7用のフォントファイルを作成するツール|
|krom|漢字ROM相当のデータを作るツール。たぶん動かない|
|mot2bin|MotorolaSファイルをバイナリに変換|
|nosys_ipl| ?? |
|romcut|ROMライタで読みだしたFM-7のROMバイナリファイルを切り出してXM7起動に必要なファイルに切り分けるツール|
|seven2av|XM7 v1用のROMファイルに小細工をしてXM7 v2を動かすのに必要なROMファイルを捏造するツール|
|subtfr| ?? |
|t772wav|T77カセットイメージファイルをWav音声ファイルに変換|
|t77dec|T77カセットイメージファイルの中身を確認したりファイルを取り出したり|
|wav2t77|Wav音声ファイルをT77カセットイメージファイルに変換|

