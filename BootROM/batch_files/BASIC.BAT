set FILE=BASIC
x6809 %FILE%.src,%FILE%.s,%FILE%.lst,%FILE%.crf,%FILE%.map
MOT2BIN %FILE%.S %FILE%.BIN
BINCUT %FILE%.BIN FBASIC30.ROM 8000 FFFF
SEVEN2AV
