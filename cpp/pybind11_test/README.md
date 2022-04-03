# pybind11 Test

## Requirements

- Python3

- pybind11

```bash
pip3 install pybind11
```

- realpath

Please install GNU Core Utilities.

## Build

```bash
make
```

## Run

```bash
python3 main.py
```

## 型アノテーションStubファイル(.pyi)を作る

`pybind11-stubgen` を入れる

```bash
pip3 install pybind11-stubgen
```

stubファイルをtypingsディレクトリに作成
```bash
PYTHONPATH=. pybind11-stubgen hoge --no-setup-py --root-module-suffix="" --ignore-invalid=all --output-dir="./typings"
```

もしくは
```bash
make create_stub_files
```
