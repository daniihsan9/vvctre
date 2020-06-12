// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <cstddef>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <vector>
#include "common/common_types.h"
#include "common/swap.h"
#include "core/hle/service/service.h"

namespace Core {
class System;
} // namespace Core

namespace Kernel {
class Event;
class SharedMemory;
} // namespace Kernel

// Local-WLAN service

namespace Service::NWM {

using MacAddress = std::array<u8, 6>;

/// Information about the received WiFi packets.
/// Acts as our own 802.11 header.
struct WifiPacket {
    enum class PacketType : u8 {
        Beacon,
        Data,
        Authentication,
        AssociationResponse,
        Deauthentication,
        NodeMap,
        MacAddress = 255,
    };
    PacketType type;      ///< The type of 802.11 frame.
    std::vector<u8> data; ///< Raw 802.11 frame data, starting at the management frame header
                          /// for management frames.
    MacAddress transmitter_address; ///< Mac address of the transmitter.
    MacAddress destination_address; ///< Mac address of the receiver.
    u8 channel;                     ///< WiFi channel where this frame was transmitted.
};

const std::size_t ApplicationDataSize = 0xC8;
const u8 DefaultNetworkChannel = 11;

// Number of milliseconds in a TU.
const double MillisecondsPerTU = 1.024;

// Interval measured in TU, the default value is 100TU = 102.4ms
const u16 DefaultBeaconInterval = 100;

/// The maximum number of nodes that can exist in an UDS session.
constexpr u32 UDSMaxNodes = 16;

struct NodeInfo {
    u64_le friend_code_seed;
    std::array<u16_le, 10> username;
    INSERT_PADDING_BYTES(4);
    u16_le network_node_id;
    INSERT_PADDING_BYTES(6);

    void Reset() {
        friend_code_seed = 0;
        username.fill(0);
        network_node_id = 0;
    }
};

static_assert(sizeof(NodeInfo) == 40, "NodeInfo has incorrect size.");

using NodeList = std::vector<NodeInfo>;

enum class NetworkStatus {
    NotConnected = 3,
    ConnectedAsHost = 6,
    Connecting = 7,
    ConnectedAsClient = 9,
    ConnectedAsSpectator = 10,
};

enum class NetworkStatusChangeReason {
    None = 0,
    ConnectionEstablished = 1,
    ConnectionLost = 4,
};

struct ConnectionStatus {
    u32_le status;
    u32_le status_change_reason;
    u16_le network_node_id;
    u16_le changed_nodes;
    u16_le nodes[UDSMaxNodes];
    u8 total_nodes;
    u8 max_nodes;
    u16_le node_bitmask;
};

static_assert(sizeof(ConnectionStatus) == 0x30, "ConnectionStatus has incorrect size.");

struct NetworkInfo {
    std::array<u8, 6> host_mac_address;
    u8 channel;
    INSERT_PADDING_BYTES(1);
    u8 initialized;
    INSERT_PADDING_BYTES(3);
    std::array<u8, 3> oui_value;
    u8 oui_type;
    // This field is received as BigEndian from the game.
    u32_be wlan_comm_id;
    u8 id;
    INSERT_PADDING_BYTES(1);
    u16_be attributes;
    u32_be network_id;
    u8 total_nodes;
    u8 max_nodes;
    INSERT_PADDING_BYTES(2);
    INSERT_PADDING_BYTES(0x1F);
    u8 application_data_size;
    std::array<u8, ApplicationDataSize> application_data;
};

static_assert(offsetof(NetworkInfo, oui_value) == 0xC, "oui_value is at the wrong offset.");
static_assert(offsetof(NetworkInfo, wlan_comm_id) == 0x10, "wlancommid is at the wrong offset.");
static_assert(sizeof(NetworkInfo) == 0x108, "NetworkInfo has incorrect size.");

/// Additional block tag ids in the Beacon and Association Response frames
enum class TagId : u8 {
    SSID = 0,
    SupportedRates = 1,
    DSParameterSet = 2,
    TrafficIndicationMap = 5,
    CountryInformation = 7,
    ERPInformation = 42,
    VendorSpecific = 221
};

struct Node {
    bool connected;
    u16 node_id;
};

struct BindNodeData {
    u32 bind_node_id;    ///< Id of the bind node associated with this data.
    u8 channel;          ///< Channel that this bind node was bound to.
    u16 network_node_id; ///< Node id this bind node is associated with, only packets from this
                         /// network node will be received.
    std::shared_ptr<Kernel::Event> event;         ///< Receive event for this bind node.
    std::deque<std::vector<u8>> received_packets; ///< List of packets received on this channel.
};

class NWM_UDS final : public ServiceFramework<NWM_UDS> {
public:
    explicit NWM_UDS(Core::System& system);
    ~NWM_UDS();
 
private:
    Core::System& system;

    void UpdateNetworkAttribute(Kernel::HLERequestContext& ctx);
    void Shutdown(Kernel::HLERequestContext& ctx);
    void DestroyNetwork(Kernel::HLERequestContext& ctx);
    void DisconnectNetwork(Kernel::HLERequestContext& ctx);
    void GetConnectionStatus(Kernel::HLERequestContext& ctx);
    void GetNodeInformation(Kernel::HLERequestContext& ctx);
    void RecvBeaconBroadcastData(Kernel::HLERequestContext& ctx);
    void SetApplicationData(Kernel::HLERequestContext& ctx);
    void Bind(Kernel::HLERequestContext& ctx);
    void Unbind(Kernel::HLERequestContext& ctx);
    void PullPacket(Kernel::HLERequestContext& ctx);
    void SendTo(Kernel::HLERequestContext& ctx);
    void GetChannel(Kernel::HLERequestContext& ctx);
    void InitializeWithVersion(Kernel::HLERequestContext& ctx);
    void InitializeDeprecated(Kernel::HLERequestContext& ctx);
    void BeginHostingNetwork(Kernel::HLERequestContext& ctx);
    void BeginHostingNetworkDeprecated(Kernel::HLERequestContext& ctx);
    void ConnectToNetwork(Kernel::HLERequestContext& ctx);
    void ConnectToNetworkDeprecated(Kernel::HLERequestContext& ctx);
    void EjectClient(Kernel::HLERequestContext& ctx);
    void DecryptBeaconData(Kernel::HLERequestContext& ctx, u16 command_id);
    template <u16 command_id>
    void DecryptBeaconData(Kernel::HLERequestContext& ctx);

    ResultVal<std::shared_ptr<Kernel::Event>> Initialize(
        u32 sharedmem_size, const NodeInfo& node, u16 version,
        std::shared_ptr<Kernel::SharedMemory> sharedmem);

    ResultCode BeginHostingNetwork(const u8* network_info_buffer, std::size_t network_info_size,
                                   std::vector<u8> passphrase);

    void ConnectToNetwork(Kernel::HLERequestContext& ctx, u16 command_id,
                          const u8* network_info_buffer, std::size_t network_info_size,
                          u8 connection_type, std::vector<u8> passphrase);

    void BeaconBroadcastCallback(u64 userdata, s64 cycles_late);

    /**
     * Returns a list of received 802.11 beacon frames from the specified sender and with the
     * specified wlan_comm_id since the last call.
     */
    std::list<WifiPacket> GetReceivedBeacons(const MacAddress& sender, u32 wlan_comm_id);

    /*
     * Start a connection sequence with an UDS server. The sequence starts by sending an 802.11
     * authentication frame with SEQ1.
     */
    void StartConnectionSequence(const MacAddress& server);

    std::optional<MacAddress> GetNodeMacAddress(u16 dest_node_id, u8 flags);

    // Shared memory provided by the application to store the receive buffer.
    // This is not currently used.
    std::shared_ptr<Kernel::SharedMemory> recv_buffer_memory;

    // Event that will generate and send the 802.11 beacon frames.
    Core::TimingEventType* beacon_broadcast_event;
};

void NetworkThread();
void NetworkThreadStop(); 

} // namespace Service::NWM
 