#pragma once

#include <atomic>

namespace enetpp {

	class client_statistics {
	public:
		std::atomic<size_t> channelCount = 0;
		std::atomic<enet_uint32> connectID = 0;
		std::atomic<enet_uint32> earliestTimeout = 0;
		std::atomic<enet_uint32> highestRoundTripTimeVariance = 0;
		std::atomic<enet_uint32> incomingBandwidth = 0;
		std::atomic<enet_uint32> incomingBandwidthThrottleEpoch = 0;
		std::atomic<enet_uint32> incomingDataTotal = 0;
		std::atomic<enet_uint16> incomingPeerID = 0;
		std::atomic<enet_uint8> incomingSessionID = 0;
		std::atomic<enet_uint16> incomingUnsequencedGroup = 0;
		std::atomic<enet_uint32> lastReceiveTime = 0;
		std::atomic<enet_uint32> lastRoundTripTime = 0;
		std::atomic<enet_uint32> lastRoundTripTimeVariance = 0;
		std::atomic<enet_uint32> lastSendTime = 0;
		std::atomic<enet_uint32> lowestRoundTripTime = 0;
		std::atomic<enet_uint32> mtu = 0;
		std::atomic<int> needsDispatch = 0;
		std::atomic<enet_uint32> nextTimeout = 0;
		std::atomic<enet_uint32> outgoingBandwidth = 0;
		std::atomic<enet_uint32> outgoingBandwidthThrottleEpoch = 0;
		std::atomic<enet_uint32> outgoingDataTotal = 0;
		std::atomic<enet_uint16> outgoingPeerID = 0;
		std::atomic<enet_uint16> outgoingReliableSequenceNumber = 0;
		std::atomic<enet_uint8> outgoingSessionID = 0;
		std::atomic<enet_uint16> outgoingUnsequencedGroup = 0;
		std::atomic<enet_uint32> packetLoss = 0;
		std::atomic<enet_uint32> packetLossEpoch = 0;
		std::atomic<enet_uint32> packetLossVariance = 0;
		std::atomic<enet_uint32> packetsLost = 0;
		std::atomic<enet_uint32> packetsSent = 0;
		std::atomic<enet_uint32> packetThrottle = 0;
		std::atomic<enet_uint32> packetThrottleAcceleration = 0;
		std::atomic<enet_uint32> packetThrottleCounter = 0;
		std::atomic<enet_uint32> packetThrottleDeceleration = 0;
		std::atomic<enet_uint32> packetThrottleEpoch = 0;
		std::atomic<enet_uint32> packetThrottleInterval = 0;
		std::atomic<enet_uint32> packetThrottleLimit = 0;
		std::atomic<enet_uint32> pingInterval = 0;
		std::atomic<enet_uint32> reliableDataInTransit = 0;
		std::atomic<int> roundTripTime = 0;
		std::atomic<int> roundTripTimeVariance = 0;
		std::atomic<enet_uint32> timeoutLimit = 0;
		std::atomic<enet_uint32> timeoutMaximum = 0;
		std::atomic<enet_uint32> timeoutMinimum = 0;
		std::atomic<size_t> totalWaitingData = 0;
		std::atomic<enet_uint32> windowSize = 0;

	public:
		client_statistics() = default;
	};

}