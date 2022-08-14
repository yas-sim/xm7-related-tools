set FILE=BOOTROM
x6809 %FILE%.src,%FILE%.s,%FILE%.lst,%FILE%.crf,%FILE%.map
rem fmencode %FILE%.s
rem fmwrite nosys_blank.d77 %FILE%.2b0
MOT2BIN %FILE%.S %FILE%.BIN
BINCUT %FILE%.BIN BOOT_BAS.ROM FE00 FFFF
