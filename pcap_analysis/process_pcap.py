import base64
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


def plot_flow_completion_time(results, group):
    sorted_results = sorted(results, key=lambda x: x[1])
    figure, axes = plt.subplots()

    for result in sorted_results:
        if result is not None:
            axes.plot(result[1], result[2], label=result[0], marker='o')

    axes.set_xlabel("number of packets in the queue")
    axes.set_ylabel("flow completion time in seconds")

    axes.set_title(f"flow completion for {group}")

    figure.savefig(f"/host/plots/{group}.png")


def record_flow_completion_time(source_directory, result_directory):
    groups = os.listdir(source_directory)
    for group in groups:
        if os.path.isdir(os.path.join(source_directory, group)):
            results = []
            queue_sizes = os.listdir(source_directory + group)
            for queue_size in queue_sizes:
                if os.path.isdir(os.path.join(source_directory + group + "/" + queue_size)):
                    senderPackets = read_pcap(
                        source_directory + group + "/" + queue_size + "/" + "baseline-no-udp/-TrafficSender-1.pcap")
                    fc_time = flow_completion_time(senderPackets)
                    results.append(("baseline_no_udp", queue_size, fc_time))

                    senderPackets = read_pcap(
                        source_directory + group + "/" + queue_size + "/" + "baseline-udp/-TrafficSender-1.pcap")
                    fc_time = flow_completion_time(senderPackets)
                    results.append(("baseline_udp", queue_size, fc_time))

                    senderPackets = read_pcap(
                        source_directory + group + "/" + queue_size + "/" + "frr-no-udp/-TrafficSender-1.pcap")
                    fc_time = flow_completion_time(senderPackets)
                    results.append(("frr_no_udp", queue_size, fc_time))

                    senderPackets = read_pcap(
                        source_directory + group + "/" + queue_size + "/" + "frr/-TrafficSender-1.pcap")
                    fc_time = flow_completion_time(senderPackets)
                    results.append(("frr", queue_size, fc_time))

            filepath = os.path.join(result_directory, f"{group}.txt")
            with open(filepath, 'w') as f:
                for result in results:
                    f.write(str(result) + "\n")

            plot_flow_completion_time(results, group)


if __name__ == '__main__':
    os.makedirs("/host/results", exist_ok=True)
    os.makedirs("/host/plots", exist_ok=True)

    record_flow_completion_time("experiments/", "/host/results")

    # results = []
    #
    # for file in files:
    #     if file.endswith(".pcap"):
    #         senderPackets = read_pcap("traces/" + file)
    #         senderIP, receiverIP = get_IP(senderPackets)
    #
    #         # timestamp, completion_time, stream_direction = packet_transfer_time(sender_packets, receiver_packets)
    #
    #         fc_time = flow_completion_time(senderPackets)
    #         result = file + ":" + str(fc_time) + "\n"
    #         print(result)
    #         results.append(result)
    #         # save_plot(timestamp, completion_time, stream_direction)
    #
    # os.makedirs("/host/results", exist_ok=True)
    #
    # with open("/host/results/results.txt", "w") as f:
    #     for result in results:
    #         f.write(result)
