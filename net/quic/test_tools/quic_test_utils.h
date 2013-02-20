// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Common utilities for Quic tests

#ifndef NET_QUIC_TEST_TOOLS_QUIC_TEST_UTILS_H_
#define NET_QUIC_TEST_TOOLS_QUIC_TEST_UTILS_H_

#include "net/quic/congestion_control/send_algorithm_interface.h"
#include "net/quic/quic_connection.h"
#include "net/quic/quic_framer.h"
#include "net/quic/quic_session.h"
#include "net/quic/test_tools/mock_clock.h"
#include "net/quic/test_tools/mock_random.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace net {

namespace test {

void CompareCharArraysWithHexError(const std::string& description,
                                   const char* actual,
                                   const int actual_len,
                                   const char* expected,
                                   const int expected_len);

void CompareQuicDataWithHexError(const std::string& description,
                                 QuicData* actual,
                                 QuicData* expected);

// Constructs a basic crypto handshake message
QuicPacket* ConstructHandshakePacket(QuicGuid guid, CryptoTag tag);

// Constructs a ClientHello crypto handshake message
QuicPacket* ConstructClientHelloPacket(QuicGuid guid,
                                       const QuicClock* clock,
                                       QuicRandom* random_generator,
                                       const std::string& server_hostname);

// Constructs a ServerHello crypto handshake message
QuicPacket* ConstructServerHelloPacket(QuicGuid guid,
                                       const QuicClock* clock,
                                       QuicRandom* random_generator);

class MockFramerVisitor : public QuicFramerVisitorInterface {
 public:
  MockFramerVisitor();
  ~MockFramerVisitor();

  MOCK_METHOD1(OnError, void(QuicFramer* framer));
  MOCK_METHOD0(OnPacket, void());
  MOCK_METHOD1(OnPublicResetPacket, void(const QuicPublicResetPacket& header));
  MOCK_METHOD0(OnRevivedPacket, void());
  // The constructor set this up to return true by default.
  MOCK_METHOD1(OnPacketHeader, bool(const QuicPacketHeader& header));
  MOCK_METHOD1(OnFecProtectedPayload, void(base::StringPiece payload));
  MOCK_METHOD1(OnStreamFrame, void(const QuicStreamFrame& frame));
  MOCK_METHOD1(OnAckFrame, void(const QuicAckFrame& frame));
  MOCK_METHOD1(OnCongestionFeedbackFrame,
               void(const QuicCongestionFeedbackFrame& frame));
  MOCK_METHOD1(OnFecData, void(const QuicFecData& fec));
  MOCK_METHOD1(OnRstStreamFrame, void(const QuicRstStreamFrame& frame));
  MOCK_METHOD1(OnConnectionCloseFrame,
               void(const QuicConnectionCloseFrame& frame));
  MOCK_METHOD0(OnPacketComplete, void());

 private:
  DISALLOW_COPY_AND_ASSIGN(MockFramerVisitor);
};

class NoOpFramerVisitor : public QuicFramerVisitorInterface {
 public:
  NoOpFramerVisitor() {}

  virtual void OnError(QuicFramer* framer) OVERRIDE {}
  virtual void OnPacket() OVERRIDE {}
  virtual void OnPublicResetPacket(
      const QuicPublicResetPacket& packet) OVERRIDE {}
  virtual void OnRevivedPacket() OVERRIDE {}
  virtual bool OnPacketHeader(const QuicPacketHeader& header) OVERRIDE;
  virtual void OnFecProtectedPayload(base::StringPiece payload) OVERRIDE {}
  virtual void OnStreamFrame(const QuicStreamFrame& frame) OVERRIDE {}
  virtual void OnAckFrame(const QuicAckFrame& frame) OVERRIDE {}
  virtual void OnCongestionFeedbackFrame(
      const QuicCongestionFeedbackFrame& frame) OVERRIDE {}
  virtual void OnFecData(const QuicFecData& fec) OVERRIDE {}
  virtual void OnRstStreamFrame(const QuicRstStreamFrame& frame) OVERRIDE {}
  virtual void OnConnectionCloseFrame(
      const QuicConnectionCloseFrame& frame) OVERRIDE {}
  virtual void OnPacketComplete() OVERRIDE {}

 private:
  DISALLOW_COPY_AND_ASSIGN(NoOpFramerVisitor);
};

class FramerVisitorCapturingAcks : public NoOpFramerVisitor {
 public:
  FramerVisitorCapturingAcks();
  virtual ~FramerVisitorCapturingAcks();

  // NoOpFramerVisitor
  virtual bool OnPacketHeader(const QuicPacketHeader& header) OVERRIDE;
  virtual void OnAckFrame(const QuicAckFrame& frame) OVERRIDE;
  virtual void OnCongestionFeedbackFrame(
      const QuicCongestionFeedbackFrame& frame) OVERRIDE;

  QuicPacketHeader* header() { return &header_; }

  QuicAckFrame* ack() { return ack_.get(); }
  QuicCongestionFeedbackFrame* feedback() { return feedback_.get(); }

 private:
  QuicPacketHeader header_;
  scoped_ptr<QuicAckFrame> ack_;
  scoped_ptr<QuicCongestionFeedbackFrame> feedback_;

  DISALLOW_COPY_AND_ASSIGN(FramerVisitorCapturingAcks);
};

class FramerVisitorCapturingPublicReset : public NoOpFramerVisitor {
 public:
  FramerVisitorCapturingPublicReset();
  virtual ~FramerVisitorCapturingPublicReset();

  virtual void OnPublicResetPacket(
      const QuicPublicResetPacket& packet) OVERRIDE;

  const QuicPublicResetPacket public_reset_packet() {
    return public_reset_packet_;
  }

 private:
  QuicPublicResetPacket public_reset_packet_;
};

class MockConnectionVisitor : public QuicConnectionVisitorInterface {
 public:
  MockConnectionVisitor();
  virtual ~MockConnectionVisitor();

  MOCK_METHOD4(OnPacket, bool(const IPEndPoint& self_address,
                              const IPEndPoint& peer_address,
                              const QuicPacketHeader& header,
                              const std::vector<QuicStreamFrame>& frame));
  MOCK_METHOD1(OnRstStream, void(const QuicRstStreamFrame& frame));
  MOCK_METHOD2(ConnectionClose, void(QuicErrorCode error, bool from_peer));
  MOCK_METHOD1(OnAck, void(AckedPackets acked_packets));
  MOCK_METHOD0(OnCanWrite, bool());

 private:
  DISALLOW_COPY_AND_ASSIGN(MockConnectionVisitor);
};

class MockHelper : public QuicConnectionHelperInterface {
 public:
  MockHelper();
  virtual ~MockHelper();

  MOCK_METHOD1(SetConnection, void(QuicConnection* connection));
  const QuicClock* GetClock() const;
  QuicRandom* GetRandomGenerator();
  MOCK_METHOD2(WritePacketToWire, int(const QuicEncryptedPacket& packet,
                                      int* error));
  MOCK_METHOD1(SetRetransmissionAlarm, void(QuicTime::Delta delay));
  MOCK_METHOD1(SetAckAlarm, void(QuicTime::Delta delay));
  MOCK_METHOD1(SetSendAlarm, void(QuicTime::Delta delay));
  MOCK_METHOD1(SetTimeoutAlarm, void(QuicTime::Delta delay));
  MOCK_METHOD0(IsSendAlarmSet, bool());
  MOCK_METHOD0(UnregisterSendAlarmIfRegistered, void());
  MOCK_METHOD0(ClearAckAlarm, void());
 private:
  const MockClock clock_;
  MockRandom random_generator_;
};

class MockConnection : public QuicConnection {
 public:
  // Uses a MockHelper.
  MockConnection(QuicGuid guid, IPEndPoint address);
  MockConnection(QuicGuid guid,
                 IPEndPoint address,
                 QuicConnectionHelperInterface* helper);
  virtual ~MockConnection();

  MOCK_METHOD3(ProcessUdpPacket, void(const IPEndPoint& self_address,
                                      const IPEndPoint& peer_address,
                                      const QuicEncryptedPacket& packet));
  MOCK_METHOD1(SendConnectionClose, void(QuicErrorCode error));

  MOCK_METHOD3(SendRstStream, void(QuicStreamId id,
                                   QuicErrorCode error,
                                   QuicStreamOffset offset));

  MOCK_METHOD0(OnCanWrite, bool());

  void ProcessUdpPacketInternal(const IPEndPoint& self_address,
                                const IPEndPoint& peer_address,
                                const QuicEncryptedPacket& packet) {
    QuicConnection::ProcessUdpPacket(self_address, peer_address, packet);
  }

 private:
  scoped_ptr<QuicConnectionHelperInterface> helper_;
  DISALLOW_COPY_AND_ASSIGN(MockConnection);
};

class PacketSavingConnection : public MockConnection {
 public:
  PacketSavingConnection(QuicGuid guid, IPEndPoint address);
  virtual ~PacketSavingConnection();

  virtual bool SendOrQueuePacket(QuicPacketSequenceNumber sequence_number,
                                 QuicPacket* packet,
                                 bool force) OVERRIDE;

  std::vector<QuicPacket*> packets_;

 private:
  DISALLOW_COPY_AND_ASSIGN(PacketSavingConnection);
};

class MockSession : public QuicSession {
 public:
  MockSession(QuicConnection* connection, bool is_server);
  virtual ~MockSession();

  MOCK_METHOD4(OnPacket, bool(const IPEndPoint& self_address,
                              const IPEndPoint& peer_address,
                              const QuicPacketHeader& header,
                              const std::vector<QuicStreamFrame>& frame));
  MOCK_METHOD2(ConnectionClose, void(QuicErrorCode error, bool from_peer));
  MOCK_METHOD1(CreateIncomingReliableStream,
               ReliableQuicStream*(QuicStreamId id));
  MOCK_METHOD0(GetCryptoStream, QuicCryptoStream*());
  MOCK_METHOD0(CreateOutgoingReliableStream, ReliableQuicStream*());
  MOCK_METHOD4(WriteData, QuicConsumedData(QuicStreamId id,
                                           base::StringPiece data,
                                           QuicStreamOffset offset,
                                           bool fin));
  MOCK_METHOD0(IsHandshakeComplete, bool());

 private:
  DISALLOW_COPY_AND_ASSIGN(MockSession);
};

class MockSendAlgorithm : public SendAlgorithmInterface {
 public:
  MockSendAlgorithm();
  virtual ~MockSendAlgorithm();

  MOCK_METHOD2(OnIncomingQuicCongestionFeedbackFrame,
               void(const QuicCongestionFeedbackFrame&, const SentPacketsMap&));
  MOCK_METHOD3(OnIncomingAck,
               void(QuicPacketSequenceNumber, QuicByteCount, QuicTime::Delta));
  MOCK_METHOD1(OnIncomingLoss, void(int number_of_lost_packets));
  MOCK_METHOD3(SentPacket, void(QuicPacketSequenceNumber, QuicByteCount, bool));
  MOCK_METHOD1(TimeUntilSend, QuicTime::Delta(bool));
  MOCK_METHOD0(BandwidthEstimate, QuicBandwidth(void));

 private:
  DISALLOW_COPY_AND_ASSIGN(MockSendAlgorithm);
};

}  // namespace test

}  // namespace net

#endif  // NET_QUIC_TEST_TOOLS_QUIC_TEST_UTILS_H_
