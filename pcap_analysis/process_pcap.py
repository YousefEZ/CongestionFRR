import os
import sys
from scapy.all import *
import matplotlib.pyplot as plt


def read_pcap(filename):
    packets = []
    data = scapy.utils.PcapReader(filename)
    for packet in data:
        packets.append(packet)
        # print(packet.getlayer('TCP').options)
    return packets


def get_IP(packets):
    sender_IP = packets[0].getlayer('IP').src
    receiver_IP = packets[0].getlayer('IP').dst
    return sender_IP, receiver_IP


# def packet_transfer_time(sender_packets, receiver_packets):
#     timestamp = []
#     completion_time = []
#     stream_direction = []
#
#     for sender_packet in sender_packets:
#         for receiver_packet in receiver_packets:
#             if (sender_packet.getlayer('TCP').seq == receiver_packet.getlayer('TCP').seq) and (
#                     sender_packet.getlayer('TCP').ack == receiver_packet.getlayer('TCP').ack) and (
#                     sender_packet.getlayer('IP').src == receiver_packet.getlayer('IP').src) and (
#                     sender_packet.getlayer('IP').dst == receiver_packet.getlayer('IP').dst) and (
#                     sender_packet.getlayer('TCP').window == receiver_packet.getlayer('TCP').window) and (
#                     sender_packet.getlayer('TCP').options == receiver_packet.getlayer('TCP').options) and (
#                     sender_packet.getlayer('TCP').options[0][1] == receiver_packet.getlayer('TCP').options[0][1]):
#                 if sender_packet.time > receiver_packet.time:
#                     completion_time.append(sender_packet.time - receiver_packet.time)
#                     stream_direction.append('down')
#                 else:
#                     completion_time.append(receiver_packet.time - sender_packet.time)
#                     stream_direction.append('up')
#                 timestamp.append(sender_packet.time)
#                 break
#
#     return timestamp, completion_time, stream_direction


def flow_completion_time(packets):
    TCP_FIN = 0x01
    TCP_ACK = 0x10
    # the hex code for flags in TCP is as above

    # print(packets[0][TCP].flags)
    for pkt in packets:
        if TCP in pkt:
            if pkt[TCP].flags & TCP_FIN and pkt[TCP].flags & TCP_ACK:
                return pkt.time
    return None


def save_plot(timestamp, completion_time, stream_direction):
    os.makedirs("/host/plots", exist_ok=True)
    plt.plot(timestamp, completion_time, label=stream_direction, marker='o')
    plt.xlabel("time stamp of the sender of the packet")
    plt.ylabel("transfer completion time")

    plt.savefig("/host/plots/completion_time-timestamp.png")


if __name__ == '__main__':
    files = os.listdir("traces")
    print(os.listdir("traces"))

    results = []

    for file in files:
        if file.endswith(".pcap"):
            senderPackets = read_pcap("traces/" + file)
            senderIP, receiverIP = get_IP(senderPackets)

            # timestamp, completion_time, stream_direction = packet_transfer_time(sender_packets, receiver_packets)

            fc_time = flow_completion_time(senderPackets)
            result = file + ":" + str(fc_time) + "\n"
            print(result)
            results.append(result)
            # save_plot(timestamp, completion_time, stream_direction)

    os.makedirs("/host/results", exist_ok=True)

    with open("/host/results/results.txt", "w") as f:
        for result in results:
            f.write(result)