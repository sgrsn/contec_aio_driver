# contec_aio_driver

CONTEC アナログ入出力ボードのROSパッケージです．  
CONTEC caioライブラリのラッパーパッケージです．はじめにCONTECが配布するcaioライブラリをインストールする必要があります．

# param list

## device_name
デバイス名(/usr/local/etc/contec_aio.conf におけるDeviceName)

## ai_range
アナログ入力範囲の設定(全チャンネル)  
以下の表の対応する値を設定  

| レンジ   | マクロ   | 値  |
|----------|----------|-----|
| ±10V     | PM10     | 0   |
| ±5V      | PM5      | 1   |
| ±2.5V    | PM25     | 2   |
| ±1.25V   | PM125    | 3   |
| 0～10V   | P10      | 50  |
| 0～5V    | P5       | 51  |
| 0～2.5V  | P25      | 53  |
| 0～1.25V | P125     | 54  |
| 0～20mA  | P20MA    | 100 |
| 4～20mA  | P4TO20MA | 101 |
| ±20mA    | PM20MA   | 102 |
| 1～5V    | P1TO5    | 150 |


## ai_channels
使用するチャンネル数