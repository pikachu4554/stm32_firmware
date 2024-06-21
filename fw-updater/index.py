import serial
import time
import threading

# Constants for the packet protocol
PACKET_LENGTH_BYTES = 1
PACKET_DATA_BYTES = 16
PACKET_CRC_BYTES = 1
PACKET_CRC_INDEX = PACKET_LENGTH_BYTES + PACKET_DATA_BYTES
PACKET_LENGTH = PACKET_LENGTH_BYTES + PACKET_DATA_BYTES + PACKET_CRC_BYTES

PACKET_ACK_DATA0 = 0x15
PACKET_RETX_DATA0 = 0x19

# Details about the serial port connection
serial_path = "/dev/ttyACM0"
baud_rate = 115200

# CRC8 implementation
def crc8(data):
    crc = 0

    for byte in data:
        crc = (crc ^ byte) & 0xff
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xff
            else:
                crc = (crc << 1) & 0xff

    return crc

# Async delay function, which gives the event loop time to process outside input
def delay(ms):
    time.sleep(ms / 1000.0)

# Class for serialising and deserialising packets
class Packet:
    def __init__(self, length, data, crc=None):
        self.length = length
        self.data = data

        bytes_to_pad = PACKET_DATA_BYTES - len(self.data)
        padding = bytes([0xff] * bytes_to_pad)
        self.data += padding

        if crc is None:
            self.crc = self.compute_crc()
        else:
            self.crc = crc

    def compute_crc(self):
        all_data = [self.length] + list(self.data)
        return crc8(all_data)

    def to_buffer(self):
        return bytes([self.length]) + self.data + bytes([self.crc])

    def is_single_byte_packet(self, byte):
        if self.length != 1:
            return False
        if self.data[0] != byte:
            return False
        for i in range(1, PACKET_DATA_BYTES):
            if self.data[i] != 0xff:
                return False
        return True

    def is_ack(self):
        return self.is_single_byte_packet(PACKET_ACK_DATA0)

    def is_retx(self):
        return self.is_single_byte_packet(PACKET_RETX_DATA0)

# Serial port instance
uart = serial.Serial(serial_path, baud_rate, timeout=1)

# Packet buffer
packets = []

last_packet = Packet(1, bytes([PACKET_ACK_DATA0])).to_buffer()

def write_packet(packet):
    global last_packet
    uart.write(packet)
    last_packet = packet

# Serial data buffer, with a splice-like function for consuming data
rx_buffer = bytearray()

def consume_from_buffer(n):
    global rx_buffer
    consumed = rx_buffer[:n]
    rx_buffer = rx_buffer[n:]
    return consumed

# This function fires whenever data is received over the serial port. The whole
# packet state machine runs here.
def handle_data():
    global rx_buffer
    while True:
        data = uart.read(uart.in_waiting or 1)
        if data:
            print(f"Received {len(data)} bytes through uart")
            # Add the data to the packet
            rx_buffer += data

            # Can we build a packet?
            if len(rx_buffer) >= PACKET_LENGTH:
                print("Building a packet")
                raw = consume_from_buffer(PACKET_LENGTH)
                packet = Packet(raw[0], raw[1:1+PACKET_DATA_BYTES], raw[PACKET_CRC_INDEX])
                computed_crc = packet.compute_crc()

                # Need retransmission?
                if packet.crc != computed_crc:
                    print(f"CRC failed, computed 0x{computed_crc:02x}, got 0x{packet.crc:02x}")
                    write_packet(Packet(1, bytes([PACKET_RETX_DATA0])).to_buffer())
                    continue

                # Are we being asked to retransmit?
                if packet.is_retx():
                    print("Retransmitting last packet")
                    write_packet(last_packet)
                    continue

                # If this is an ack, move on
                if packet.is_ack():
                    print("It was an ack, nothing to do")
                    continue

                # Otherwise write the packet in to the buffer, and send an ack
                print("Storing packet and ack'ing")
                packets.append(packet)
                write_packet(Packet(1, bytes([PACKET_ACK_DATA0])).to_buffer())

# Start the data handling thread
threading.Thread(target=handle_data, daemon=True).start()

# Function to allow us to await a packet
def wait_for_packet():
    while len(packets) < 1:
        delay(1)
    packet = packets.pop(0)
    return packet

# Do everything in a main function so we can have loops, awaits etc
def main():
    print("Waiting for packet...")
    packet = wait_for_packet()
    print(packet.__dict__)

    packet_to_send = Packet(4, bytes([5, 6, 7, 8]))
    packet_to_send.crc += 1  # Intentionally corrupting the CRC for demonstration
    uart.write(packet_to_send.to_buffer())

if __name__ == "__main__":
    main()
