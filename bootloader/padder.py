#python3 works for this. not python

BOOTLOADER_SIZE = 0x8000
BOOTLOADER_FILE = "bootloader.bin"

with open(BOOTLOADER_FILE, "rb") as f:
    raw_file = f.read()

#print(len(raw_file))
bytes_to_pad = BOOTLOADER_SIZE - len(raw_file)
#print(bytes_to_pad)
padding = bytes([0xff for _ in range(bytes_to_pad)])

with open(BOOTLOADER_FILE, "wb") as f:
    f.write(raw_file + padding)