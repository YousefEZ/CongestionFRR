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


# def save_plot(timestamp, completion_time, stream_direction):
#     os.makedirs("/host/plots", exist_ok=True)
#     plt.plot(timestamp, completion_time, label=stream_direction, marker='o')
#     plt.xlabel("time stamp of the sender of the packet")
#     plt.ylabel("transfer completion time")
#
#     plt.savefig("/host/plots/completion_time-timestamp.png")


def record_flow_completion_time(source_directory, result_directory, mode):
    variables = os.listdir(source_directory)
    results = []
    for variable in variables:
        if os.path.isdir(os.path.join(source_directory, variable)):
            queue_sizes = os.listdir(source_directory + variable)
            for queue_size in queue_sizes:
                if os.path.isdir(os.path.join(source_directory + variable + "/" + queue_size)):
                    senderPackets = read_pcap(
                        source_directory + variable + "/" + queue_size + "/" + "baseline-no-udp/-TrafficSender-1.pcap")
                    fc_time = flow_completion_time(senderPackets)
                    results.append(("baseline_no_udp", queue_size, fc_time, variable))

                    senderPackets = read_pcap(
                        source_directory + variable + "/" + queue_size + "/" + "baseline-udp/-TrafficSender-1.pcap")
                    fc_time = flow_completion_time(senderPackets)
                    results.append(("baseline_udp", queue_size, fc_time, variable))

                    senderPackets = read_pcap(
                        source_directory + variable + "/" + queue_size + "/" + "frr-no-udp/-TrafficSender-1.pcap")
                    fc_time = flow_completion_time(senderPackets)
                    results.append(("frr_no_udp", queue_size, fc_time, variable))

                    senderPackets = read_pcap(
                        source_directory + variable + "/" + queue_size + "/" + "frr/-TrafficSender-1.pcap")
                    fc_time = flow_completion_time(senderPackets)
                    results.append(("frr", queue_size, fc_time, variable))

        filepath = os.path.join(result_directory, f"{mode}.txt")
        with open(filepath, 'w') as f:
            for result in results:
                f.write(str(result) + "\n")

    return results


def plot_flow_completion_time(results, mode, cases):
    sorted_results = sorted(results,
                            key=lambda x: int(x[3].split('Kbps')[0]) if 'Kbps' in x[3] else int(x[3].split('ms')[0]))
    figure, axes = plt.subplots(figsize=(10, 6))

    marker_styles = {'baseline': {'marker': 'o', 'colour': 'black'},
                     '20': {'marker': 's', 'colour': 'blue'},
                     '40': {'marker': '^', 'colour': 'green'},
                     '60': {'marker': 'v', 'colour': 'purple'},
                     '80': {'marker': 'x', 'colour': 'brown'},
                     '99': {'marker': 'd', 'colour': 'red'}
                     }

    legend_handles = {}
    legend_labels = {}

    points_dict = {}

    for result in sorted_results:
        if result is not None and result[2] is not None:
            if cases[0] == result[0]:
                marker_style = marker_styles.get('baseline')
                axes.plot(result[3], result[2], label=result[1], marker=marker_style['marker'],
                          color=marker_style['colour'], linestyle='-', markersize=5)

                if 'baseline' not in legend_handles:
                    legend_handles['baseline'], = axes.plot([], [], marker=marker_style['marker'],
                                                            color=marker_style['colour'], linestyle='-')
                    legend_labels['baseline'] = 'baseline'

                key = (result[0], result[1])
                if key not in points_dict:
                    points_dict[key] = []
                points_dict[key].append(result)

            if cases[1] == result[0]:
                marker_style = marker_styles.get(result[1])
                axes.plot(result[3], result[2], label=result[1], marker=marker_style['marker'],
                          color=marker_style['colour'], linestyle='-')

                if result[1] not in legend_handles:
                    legend_handles[result[1]], = axes.plot([], [], marker=marker_style['marker'],
                                                           color=marker_style['colour'], linestyle='-')
                    legend_labels[result[1]] = result[1]

                key = (result[0], result[1])
                if key not in points_dict:
                    points_dict[key] = []
                points_dict[key].append(result)

    for key, points in points_dict.items():
        for i in range(1, len(points)):
            axes.plot([points[i - 1][3], points[i][3]], [points[i - 1][2], points[i][2]],
                      color=marker_styles[points[i][1]]['colour'], linestyle='-')

    axes.set_xlabel(mode)
    axes.set_ylabel("flow completion time in seconds")

    axes.set_title(f"flow completion for {mode}, {cases[0]} and {cases[1]}")

    handles = [legend_handles[key] for key in sorted(legend_handles.keys())]
    labels = [legend_labels[key] for key in sorted(legend_handles.keys())]
    figure.legend(handles, labels, loc='upper left', fontsize='large', title='Legend')

    figure.subplots_adjust(left=0.2)
    # figure.tight_layout()

    figure.savefig(f"/host/plots/{mode}-{cases[0]}-{cases[1]}.png", dpi=300)


if __name__ == '__main__':
    os.makedirs("/host/results", exist_ok=True)
    os.makedirs("/host/plots", exist_ok=True)

    bandwidth_primary_results = record_flow_completion_time("experiments/bandwidth-primary/",
                                                            "/host/results", "bandwidth_primary")
    plot_flow_completion_time(bandwidth_primary_results, "bandwidth_primary", ['baseline_no_udp', 'frr_no_udp'])
    # no udp
    plot_flow_completion_time(bandwidth_primary_results, "bandwidth_primary", ['baseline_udp', 'frr'])
    # with udp

    bandwidth_alternate_results = record_flow_completion_time("experiments/bandwidth-alternate/",
                                                              "/host/results", "bandwidth_alternate")
    plot_flow_completion_time(bandwidth_alternate_results, "bandwidth_alternate", ['baseline_no_udp', 'frr_no_udp'])
    plot_flow_completion_time(bandwidth_alternate_results, "bandwidth_alternate", ['baseline_udp', 'frr'])

    delay_all_results = record_flow_completion_time("experiments/delay-all/",
                                                    "/host/results", "delay_all")
    plot_flow_completion_time(delay_all_results, "delay_all", ['baseline_no_udp', 'frr_no_udp'])
    plot_flow_completion_time(delay_all_results, "delay_all", ['baseline_udp', 'frr'])

    delay_primary_results = record_flow_completion_time("experiments/delay-primary/",
                                                        "/host/results", "delay_primary")
    plot_flow_completion_time(delay_primary_results, "delay_primary", ['baseline_no_udp', 'frr_no_udp'])
    plot_flow_completion_time(delay_primary_results, "delay_primary", ['baseline_udp', 'frr'])

    delay_alternate_results = record_flow_completion_time("experiments/delay-alternate/",
                                                          "/host/results", "delay_alternate")
    plot_flow_completion_time(delay_alternate_results, "delay_alternate", ['baseline_no_udp', 'frr_no_udp'])
    plot_flow_completion_time(delay_alternate_results, "delay_alternate", ['baseline_udp', 'frr'])

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
