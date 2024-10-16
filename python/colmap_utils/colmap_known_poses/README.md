# Correcter for Reconstruction sparse/dense model from known camera poses

## Requirement
- Colmap 3.10 (その他のバージョンだとDBのスキーマが違う可能性が有るので動かない可能性が高い)

## 説明
[](https://colmap.github.io/faq.html#reconstruct-sparse-dense-model-from-known-camera-poses)
にある通りに、内パラ、外パラが求まっているの再構成は可能。  
しかし、feature extract時のimage idと、姿勢入力に使うcolmap sparseのimage idが異なる場合がある。  
よって、それを修正するスクリプトを書いた。
ファイルは  
- main.py
- read_write_model.py

の二つ。

## TODO
- prior ポーズも挿入できるようにして、Spatial Matchができるようにする
