#include <chrono>
#include <experimental/optional>
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

#include "proof_of_work_game.grpc.pb.h"
#include "pow_client.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::CompletionQueue;
using grpc::Status;

namespace event_tags {
const uint32_t kStreamInitialized = 1;
const uint32_t kReadCompleted = 2;
const uint32_t kWriteCompleted = 3;
}

ProofOfWorkGameClient::ProofOfWorkGameClient(const std::string& target)
  : connected_(false) {
  stub_ = ProofOfWorkGame::NewStub(grpc::CreateChannel(
      target, grpc::InsecureChannelCredentials()));
}

bool ProofOfWorkGameClient::Start() {
  stream_ = stub_->AsyncSolvePuzzles(
      &context_, &cq_, (void*)event_tags::kStreamInitialized);
  uint32_t tag;
  bool ok;
  if (cq_.Next(reinterpret_cast<void**>(&tag), &ok)
      && tag == event_tags::kStreamInitialized && ok) {
    connected_ = true;
    ReadPuzzleAsync();
    return true;
  }
  return false;
}

bool ProofOfWorkGameClient::IsConnected() {
  return connected_;
}

std::experimental::optional<Puzzle> ProofOfWorkGameClient::PollNextPuzzle(
    uint32_t millis) {
  if (!connected_) {
    return {};
  }

  auto result = PollAsync(millis);

  if (!result.ok) {
    connected_ = false;
    return {};
  }

  if (result.status == CompletionQueue::GOT_EVENT) {
    if (result.tag == event_tags::kReadCompleted) {
      Puzzle puzzle = puzzle_;
      ReadPuzzleAsync();
      return {puzzle};
    }
  } else if (result.status == CompletionQueue::SHUTDOWN) {
    connected_ = false;
  }

  return {};
}

void ProofOfWorkGameClient::SendSolution(const Solution &solution) {
  if (connected_) {
    stream_->Write(solution, (void*)event_tags::kWriteCompleted);
  }
}

ProofOfWorkGameClient::PollResult ProofOfWorkGameClient::PollAsync(
    uint32_t millis) {
  PollResult result = {(CompletionQueue::NextStatus)0, true, 0};
  result.status = cq_.AsyncNext(
      (void**)&result.tag, &result.ok,
      std::chrono::system_clock::now()
          + std::chrono::milliseconds(millis));
  return result;
}

void ProofOfWorkGameClient::ReadPuzzleAsync() {
  stream_->Read(&puzzle_, (void*)event_tags::kReadCompleted);
}
