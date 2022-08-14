ASM = X6809

.src.mot:
	$(ASM) $*.src,$*.mot,$*.lst,$*.crf,$*.map

.mot.bin:
	mot2bin $*.mot $*.bin

ALL: fbasic30.rom boot_bas.rom boot_dos.rom subsys_c.rom \
	initiate.rom subsys_a.rom subsys_b.rom subsyscg.rom kanji.rom \
	kanji2.rom dicrom.rom extsub.rom


CLEAN:
	del *.mot
	del *.lst
	del *.crf
	del *.map
	del *.bin

ALLCLEAN:
	del *.rom
	del *.mot
	del *.lst
	del *.crf
	del *.map
	del *.bin


#
# Make F-BASIC rom file
#
fbasic30.rom : basic.bin
	bincut $? $@ 8000 fbff

#
# Make BASIC mode boot loader
#
boot_bas.mot : bootrom.src
	$(ASM) bootrom.src,$*.mot,$*.lst,$*.crf,$*.map /DIPLENT=256

boot_bas.rom : boot_bas.bin
	bincut boot_bas.bin boot_bas.rom fe00 ffff

#
# Make DOS mode boot loader
#
boot_dos.mot : bootrom.src
	$(ASM) bootrom.src,$*.mot,$*.lst,$*.crf,$*.map /DIPLENT=768

boot_dos.rom : boot_dos.bin
	bincut boot_dos.bin boot_dos.rom fe00 ffff

#
# Genarate KANJI.ROM file
#
kanji.rom:
	krom.exe


#
# Make Subsystem monitor
#
subsys_c.rom : submon.bin font1a
	copy font1a /b + submon.bin /b subsys_c.rom /b

submon.bin : subrom.bin
	bincut subrom.bin submon.bin e000 ffff


#
# Make initiator
#
initiate2.bin : initiate.bin
	bincut initiate.bin initiate2.bin 6000 6fff
	copy /Y initiate2.bin initiate.bin

initiate.rom : initiate2.bin boot_bas.rom boot_dos.rom
	seven2av


#
# Generate subsys_a.rom file
#
subsys_a.rom : subsys_c.rom
	seven2av


#
# Generate subsys_b.rom file
#
subsys_b.rom : subsys_c.rom
	seven2av


#
# Generate subsyscg.rom file
#
subsyscg.rom : subsys_c.rom
	seven2av


#
# Generate dummy kanji2.rom file
#
kanji2.rom :
	dmygen kanji2.rom 20000


#
# Generate dummy dicrom.rom file
#
dicrom.rom :
	dmygen dicrom.rom 40000


#
# Generate dummy extsub.rom file
#
extsub.rom :
	dmygen extsub.rom c000
