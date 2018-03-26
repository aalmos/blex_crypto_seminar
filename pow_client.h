#ifndef POW_CLIENT_H
#define POW_CLIENT_H

#include <experimental/optional>
#include <grpc++/completion_queue.h>

#include "proof_of_work_game.grpc.pb.h"

class ProofOfWorkGameClient {
 public:
  explicit ProofOfWorkGameClient(const std::string& target);
  bool Start();
  bool IsConnected();
  std::experimental::optional<Puzzle> PollNextPuzzle(uint32_t millis);
  void SendSolution(const Solution& solution);

 private:
  struct PollResult {
    grpc::CompletionQueue::NextStatus status;
    bool ok;
    uint32_t tag;
  };

  grpc::ClientContext context_;
  grpc::CompletionQueue cq_;
  std::unique_ptr<ProofOfWorkGame::Stub> stub_;
  std::shared_ptr<grpc::ClientAsyncReaderWriter<Solution, Puzzle>> stream_;
  bool connected_;

  Puzzle puzzle_;

  PollResult PollAsync(uint32_t millis);
  void ReadPuzzleAsync();
};

#endif //POW_CLIENT_H
