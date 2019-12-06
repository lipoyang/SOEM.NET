# SOEMの.NETラッパー
<!--
## 解説記事

- [SOEMを.NETで使う - 滴了庵日録](TODO)
-->
## DLL(soemlib.dll)のビルド

- Npcapライブラリをインストールする。([こちらの記事](https://lipoyang.hatenablog.com/entry/2019/04/19/204636)を参照)
- [SOEM](https://github.com/OpenEtherCATsociety/SOEM)のソースをクローンないしダウンロードする。
- SOEM/CMakeLists.txt の最後のほうに1行追加する。

```CMake
if(BUILD_TESTS) 
  add_subdirectory(test/linux/slaveinfo)
  add_subdirectory(test/linux/eepromtool)
  add_subdirectory(test/linux/simple_test)
  add_subdirectory(test/linux/soemlib)     # ←この行を追加
endif()
```
- このリポジトリのsoemlibフォルダをSOEM/test/linux/soemlibにコピーする。
- CMakeでビルドすると SOEM/build/test/linux/soemlib/soemlib.dll ができる。

## .NETラッパーとサンプルアプリ

- [EtherCAT/SOEM/EtherCAT.cs](EtherCAT/SOEM/EtherCAT.cs) が.NETラッパー。
- [EtherCAT/EasyTest](EtherCAT/EasyTest) がサンプルアプリ。
- 上記のDLL(soemlib.dll) を EtherCAT/EasyTest/soemlib.dll にコピーする。
- Visual Studioでビルドする。

## サンプルアプリ用のスレーブ

- ハードウェア: Arduino Uno + EasyCAT Shield
- 依存ライブラリ: [EasyCAT Library V2.0](https://www.bausano.net/en/hardware/ethercat-e-arduino/easycat.html)
- スケッチ: [ec_slave/ec_slave.ino](ec_slave/ec_slave.ino)
- 入力デバイス: ArduinoのピンA0にボリューム(可変抵抗)で分圧した電圧を入力する。
- 出力デバイス: Arduinoのピン3にラジコンサーボを接続する。
