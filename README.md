参考 [https://101010.fun/iot/m5stickc-plus-view-images.html](https://101010.fun/iot/m5stickc-plus-view-images.html)  
↑の中で紹介されていた、画像をヘッダファイルにするやつ  [https://lang-ship.com/tools/image2data/](https://lang-ship.com/tools/image2data/)

表示したい画像を135*240で作成する
[.hにするやつ](https://lang-ship.com/tools/image2data/)で変換してもらう
データ名と同じでもいいし違うんでもいいけど適当にヘッダファイルを作って↑の結果をコピペ
(platformIOの場合)`main.cpp`と同じディレクトリに配置する
`#include "hoge.h"`して表示したい画像の配列に追加する
多分動く
