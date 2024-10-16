# Check non connected images

## Requirement
- Colmap 3.10 (その他のバージョンだとDBのスキーマが違う可能性が有るので動かない可能性が高い)

## 説明
[https://colmap.github.io/faq.html#reconstruct-sparse-dense-model-from-known-camera-poses](https://colmap.github.io/faq.html#reconstruct-sparse-dense-model-from-known-camera-poses) などで point_triangulator を使う場合、マッチングでどの画像ともペアでない(Two view geometoryを求められていない)と、out of rangeが発生する。  
そのような画像が無いかをチェックするスクリプトを書いた。
また、オプションで画像のディレクトリやsparse modelがそのような画像を取り除けるような機能がある。

ファイルは  
- main.py
- read_write_model.py

の二つ。

## TODO
- dbも修正できるようにする。
