# 当時ホームページに載せていた内容をマークダウン用に修正
---

★D77形式ディスクイメージ操作ツール

　

このツールを使用した結果、D77イメージが破壊されてしまう可能性があります(バグがあるかもしれないため)。D77UTYで操作する前に必ずバックアップを取ってから作業するようにしてください。

また、D77UTYを使用した結果生じるいかなる損害に対しても補償は致しかねますので予めご了承ください。

上記制限、免責事項を了承いただけない場合、D77UTYの使用はできません。



== 説明 ==

XM7で標準サポートされているディスクイメージファイル(.D77形式ファイル)を操作するためのツールです。もちろん、".D88"ファイルも操作できますので、PC88エミュ関係者もどうぞ(?)

　[d77uty_01.zip](bin/d77uty_01a.zip) v0.1aダウンロード (20KB)


== 使い方 ==

今回はオプション位置固定の仕様になっています。サブコマンド(２つめのパラメータ)ごとにフォーマットがありますので注意してください。

書式：　

|番号|書式|説明|
|--|--|--|
|1|`d77uty入力ファイル ADD 追加ファイル イメージ番号`<br>`d77uty 入力ファイル +   追加ファイル イメージ番号`|入力ファイルに、追加ファイルを追加する。追加されるファイルは、先頭から「イメージ番号」番の位置に追加される。**入力ファイルの最後に追加する場合は2の書式を使用する。**|
|2|`d77uty 入力ファイル ADD 追加ファイル`<br>`d77uty 入力ファイル + 追加ファイル`|入力ファイルの最後に追加ファイルを追加する(イメージの追加)。追加ファイルはD77形式のイメージファイルであることが必要。|
3|`d77uty 入力ファイル L`<br>`d77uty 入力ファイル       (サブコマンド省略時)`|入力ファイルの中のイメージ名および、ライトプロテクト状態の一覧を表示する。|
|4|`d77uty 入力ファイル EXT 取り出しファイル イメージ番号`<br>`d77uty 入力ファイル -   取り出しファイル イメージ番号`|入力ファイルの「イメージ番号」番目のイメージをファイルとして抜き取る。入力ファイルからは抜き出されたイメージは削除される。|
|5|`d77uty 入力ファイル CPY 取り出しファイル イメージ番号`|入力ファイルの「イメージ番号」番目のイメージをファイルとして抜き取る。入力ファイルからは抜き出されたイメージは削除されない。|
|6|`d77uty 入力ファイル P+  イメージ番号`<br>`d77uty 入力ファイル P-  イメージ番号`<br>`d77uty 入力ファイル P[+\|-] ALL`|入力ファイルの「イメージ番号」番のイメージのライトプロテクトフラグを解除または設定する。イメージ番号に"ALL"を指定すると、入力ファイルに含まれる全てのイメージに対して設定を行う。<br>P+：ライトプロテクトあり(書込み禁止)<br>P-：ライトプロテクトなし(書き込み許可)|
|7|`d77uty 入力ファイル NAM イメージ名 イメージ番号`|入力ファイルの「イメージ番号」番のイメージのイメージ名を設定する。イメージ名はANK16文字まで。16文字以上が指定されたときは先頭の16文字のみを使用する。|
|8|`d77uty 入力ファイル ORD オーダー`|入力ファイル内のイメージを指定の順番に並べ替える。順番(オーダー)は、数字一文字で指定する。したがって、9個以上のイメージを含むファイルは扱えない。同じ番号を複数指定することも可能(実行例参照)。<br>e.g. d77uty test.d77 ord 4123  →イメージを、元々の番号で言う４，１，２，３の順位並び替える|

当然ですが、入力ファイルおよび、追加ファイルは正常なD77形式ファイルである必要があります。

以下に、コマンド例とその実行結果の関係を図で分かりやすく説明してありますのでご参照ください。

★ADDコマンド(1)  

`d77uty original.d77 add addfile.d77 2`

|||||
|-|-|-|-|
|original.d77|addfile.d77||original.d77|
|IMG1<br>IMG2<br>IMG3|ADD_IMG|→|IMG1<br>ADD_IMG<br>IMG2<br>IMG3|

★ADDコマンド(2)  

`d77uty original.d77 add addfile.d77`

|||||
|-|-|-|-|
|original.d77|addfile.d77||original.d77|
|IMG1<br>IMG2<br>IMG3|ADD_IMG|→|IMG1<br>IMG2<br>IMG3<br>ADD_IMG|

★EXTコマンド  

`d77uty original.d77 ext extfile.d77 2`

|||||
|-|-|-|-|
|original.d77||original.d77|extfile.d77|
|IMG1<br>IMG2<br>IMG3|→|IMG1<br>IMG3|IMG2|

★CPYコマンド  

`d77uty original.d77 cpy extfile.d77 2`

|||||
|-|-|-|-|
|original.d77||original.d77|extfile.d77|
|IMG1<br>IMG2<br>IMG3|→|IMG1<br>IMG2<br>IMG3|IMG2|

★ORDコマンド

`d77uty original.d77 ord 231`

||||
|-|-|-|
|original.d77||original.d77|
|IMG1<br>IMG2<br>IMG3|→|IMG2<br>IMG3<br>IMG1|  


`d77uty original.d77 ord 3313` (IMG2はなくなってしまう。IMG3は３つに増える)

| | | |
|-|-|-|
|original.d77||original.d77|
|IMG1<br>IMG2<br>IMG3|→|IMG3<br>IMG3<br>IMG1<br>IMG3|  
　


== 実行例 ==　
```sh
C:/>d77uty test.d77
D77 image manipulation program 0.1
Programmed by an XM7 supporter
1 : 'Program '      WP=ON
2 : 'Data1        ' WP=ON
3 : 'Data2        ' WP=ON
4 : 'User '         WP=ON　
C:/>d77uty test.d77 p- 4                   (←4番目のイメージのライトプロテクトを解除)
D77 image manipulation program 0.1
Programmed by an XM7 supporter
C:/>d77uty test.d77
D77 image manipulation program 0.1
Programmed by an XM7 supporter
1 : 'Program '      WP=ON
2 : 'Data1        ' WP=ON
3 : 'Data2        ' WP=ON
4 : 'User '         WP=OFF
C:/>d77uty test.d77 + ProgramData3.d77 3   (←3番目にくるように、"ProgramData3.d77"を追加する)
D77 image manipulation program 0.1
Programmed by an XM7 supporter
C:/>d77uty test.d77
D77 image manipulation program 0.1
Programmed by an XM7 supporter
1 : 'Program '      WP=ON
2 : 'Data1        ' WP=ON
3 : 'Data3        ' WP=ON
4 : 'Data2        ' WP=ON
5 : 'User '         WP=OFF
C:/>d77uty test.d77 ord 12435              (←現在の番号で1,2,4,3,5の順になるように並べ替え)
D77 image manipulation program 0.1
Programmed by an XM7 supporter
C:/>d77uty test.d77
D77 image manipulation program 0.1
Programmed by an XM7 supporter
1 : 'Program '      WP=ON
2 : 'Data1        ' WP=ON
3 : 'Data2        ' WP=ON
4 : 'Data3        ' WP=ON
5 : 'User '         WP=OFF
```

== その他ノウハウなど　==

・くれぐれもd77utyでいじる前にイメージのバックアップを取るようにお願いいたします。自分で何度も使用してから公開していますが、作者は構造を知っているので意図的にまずそうなパラメータの与え方をさけているはずです(^^; 一般に公開してから発覚するバグが潜んでいる可能性があるので大切なイメージを控えもなしにいじくりまわすことはさけてください。

・D88形式のツールでも同じようなものが存在するようですが(しかもそちらはGUI版らしいし)、リンクが切れていてダウンロードできないので作ってしまいました。GUI版が欲しい方はD88のツールをお探しください。きっとすばらしいツールがあるはずです(^^;

・ADDコマンドで追加する時の追加ファイルは必ずD77形式のファイルを指定してください。D77/D88形式は正しいファイルかどうか判別するためのマジックワードを持たないため、無条件に指定されたファイルを挿入する仕様になっています。

== 改版履歴 ==

v0.1    2001/01/01    初回公開バージョン
v0.1a   2001/01/01    パラメータ省略時のコマンドパラメータ説明を追加